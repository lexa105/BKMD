# CLAUDE.md

Guidance for Claude Code (and future contributors) working in this repository.

## What this project is

BKMD (Bluetooth Keyboard Mouse Dongle) lets you control one computer ("**PC2**", the target)
using the keyboard (and eventually mouse) of another computer ("**PC1**", typically a laptop),
over BLE, via a custom ESP32-S3 USB dongle.

Motivating use case: a desktop PC + a laptop set up side by side, where the laptop is physically
in front of/on top of the desktop's keyboard, leaving no room to use it. Instead of reaching
around, the user types on the laptop and those keystrokes are forwarded wirelessly to the desktop.

### Physical/data flow

```
Laptop (PC1)                         Desktop/target PC (PC2)
┌─────────────────────┐              ┌──────────────────────┐
│ Electron app         │   BLE        │  USB port              │
│  - KeyMonitor         │───────────▶│  ┌──────────────────┐  │
│    (uiohook-napi)     │  writes    │  │ ESP32-S3 dongle    │  │
│  - BluetoothManager   │  HID       │  │  - NimBLE server   │  │
│    (@stoprocent/noble)│  reports   │  │  - USB HID device  │──┼─▶ appears as a
└─────────────────────┘              │  │    (keyboard/mouse)│  │   real keyboard/mouse
                                      │  └──────────────────┘  │   to PC2
                                      └──────────────────────┘
```

1. The Electron app runs on the laptop (PC1) and hooks global keyboard events with
   `uiohook-napi`.
2. Keys are translated into standard USB HID usage codes and packed into an 8-byte HID
   boot-keyboard report.
3. The report is written over BLE to a characteristic exposed by the ESP32 dongle, which is
   plugged in via USB-A to PC2.
4. The dongle firmware (NimBLE peripheral + USB HID device) receives the report and replays it
   over USB HID, so PC2's OS sees a normal hardware keyboard/mouse.
5. A global shortcut in the Electron app (`Cmd/Ctrl+Shift+R`) toggles whether local keystrokes
   are currently being captured/forwarded — this is the "switching" mechanism referenced
   throughout the code, meant to avoid sending input to both machines at once.

## Repo layout

```
apps/
  electron/     Active cross-platform desktop app (Electron + React + Tailwind + TypeScript)
  macOS/        Legacy native Swift/SwiftUI prototype — superseded, kept for reference only
firmware/
  BKMD_firmware/        Active ESP32-S3 firmware (PlatformIO + Arduino framework + NimBLE)
  AirdropOnly_firmware/ WIP experimental firmware exploring the "AirDrop toggle" feature in isolation
  platformio/            Local scratch PlatformIO scaffold, gitignored, not part of the build
docs/           Currently empty — intended home for future protocol/architecture docs
python-scripts/, protocol/   Out of scope / not actively maintained — ignore unless asked
```

Ownership split: application (Electron/macOS) is developed by the primary maintainer
(lexatuan@gmail.com); firmware is developed by hardware collaborator **@Dubleriino**. Firmware
source comments are frequently written in Czech.

## Current implementation status (as of 2026-07)

Working:
- Electron app: global shortcut start/stop of keyboard capture, and forwarding of keyboard-only
  HID reports over BLE to a dongle.
- Firmware: receiving 8-byte HID keyboard reports over BLE and replaying them via `USBHIDKeyboard`;
  legacy 1-byte usage-ID path; clipboard-paste-as-typed-text util command; "AirDrop" advertising
  toggle via long button press; optional TFT status display on the LilyGO board variant.

Not yet working / explicitly TODO in code:
- Mouse support end-to-end (no mouse hook in `keymonitor.ts`; firmware's `hid_decode` has a
  `// TODO mouse handle` stub; `USBHIDMouse` is instantiated in `main.cpp` but unused).
- Device discovery/selection UI — `bluetooth-manager.ts` currently scans and auto-connects to a
  hardcoded dongle UUID ("TEST DEMO" in comments), rather than letting the user pick a device.
- The React UI (`apps/electron/src/ui/App.tsx`) is a static mockup with hardcoded fake device
  data, not wired to the backend (`BluetoothManager`/`KeyMonitor`) yet.
- Per-user keybind configuration for switching between devices — only the single hardcoded
  global shortcut exists today.
- `apps/macOS` is frozen; do not add new features there — port relevant logic to
  `apps/electron` instead.

## BLE protocol (dongle firmware ↔ Electron app)

Defined in `firmware/BKMD_firmware/src/ble/ble_server.h`.

- Service UUID: `B00B`
- Characteristic `1234` ("UTIL", read/write): control-plane messages
  - first byte `'1'` → toggle AirDrop mode (soft-stops/resumes BLE advertising)
  - first byte `'C'` → remaining bytes are UTF-8 text to type out via USB HID (clipboard paste)
- Characteristic `1235` ("DATA", read/write): HID reports
  - 8-byte payload → standard USB HID boot-keyboard report (`report[0]` = modifier bitmask,
    `report[2..7]` = up to 6 pressed HID usage codes)
  - 1-byte payload → legacy single usage-ID press with an auto-release timer (~200ms)

The firmware decodes both channels through a FreeRTOS queue (`BlePacket`) consumed by
`DecoderTask` in `main.cpp`.

## Building & running

### Electron app (`apps/electron`)

```bash
npm install
npm run dev          # runs Vite (React UI) + Electron concurrently
npm run build         # type-check + production build
npm run dist:mac      # package a macOS .dmg/.app (arm64)
npm run dist:win       # package for Windows
npm run dist:linux     # package for Linux
npm run lint
```

Key files:
- `src/electron/main.ts` — app lifecycle, global shortcut registration, wiring `KeyMonitor` →
  `BluetoothManager`
- `src/electron/keymonitor.ts` — global key capture + HID report construction
- `src/electron/bluetooth-manager.ts` — BLE central role (scan/connect/write) via `noble`
- `src/ui/` — React/Tailwind renderer (currently a non-functional mockup)

### Firmware (`firmware/BKMD_firmware`)

PlatformIO project with two environments:
- `esp32-s3-devkitc-1` — generic ESP32-S3 dev board, no display
- `lilygo-t-dongle-s3` — LilyGO T-Dongle S3 (USB-A form factor, has a TFT display) — the actual
  target hardware for this project

```bash
pio run -e lilygo-t-dongle-s3            # build
pio run -e lilygo-t-dongle-s3 -t upload   # build + flash
pio device monitor -b 115200               # serial log
```

Note: the `lilygo-t-dongle-s3` env requires `TFT_eSPI`'s `User_Setup.h` to be configured after
first library download (see root README "Notes" section).

## Conventions & gotchas

- HID usage-code mapping in `keymonitor.ts` (`MAC_HID_MAP`) is keyed on **macOS** `uiohook-napi`
  keycodes — it has not been verified against Windows/Linux keycodes despite the app targeting
  all three via Electron.
- Several files mix English and Czech comments/TODOs (e.g. `main.cpp`, `keymonitor.ts`) — this is
  normal for this repo given the two collaborators; don't "clean up" language when editing nearby
  code unless asked.
- Do not touch `python-scripts/` or `protocol/` unless explicitly asked — they're excluded from
  the active app/firmware work described above.
- When making changes that span the BLE boundary (report format, characteristic UUIDs, packet
  framing), update **both** `apps/electron/src/electron/keymonitor.ts` /
  `bluetooth-manager.ts` and `firmware/BKMD_firmware/src/ble/ble_server.h` — they must agree on
  wire format since there's no shared schema/codegen between the two languages.
