# BKMD - Bluetooth Keyboard and Mouse Dongler/Device

## What is BKMD?

BKMD is a custom ESP32-S3 USB dongle plus a companion desktop app that lets you control one
computer using the keyboard (and, eventually, mouse) of another computer over Bluetooth Low
Energy.

The motivating scenario: you have a desktop PC and a laptop set up side by side, and the laptop
sits in front of (or on top of) the desktop's keyboard, leaving little to no room to use it.
Instead of physically reaching around your laptop to use the desktop's keyboard, BKMD lets you
just type on the laptop and forwards those keystrokes wirelessly to the desktop.

**How it works, end to end:**
1. The BKMD dongle plugs via USB-A into the PC you want to *control* (the "target"). To that PC,
   it enumerates as a normal USB keyboard/mouse (USB HID device).
2. The desktop app runs on the machine you want to *control from* (e.g. your laptop) and hooks
   your keyboard globally, using the [`uiohook-napi`](https://github.com/SnosMe/uiohook-napi)
   library (native bindings around `libuiohook`) for OS-level, cross-platform global key capture.
3. Captured keystrokes are converted into standard USB HID reports and sent to the dongle over
   BLE.
4. The dongle replays those reports over USB HID into the target PC — so from the target PC's
   point of view, it's just a keyboard being typed on.
5. A global shortcut in the app toggles capture on/off, so you can switch which machine your
   keyboard is currently "aimed at" without unplugging anything.

### Repository layout

- `apps/electron/` — the actively developed cross-platform desktop app (Electron + React +
  TypeScript). This is where current development happens.
- `apps/macOS/` — the original native Swift/SwiftUI prototype from the first development phase.
  Frozen/kept for reference; not under active development.
- `firmware/BKMD_firmware/` — the active ESP32-S3 firmware (PlatformIO, Arduino framework,
  NimBLE) that runs on the dongle itself.
- `firmware/AirdropOnly_firmware/` — a work-in-progress firmware variant isolating a newer
  feature under development.
- `docs/` — reserved for future protocol/architecture documentation.

Key libraries used in `apps/electron`:
- [`uiohook-napi`](https://github.com/SnosMe/uiohook-napi) — global keyboard (and mouse, once
  wired up) hook used to capture input on the controlling machine.
- [`@stoprocent/noble`](https://github.com/stoprocent/noble) — BLE central-role library used to
  scan for, connect to, and write HID reports to the dongle.

See `CLAUDE.md` at the repo root for a more detailed technical breakdown (BLE protocol/UUIDs,
build commands, current implementation status and known gaps) intended for contributors and AI
coding assistants working in this codebase.

### Project status (2026-07)

Currently working: toggling keyboard capture via global shortcut, and forwarding captured
keystrokes as BLE HID reports from the app to the dongle, which the firmware correctly replays
as USB HID keyboard input on the target PC.

Not yet implemented: mouse forwarding (app and firmware both have stubs but no working path
end-to-end), an in-app device picker (the app currently auto-connects to a hardcoded dongle
during testing), and a real UI (the current React UI is a visual mockup, not yet wired to the
backend).

## SOFTWARE Desktop App
As the first development phase in 2025 occurred in Swift to create a demo prototype and verify project's core functionality and feasibility (DONE: With some errors, but the main function worked in Q4 2025)

We decided to shift from native development to cross-platform development in Electron or similar framework. Main motivations behind this move is to:
* **Maintain Uniformity:** Develop for Windows, macOS, and Linux using a single codebase.
* **Increase Flexibility:** Focus on rapid feature deployment and lower development costs.
* **Optimize for Workflow:** The primary use case is `Desktop PC + BKMD <-> BKMD-config-app + Laptop`.

Sacrificing some native optimization for flexibility, cost and uniformity of development.

We expect major updates and upgrades, to early final product to be made in 2026. 

## HARDWARE

The dongle firmware (`firmware/BKMD_firmware`) is built with PlatformIO on the Arduino framework
and targets two board configurations:

* **`lilygo-t-dongle-s3`** — the actual target hardware: a LilyGO T-Dongle S3 with a male USB-A
  connector (so it plugs directly into the target PC) and a small onboard TFT display used to
  show connection/debug state.
* **`esp32-s3-devkitc-1`** — a generic ESP32-S3 dev board used for development/testing without a
  display.

The firmware advertises itself over BLE (NimBLE) as a GATT server with one service exposing a
data characteristic (keyboard/mouse HID reports) and a util characteristic (control messages like
clipboard paste and an "AirDrop" advertising toggle triggered by a hardware button). On the USB
side it enumerates as a native HID keyboard/mouse via the ESP32-S3's USB peripheral, so the
target PC needs no special drivers.

## WORK IN PROGRESS

Our goal is to create universal USB dongle that would allow you to control different PC over WPAN.
We use ESP32S3 as it offers HID and BLE peripherals. 

https://www.aliexpress.com/item/1005009024098181.html?spm=a2g0o.detail.0.0.7031xi6bxi6b8k&productId=1005009024098181&pdp_ext_f=%7B%22tabScene%22%3A%22retail%22%2C%22sku_id%22%3A12000047619166787%2C%22origProductId%22%3A%221005009024098181%22%7D&#nav-description

## Notes

Dont forget to change UserSetup.h after you download TFTeSPI library. 
