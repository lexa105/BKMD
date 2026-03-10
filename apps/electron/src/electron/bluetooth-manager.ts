import { withBindings } from '@stoprocent/noble';
import { Buffer } from 'node:buffer';


// 'default' automatically selects the right driver for Windows, Mac, or Linux
const noble = withBindings('default');


class BluetoothManager {
    private connectedPeripheral: any = null;

    // TEST DATA and const
    private targetCharacteristics: any = null;
    // private targetService = "b00b"; 
    // private targetCharacteristics2 = "1235";


    public async isBluetoothAvailable(): Promise<boolean> {
        try {
            // Check current state or wait for it to be powered on
            return noble.state === 'poweredOn';
        } catch (err) {
            console.error('Error checking Bluetooth availability:', err);
            return false;
        }
    }

    public async initialize() {
        //wait for bluetooth hardware to be ready
        try {
            await noble.waitForPoweredOnAsync();
            console.log('Bluetooth is powered on and ready!');
        } catch (err) {
            console.log(err)
        }
        
    }

    public async startScanningAndConnect() {
        // TEST DEMO: Now it will connect immediately to the old dongle uuid.

        // Start looking for any device (empty array [] means all services)
        // The second parameter 'true' allows duplicate results (useful for RSSI tracking)
        await noble.startScanningAsync([], true);
        let bluetoothMap = new Map<string, string>

        noble.on('discover', async (peripheral) => {
            const peripheral_uuid = peripheral.id
            const peripheral_name = peripheral.advertisement.localName || "Unknown";
            bluetoothMap.set(peripheral_uuid, peripheral_name)
            
		console.log(bluetoothMap);
            //Pokud najdeš přímo náš dongle. Připoj.
		const dongle_lily = "8d26e87c48ee0d4c63a97b78a319fcf5" 
             const dongle_new = "e80af61f3d5b50333abf2280c5ade676";
            //const dongle_old = "15e7cd4e46ed787ef8167cadcccee727";
            if(peripheral_uuid === dongle_lily) {
                await noble.stopScanningAsync();
                try {
                    await peripheral.connectAsync(); 
                    this.connectedPeripheral = peripheral; //Saving peripheral

                    console.log('Dongle pripojen');
                    const { services, characteristics } = await peripheral.discoverAllServicesAndCharacteristicsAsync();

                    // 3. Print them out (Tip: mapping them makes it easier to read in the console)

                    console.log("Found Services:", services.map(s => s.uuid));
                    console.log("Found Characteristics:", characteristics.map(c => c.uuid));

                    // Set target characteristics for HID reports
                    const hidChar = characteristics.find(c => c.uuid === '1235');
                    if (hidChar) {
                        this.targetCharacteristics = hidChar;
                        console.log('Target characteristic (1235) found and set!');
                    } else {
                        console.warn('Target characteristic (1235) NOT found on this device.');
                    }
                } catch(err) {
                    console.log(err)
                }
                

                
            }

        })


    }

    public async sendData(peripheral: any, serviceUuid: string, charUuid: string, data: any) {
        try {
            const { characteristics } = await peripheral.discoverSomeServicesAndCharacteristicsAsync(
            [serviceUuid], 
            [charUuid]
            );

            const targetChar = characteristics[0];

            if (!targetChar) {
            throw new Error('Characteristic not found');
            }

            const payload = Buffer.from(data, 'utf-8')

            await targetChar.writeAsync(payload, false);
            console.log('Data sent successfully!');

        } catch (error) {
            console.error('Failed to send data:', error);
        }
    }

    public async sendHidReport(report: Buffer) {
        if(!this.targetCharacteristics) {
            console.warn('Cannot send HID report: No target characteristic connected.')
            return;
        }

        try {
            await this.targetCharacteristics.writeAsync(report, false);
            console.log('HID report sent')
        } catch (e) {
            console.error("Failed to send HID report: ", e)
        }
    }

    

    public async disconnect() {
        if (this.connectedPeripheral) {
            console.log('Disconnecting from Bluetooth peripheral...')
            try { 
                await this.connectedPeripheral.disconnectAsync();
                this.connectedPeripheral = null;
                console.log('Bluetooth disconnected cleanly.');
            } catch (err) {
                console.error('Error during Bluetooth disconnect')
            }
        }
    }
}


export const bluetoothManager = new BluetoothManager()
