import { uIOhook } from "uiohook-napi";



export class Keylogger {
    constructor() {
        // Initialize uiohook
        uIOhook.on('keydown', (e) => {
        console.log('Keydown:', e.keycode);
           // Your keylogging logic here
        });
    }
}