import { withBindings } from '@stoprocent/noble';
import { Buffer } from 'node:buffer';


// 'default' automatically selects the right driver for Windows, Mac, or Linux
const noble = withBindings('default');


export class BluetoothManager {
    public static async initialize() {
        //wait for bluetooth hardware to be ready
        await noble.waitForPoweredOnAsync();
        console.log('Bluetooth is powered on and ready!');
    }

    public static async startScanning() {
        // Start looking for any device (empty array [] means all services)
        // The second parameter 'true' allows duplicate results (useful for RSSI tracking)
        await noble.startScanningAsync([], true);
        let bluetoothMap = new Map<string, string>

        noble.on('discover', async (peripheral) => {
            const peripheral_uuid = peripheral.id
            const peripheral_name = peripheral.advertisement.localName || "Unknown";
            bluetoothMap.set(peripheral_uuid, peripheral_name)
            // console.log(`Found: ${peripheral_name} [${peripheral.id}]`)
            console.log(bluetoothMap)

            //Pokud najdeš přímo náš dongle. Připoj. 
            if(peripheral_uuid === "e80af61f3d5b50333abf2280c5ade676") {
                await noble.stopScanningAsync();
                try {
                    await peripheral.connectAsync(); 
                } catch(err) {
                    console.log(err)
                }
                console.log('Dongle pripojen');
                

                const { services, characteristics } = await peripheral.discoverAllServicesAndCharacteristicsAsync();

                console.log(services, characteristics)
                this.sendData(peripheral, "b00b", "1235", 12)

            }

        })


    }


    public static async sendData(peripheral: any, serviceUuid: string, charUuid: string, data: any) {
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
