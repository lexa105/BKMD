import { app } from 'electron';
import path from 'node:path';
import fs from 'node:fs';

export interface AppSettings {
    /** Electron accelerator that toggles key/mouse forwarding on and off. */
    switchKeybind: string;
    forwardKeyboard: boolean;
    forwardMouse: boolean;
}

const DEFAULT_SETTINGS: AppSettings = {
    switchKeybind: 'CommandOrControl+Shift+R',
    forwardKeyboard: true,
    forwardMouse: true,
};

class SettingsStore {
    private settings: AppSettings = { ...DEFAULT_SETTINGS };
    private filePath = '';

    /** Must be called once the app is ready (needs the userData path). */
    public load() {
        this.filePath = path.join(app.getPath('userData'), 'bkmd-settings.json');
        try {
            const raw = JSON.parse(fs.readFileSync(this.filePath, 'utf-8'));
            this.settings = { ...DEFAULT_SETTINGS, ...raw };
        } catch {
            // First run or unreadable file - keep defaults.
        }
    }

    public get(): AppSettings {
        return { ...this.settings };
    }

    public update(patch: Partial<AppSettings>): AppSettings {
        this.settings = { ...this.settings, ...patch };
        try {
            fs.writeFileSync(this.filePath, JSON.stringify(this.settings, null, 2));
        } catch (err) {
            console.error('Failed to persist settings:', err);
        }
        return this.get();
    }
}

export const settingsStore = new SettingsStore();
