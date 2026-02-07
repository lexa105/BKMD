import {app, BrowserWindow, globalShortcut } from 'electron';
import { uIOhook, UiohookKey } from 'uiohook-napi';
import * as fs from 'fs';
import path from 'path';
import { isDev } from './util.js';




app.on('ready', () => {

    let isMonitoring = false;
    const mainWindow = new BrowserWindow({});
    if (isDev()) {   
        mainWindow.loadURL('http://localhost:5123');
    } else {
        mainWindow.loadFile(path.join(app.getAppPath(), '/dist-react/index.html'));
    }

    const ret = globalShortcut.register('CommandOrControl+Shift+R', () => {
    if(isMonitoring) {
            console.log("Monitoring have already started.")
    } else {
        console.log('Monitoring combination pressed! Monitoring now...');
        setupKeyboardListeners()
    }
    
    });

    if (!ret) {
        console.log('Registration failed. Maybe another app is using this combo?');
    }

    
})



function setupKeyboardListeners() {
    uIOhook.on('keydown', (e) => {
        console.log(e.keycode)

        if (e.keycode === 15 && e.altKey) {
            console.log('User performed an Alt + Tab (Window Switch)');
        }
    })
    uIOhook.start();
    console.log("uIOhook is now running in the background.");

}

