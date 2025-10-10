# Repository Guidelines

## Project Structure & Module Organization
- `apps/macOS` hosts the SwiftUI companion app plus `BKMDTests` and `BKMDUITests` for regression coverage.
- `firmware/BKMD_firmware` is the PlatformIO workspace for the ESP32-S3 firmware; `src/` stores runtime code, `include/` shares headers, and `test/` holds hardware checks.
- Supporting tools live in `python-scripts`, while board/display tuning stays in `User_Setup.h`; leave generated binaries in `KeyLogger/built` untouched.

## Build, Test, and Development Commands
- `xcodebuild -project apps/macOS/BKMD.xcodeproj -scheme BKMD -configuration Debug build` compiles the macOS app; add `-destination 'platform=macOS' test` to run both test bundles.
- `pio run -d firmware/BKMD_firmware -e esp32-s3-devkitc-1` builds firmware; append `-t upload` for flashing and `pio device monitor ...` to collect serial logs.
- `python3 -m venv python-scripts/venv && source python-scripts/venv/bin/activate` prepares automation scripts before installing dependencies with pip.

## Coding Style & Naming Conventions
- Swift code follows Swift API Design Guidelines: camelCase members, UpperCamelCase types, and concise SwiftUI views.
- Firmware sources use 4-space indentation, ALL_CAPS macros (e.g. `LED_PIN`), and snake_case helpers; keep reusable declarations in headers.
- Python utilities track PEP 8, descriptive snake_case names, and protect script entry with `if __name__ == "__main__":`.

## Testing Guidelines
- Name Swift tests `test<Scenario>` inside `BKMDTests` (logic) or `BKMDUITests` (interaction); capture screenshots for UI regressions.
- Place PlatformIO cases under `firmware/BKMD_firmware/test` and scope fixtures by environment (e.g. `esp32-s3-devkitc-1`).
- Before opening a PR, verify a macOS build, firmware boot/connection, and at least one serial round-trip via the Python sender.

## Commit & Pull Request Guidelines
- Mirror existing history with short, present-tense messages (e.g. `Add TFT display support`) and list touched modules in the body.
- Reference issues, document test evidence, and mention hardware or ports changed (especially updates to `User_Setup.h`).
- PRs should tag firmware or app reviewers, wait for CI/build feedback, and summarize any manual validation steps.

## Hardware & Security Notes
- Keep secret ports and Wi-Fi credentials out of source; store per-device display tweaks in `User_Setup.h`.
- Disconnect nonessential peripherals before flashing and review startup logs for unexpected HID behavior prior to distributing builds.
