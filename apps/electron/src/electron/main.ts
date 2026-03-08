import {app, BrowserWindow, globalShortcut } from 'electron';

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
            bluetoothManager.startScanningAndConnect();
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

    keyMonitor.on('hid-report', async (report: Buffer) => {
        await bluetoothManager.sendHidReport(report);
    })
})








async function cleanup() {
    console.log('Performing app cleanup...');
    keyMonitor.stop();
    await bluetoothManager.disconnect()
}


app.on('will-quit', async (event) => {
    event.preventDefault();
    await cleanup();
    app.exit();
})


// Handle unexpected crashes
process.on('uncaughtException', async (error) => {
    console.error('CRASH: Uncaught Exception:', error);
    await cleanup();
    process.exit(1);
});

// Handle termination signals (Ctrl+C)
process.on('SIGINT', async () => {
    await cleanup();
    process.exit(0);
});