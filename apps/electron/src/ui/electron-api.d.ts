export interface BluetoothDevice {
    id: string;
    name: string;
    rssi: number;
    connectable: boolean;
}

export type ConnectionState = 'disconnected' | 'connecting' | 'connected';

export interface AppSettings {
    switchKeybind: string;
    forwardKeyboard: boolean;
    forwardMouse: boolean;
}

export interface BkmdApi {
    isBluetoothAvailable(): Promise<boolean>;
    isScanning(): Promise<boolean>;
    getConnectionState(): Promise<ConnectionState>;
    getDevices(): Promise<BluetoothDevice[]>;
    startScan(): Promise<void>;
    stopScan(): Promise<void>;
    connect(deviceId: string): Promise<{ ok: true } | { ok: false; error: string }>;
    disconnect(): Promise<void>;

    getSettings(): Promise<AppSettings>;
    setForwarding(patch: { forwardKeyboard?: boolean; forwardMouse?: boolean }): Promise<AppSettings>;
    beginKeybindCapture(): Promise<void>;
    cancelKeybindCapture(): Promise<void>;
    setKeybind(accelerator: string): Promise<{ ok: true; settings: AppSettings } | { ok: false; error: string }>;
    getMonitorState(): Promise<boolean>;
    setMonitorState(active: boolean): Promise<boolean>;

    onDeviceDiscovered(callback: (device: BluetoothDevice) => void): () => void;
    onScanStateChanged(callback: (scanning: boolean) => void): () => void;
    onConnectionStateChanged(callback: (state: ConnectionState, device: BluetoothDevice | null) => void): () => void;
    onMonitorStateChanged(callback: (active: boolean) => void): () => void;
}

declare global {
    interface Window {
        bkmd: BkmdApi;
    }
}
