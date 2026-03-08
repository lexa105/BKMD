# Vibe-Coded Firmware: 8-Byte HID Report Implementation

## Summary of Changes
The firmware has been upgraded to support **transparent HID reporting**. Previously, it only accepted 1-byte payloads and had to "guess" when to release keys. Now, it can receive full 8-byte keyboard reports from the Electron app and forward them directly to the PC.

## Technical Details

### 1. New HID Decoding Logic
The `hid_decode` function in `main.cpp` was completely rewritten to support two modes:
- **Direct Mode (8-byte packets):** If the received BLE packet is exactly 8 bytes, it is treated as a raw USB HID Keyboard Report.
- **Legacy Mode (1-byte packets):** Maintains backward compatibility for single-key presses with a 200ms auto-release timer.

### 2. Implementation of `KeyReport`
We are using the standard USB HID keyboard report structure provided by the `USBHIDKeyboard` library:
```cpp
typedef struct {
  uint8_t modifiers; // Shift, Ctrl, Alt, GUI
  uint8_t reserved;  // Always 0
  uint8_t keys[6];   // Up to 6 simultaneous key codes
} KeyReport;
```

### 3. Direct Forwarding
By using `keyboard.sendReport((KeyReport*)pkt.data)`, we bypass the high-level Arduino `press()`/`release()` functions. This means:
- **Zero Latency:** The report is sent as-is to the USB stack.
- **Perfect Sync:** The Electron app now has 100% control over key-down and key-up events.
- **N-Key Rollover:** Support for up to 6 keys pressed simultaneously (standard HID limit).

## Why this is better
Previously, the dongle was "smart" but limited. By making it "dumb" and just forwarding raw reports, we moved the intelligence to the Electron app, allowing for much more complex keyboard interactions (like gaming shortcuts or custom macros) without needing to re-flash the firmware.
