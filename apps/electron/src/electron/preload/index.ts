import { contextBridge, ipcRenderer } from 'electron';

// Kept in sync by hand with the equivalent types in ../bluetooth-manager.ts.
// Duplicated (rather than imported) so this file's CommonJS build - required
// by Electron's sandboxed preload loader - never pulls in the ESM-only main
// process modules (noble, etc.) via rootDir inference.
export interface BluetoothDevice {
    id: string;
    name: string;
    rssi: number;
    connectable: boolean;
}

export type ConnectionState = 'disconnected' | 'connecting' | 'connected';

function subscribe<T extends unknown[]>(channel: string, callback: (...args: T) => void) {
    const listener = (_event: Electron.IpcRendererEvent, ...args: T) => callback(...args);
    ipcRenderer.on(channel, listener);
    return () => ipcRenderer.removeListener(channel, listener);
}

const bkmdApi = {
    isBluetoothAvailable: (): Promise<boolean> => ipcRenderer.invoke('bluetooth:is-available'),
    isScanning: (): Promise<boolean> => ipcRenderer.invoke('bluetooth:is-scanning'),
    getConnectionState: (): Promise<ConnectionState> => ipcRenderer.invoke('bluetooth:get-connection-state'),
    getDevices: (): Promise<BluetoothDevice[]> => ipcRenderer.invoke('bluetooth:get-devices'),
    startScan: (): Promise<void> => ipcRenderer.invoke('bluetooth:start-scan'),
    stopScan: (): Promise<void> => ipcRenderer.invoke('bluetooth:stop-scan'),
    connect: (deviceId: string): Promise<{ ok: true } | { ok: false; error: string }> =>
        ipcRenderer.invoke('bluetooth:connect', deviceId),
    disconnect: (): Promise<void> => ipcRenderer.invoke('bluetooth:disconnect'),

    onDeviceDiscovered: (callback: (device: BluetoothDevice) => void) =>
        subscribe<[BluetoothDevice]>('bluetooth:device-discovered', callback),
    onScanStateChanged: (callback: (scanning: boolean) => void) =>
        subscribe<[boolean]>('bluetooth:scan-state-changed', callback),
    onConnectionStateChanged: (callback: (state: ConnectionState, device: BluetoothDevice | null) => void) =>
        subscribe<[ConnectionState, BluetoothDevice | null]>('bluetooth:connection-state-changed', callback),
};

contextBridge.exposeInMainWorld('bkmd', bkmdApi);

export type BkmdApi = typeof bkmdApi;
