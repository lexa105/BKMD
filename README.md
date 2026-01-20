# BKMD - Bluetooth Keyboard and Mouse Dongler/Device

## SOFTWARE Desktop App
As the first development phase in 2025 occurred in Swift to create a demo prototype and verify project's core functionality and feasibility (DONE: With some errors, but the main function worked in Q4 2025)

We decided to shift from native development to cross-platform development in Electron or similar framework. Main motivations behind this move is to:
* **Maintain Uniformity:** Develop for Windows, macOS, and Linux using a single codebase.
* **Increase Flexibility:** Focus on rapid feature deployment and lower development costs.
* **Optimize for Workflow:** The primary use case is `Desktop PC + BKMD <-> BKMD-config-app + Laptop`.

Sacrificing some native optimization for flexibility, cost and uniformity of development.

We expect major updates and upgrades, to early final product to be made in 2026. 

## HARDWARE


## WORK IN PROGRESS

Our goal is to create universal USB dongle that would allow you to control different PC over WPAN.
We use ESP32S3 as it offers HID and BLE peripherals. 


## Notes

Dont forget to change UserSetup.h after you download TFTeSPI library. 
