import {app, BrowserWindow, globalShortcut } from 'electron';
import { uIOhook } from 'uiohook-napi';

//Bluetooth Manager
import { bluetoothManager } from './bluetooth-manager.js'
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { isDev } from './util.js';


const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let mainWindow: BrowserWindow | null = null;

async function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
        // Ensure this path also points to the COMPILED .js file
        preload: path.join(__dirname, '../preload/index.js'),
        contextIsolation: true,
        sandbox: true, // Recommended for security
        },
    });
    if (isDev()) {   
        mainWindow.loadURL('http://localhost:5123');
    } else {
        mainWindow.loadFile(path.join(app.getAppPath(), '/dist-react/index.html'));
    }

}


app.on('ready', async () => {

    try {
        await bluetoothManager.initialize();
        console.log("Bluetooth Ready");
    } catch (err) {
        console.error("Bluetooth initialization failed:", err);
    }

    createWindow();
    bluetoothManager.startScanning();

    let isMonitoring = false;

    const ret = globalShortcut.register('CommandOrControl+Shift+R', () => {
    if(isMonitoring) {
            console.log("Monitoring have already started.")
            return
    } else {
        console.log('Monitoring combination pressed! Monitoring now...');
        setupKeyboardListeners()
    }
    /// No dobry more tohle nefunguje.
    
    });

    if (!ret) {
        console.log('Registration failed. Maybe another app is using this combo?');
    }

    
})



function setupKeyboardListeners() {
    uIOhook.on('keydown', (e) => {
        console.log(`${e.keycode} down`)

        if (e.keycode === 27) {
        }
    })
    uIOhook.on('keyup', (e) => {
        console.log(`${e.keycode} up`)
    })
    uIOhook.start();
    console.log("uIOhook is now running in the background.");


}

