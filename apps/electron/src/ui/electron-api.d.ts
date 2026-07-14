export interface BluetoothDevice {
    id: string;
    name: string;
    rssi: number;
    connectable: boolean;
}

export type ConnectionState = 'disconnected' | 'connecting' | 'connected';

export interface BkmdApi {
    isBluetoothAvailable(): Promise<boolean>;
    isScanning(): Promise<boolean>;
    getConnectionState(): Promise<ConnectionState>;
    getDevices(): Promise<BluetoothDevice[]>;
    startScan(): Promise<void>;
    stopScan(): Promise<void>;
    connect(deviceId: string): Promise<{ ok: true } | { ok: false; error: string }>;
    disconnect(): Promise<void>;

    onDeviceDiscovered(callback: (device: BluetoothDevice) => void): () => void;
    onScanStateChanged(callback: (scanning: boolean) => void): () => void;
    onConnectionStateChanged(callback: (state: ConnectionState, device: BluetoothDevice | null) => void): () => void;
}

declare global {
    interface Window {
        bkmd: BkmdApi;
    }
}
