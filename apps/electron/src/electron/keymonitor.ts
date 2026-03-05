import { uIOhook, UiohookKey } from "uiohook-napi";

const MAC_HID_MAP: Record<number, number> = {
    // Letters
    30: 0x04, 48: 0x05, 46: 0x06, 32: 0x07, 18: 0x08, 33: 0x09,
    34: 0x0A, 35: 0x0B, 23: 0x0C, 36: 0x0D, 37: 0x0E, 38: 0x0F,
    50: 0x10, 49: 0x11, 24: 0x12, 25: 0x13, 16: 0x14, 19: 0x15,
    31: 0x16, 20: 0x17, 22: 0x18, 47: 0x19, 17: 0x1A, 45: 0x1B,
    21: 0x1C, 44: 0x1D,

    // Numbers
    2: 0x1E, 3: 0x1F, 4: 0x20, 5: 0x21, 6: 0x22, 7: 0x23,
    8: 0x24, 9: 0x25, 10: 0x26, 11: 0x27,

    // Modifiers & UI
    3675: 0xE3, // Left Command
    56: 0xE2,   // Left Option
    29: 0xE0,   // Left Control
    42: 0xE1,   // Left Shift
    28: 0x28,   // Return
    1: 0x29,    // Esc
    57: 0x2C    // Space
    }


export class KeyMonitor {
    //TODO: Dopracovat zde architekturu z Electron strany a pak to prepsat do dokumentace v Notionu. 
    
    // 2.state manegment - toto by se mělo měnit, podle stavu zmáčknutých kláves
    private currentModifiers = 0x00;
    private pressedKeys = new Set<number>();

    //Track if we are monitoring.
    private isRunning = false;


    constructor() {
        // Initialize uiohook
        uIOhook.on('keydown', (e) => {
            this.handleKeyEvent(e.keycode, true);
        });

        uIOhook.on('keyup', (e) => {
            this.handleKeyEvent(e.keycode, false)
        })
    }

    public start() {
        if (this.isRunning) return;
        uIOhook.start();
    }

    public stop() {
        console.log("Stopping KeyMonitor");
        this.isRunning = false;

        uIOhook.stop()

        //Reset our state so we dont have "stuck" ?
        this.currentModifiers = 0x00;
        this.pressedKeys.clear;

        //Sending bluetooth default empty buffer.
        this.sendReport()

    }

    

    private handleKeyEvent(uiHookKeycode: number, isDown: boolean) {
        if(this.isModifier(uiHookKeycode)) {
            this.updateModifier(uiHookKeycode, isDown);
        } else {
            const hidCode = MAC_HID_MAP[uiHookKeycode];
            if (!hidCode) {
                console.log("unmapped keycode key ", uiHookKeycode);
                return;
            } 

            if (isDown) {
                this.pressedKeys.add(hidCode);
            } else {
                this.pressedKeys.delete(hidCode);
            }
        }

        this.sendReport();
    }

    private sendReport() {
        //HID report default by 8bytes - default 
        const report = Buffer.alloc(8, 0);

        //byte 0 is for modifiers. 
        report[0] = this.currentModifiers;
        //Byte 1 stays 0
        let i= 2;
        for (const hid of this.pressedKeys) {
            if (i >= 8) break;
            report[i] = hid;
            i++;
        }

        console.log('Sending HID report to BLE: ', report)
        // ZDE PRIDAT BLESENDREPORT
        //TODO: PRIDAT BLE az to bude done. 
    }


    private isModifier(keycode: number): boolean {
        const modifiers = [
            UiohookKey.Ctrl, 
            UiohookKey.CtrlRight, 
            UiohookKey.Shift, 
            UiohookKey.ShiftRight, 
            UiohookKey.Alt, 
            UiohookKey.AltRight, 
            UiohookKey.Meta,
            UiohookKey.MetaRight
        ]
        //Checkne jestli je to modifier.
        // @ts-ignore
        return modifiers.includes(keycode)
    }

    private updateModifier(keycode: number, isDown: boolean) {
        let modifierBit = 0;
        switch(keycode) {
            case UiohookKey.Ctrl:       modifierBit = 0x01; break;
            case UiohookKey.Shift:      modifierBit = 0x02; break;
            case UiohookKey.Alt:        modifierBit = 0x04; break;
            case UiohookKey.Meta:       modifierBit = 0x08; break; // Left Cmd
            case UiohookKey.CtrlRight:  modifierBit = 0x10; break;
            case UiohookKey.ShiftRight: modifierBit = 0x20; break;
            case UiohookKey.AltRight:   modifierBit = 0x40; break;
            case UiohookKey.MetaRight:  modifierBit = 0x80; break; // Right Cmd
        }

        if (isDown) {
            this.currentModifiers |= modifierBit
        } else {
            this.currentModifiers &= ~modifierBit
        }
    }




}