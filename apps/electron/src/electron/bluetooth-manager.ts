import { withBindings } from '@stoprocent/noble';
import { Buffer } from 'node:buffer';


// 'default' automatically selects the right driver for Windows, Mac, or Linux
const noble = withBindings('default');


class BluetoothManager {
    public async initialize() {
        //wait for bluetooth hardware to be ready
        try {
            await noble.waitForPoweredOnAsync();
            console.log('Bluetooth is powered on and ready!');
        } catch (err) {
            console.log(err)
        }
        
    }

    public async startScanning() {
        // Start looking for any device (empty array [] means all services)
        // The second parameter 'true' allows duplicate results (useful for RSSI tracking)
        await noble.startScanningAsync([], true);
        let bluetoothMap = new Map<string, string>

        noble.on('discover', async (peripheral) => {
            const peripheral_uuid = peripheral.id
            const peripheral_name = peripheral.advertisement.localName || "Unknown";
            bluetoothMap.set(peripheral_uuid, peripheral_name)
            

            //Pokud najdeš přímo náš dongle. Připoj. 
            const dongle_new = "e80af61f3d5b50333abf2280c5ade676";
            const donge_old = "15e7cd4e46ed787ef8167cadcccee727";
            if(peripheral_uuid === donge_old) {
                await noble.stopScanningAsync();
                try {
                    peripheral.connect(); 
                } catch(err) {
                    console.log(err)
                }
                console.log('Dongle pripojen');
                

                const { services, characteristics } = await peripheral.discoverAllServicesAndCharacteristicsAsync();

                console.log(services, characteristics)

                const sleep = (ms: number) => new Promise(resolve => setTimeout(resolve, ms));
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
}


export const bluetoothManager = new BluetoothManager()
