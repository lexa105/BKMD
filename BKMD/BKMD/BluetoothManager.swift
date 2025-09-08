//
//  BluetoothManager.swift
//  BKMD
//
//  Created by MacBook on 12.07.2025.
//

import Foundation
import CoreBluetooth

class BluetoothManager: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    var centralManager: CBCentralManager!
    @Published var peripherals: [CBPeripheral] = []
    @Published var connectedPeripheralIDs: Set<UUID> = []
    @Published var peripheralCharacteristics: [UUID: [CBCharacteristic]] = [:]
    @Published var showBluetoothAlert = false
    @Published var bluetoothIsOn: Bool = false
        
    
    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(handleBluetoothUnavailable), name: Notification.Name("BluetoothUnavailable"), object: nil)
    }
    
    @objc private func handleBluetoothUnavailable() {
        DispatchQueue.main.async {
            self.showBluetoothAlert = true
        }
    }
    
    func connect(_ peripheral: CBPeripheral) {
        centralManager.connect(peripheral, options: nil)
    }
    
    func disconnect(_ peripheral: CBPeripheral) {
        centralManager.cancelPeripheralConnection(peripheral)
    }
    
    // 1️⃣ Called when Bluetooth state changes
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            print("🔵 Bluetooth is ON. Starting scan.")
            centralManager.scanForPeripherals(withServices: nil, options: nil)
        } else {
            print("⚠️ Bluetooth is not available.")
            DispatchQueue.main.async {
                NotificationCenter.default.post(name: Notification.Name("BluetoothUnavailable"), object: nil)
            }
        }
    }
    
    // 2️⃣ Called when a device is discovered
   func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
       if !peripherals.contains(peripheral) {
           peripherals.append(peripheral)
           print("📡 Found: \(peripheral.name ?? "Unknown")")
       }
    }
    
    // Called when connect to the peripheral
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("✅ Connected to \(peripheral.name ?? "Unknown")")
        print("\(peripheral.name ?? "Uknown") Service: \(peripheral.services ?? [])")
        connectedPeripheralIDs.insert(peripheral.identifier)
        peripheral.delegate = self
        peripheral.discoverServices(nil)
    }
    
    func centralManager(_ central: CBCentralManager,
                        didDisconnectPeripheral peripheral: CBPeripheral,
                        error: Error?) {
        connectedPeripheralIDs.remove(peripheral.identifier)
        print("❌ Disconnected from \(peripheral.name ?? "Unknown")")
    }
    
    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverServices
                    error : Error?) {
        if let error = error {
            print("Error has occured while discovering services: \(error.localizedDescription)")
            return
        }
        
        guard let services = peripheral.services else { return }
        for service in services {
            print("Found \(peripheral.name ?? "Unknown") Service:\(service.uuid)")
            peripheral.discoverCharacteristics(nil, for: service)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverCharacteristicsFor service: CBService,
                    error: Error?) {
        if let error = error {
            print("Error discovering characteristics for \(service.uuid): \(error.localizedDescription)")
            return
        }
        
        guard let chars = service.characteristics, !chars.isEmpty else {
            print("No characteristics for service \(service.uuid)")
            return
        }
        print("Discovered \(chars.count) characteristics for \(service.uuid)")
        
        var all = peripheralCharacteristics[peripheral.identifier] ?? []
        all.append(contentsOf: chars)
        peripheralCharacteristics[peripheral.identifier] = all
        
        for ch in chars {
            print("Char: \(ch.uuid), Props:\(ch.properties)")
//          IMPORTANT NOTE: move to the function alone: Switch mode.
            if ch.uuid.uuidString == "1234" {
                print("1234 is for the write")
                let text = "01"
                guard let payload = text.data(using: .ascii) else {
                    print("⚠️ Failed to encode string")
                    return
                }
                // 0x0A = .read (0x02) + .write (0x08) -> use .withResponse
                if ch.properties.contains(.write) {
                    peripheral.writeValue(payload, for: ch, type: .withResponse)
                    print("➡️ Wrote with response to \(ch.uuid)")
                } else if ch.properties.contains(.writeWithoutResponse) {
                    // not expected for 0x0A, but safe fallback
                    peripheral.writeValue(payload, for: ch, type: .withoutResponse)
                    print("➡️ Wrote without response to \(ch.uuid)")
                } else {
                    print("⚠️ Characteristic \(ch.uuid) does not support write operations")
                }
            }
            
        }
    }
    
    func switchModeToWriteMode(_ peripheral: CBPeripheral,
                               serivce: CBService,) {
        
    }
    
    func peripheral(_ peripheral: CBPeripheral,
                    didUpdateValueFor characteristic: CBCharacteristic,
                    error: Error?) {
        if let error = error {
            print("❌ Error updating value for \(characteristic.uuid): \(error.localizedDescription)")
            return
        }

        guard let data = characteristic.value else {
            print("⚠️ No value for \(characteristic.uuid)")
            return
        }

        // Convert raw Data to hex string
        let hexString = data.map { String(format: "%02X", $0) }.joined(separator: " ")
        print("📥 Value for \(characteristic.uuid): \(hexString)")

        // Or, if it's UTF-8 text:
        if let stringValue = String(data: data, encoding: .utf8) {
            print("📄 Text value: \(stringValue)")
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didWriteValueFor characteristic: CBCharacteristic,
                    error: Error?) {
        if let error = error {
            print("❌ Write failed for \(characteristic.uuid): \(error.localizedDescription)")
        } else {
            print("✅ Write succeeded for \(characteristic.uuid)")
        }
    }
    
    
}
