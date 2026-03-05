import {app, BrowserWindow, globalShortcut } from 'electron';
import { uIOhook } from 'uiohook-napi';

//Bluetooth Manager
import { bluetoothManager } from './bluetooth-manager.js'
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { isDev } from './util.js';

// Key Monitor
import { KeyMonitor } from './keymonitor.js';


const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let mainWindow: BrowserWindow | null = null;
const keyMonitor: KeyMonitor = new KeyMonitor();

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
        const isAvailable = await bluetoothManager.isBluetoothAvailable();
        if (!isAvailable) {
            console.error("Bluetooth is not available or powered off.");
            // Optionally notify user or handle accordingly
        } else {
            console.log("Bluetooth Ready and Available");
            bluetoothManager.startScanning();
        }
    } catch (err) {
        console.error("Bluetooth initialization failed:", err);
    }

    createWindow();

    const ret = globalShortcut.register('CommandOrControl+Shift+R', () => {
        if (keyMonitor.isRunning) {
            console.log('Stopping monitoring...');
            keyMonitor.stop();
        } else {
            console.log('Starting monitoring...');
            keyMonitor.start();
        }
    });

    if (!ret) {
        console.log('Registration failed. Maybe another app is using this combo?');
    }
})




