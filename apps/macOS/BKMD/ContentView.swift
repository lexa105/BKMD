//
//  ContentView.swift
//  BKMD
//
//  Created by MacBook on 12.07.2025.
//

import SwiftUI

struct ContentView: View {
    @StateObject var bluetoothManager: BluetoothManager
    @StateObject var keyboardMonitor: KeyboardMonitor
    @State private var selectedPeripheralID: UUID? = nil

    
    
    init() {
        let manager = BluetoothManager()
          _bluetoothManager = StateObject(wrappedValue: manager)
          _keyboardMonitor = StateObject(wrappedValue:
          KeyboardMonitor(bluetoothManager: manager))
    }
    
    var body: some View {
        NavigationSplitView {
            List(bluetoothManager.peripherals, id: \.identifier, selection: $selectedPeripheralID) {
                peripheral in Text(peripheral.name ?? "Unnamed Device")
            }
        } detail: {
            if let id = selectedPeripheralID,
               let peripheral = bluetoothManager.peripherals.first(where: { $0.identifier == id}) {
                PeripheralView(manager: bluetoothManager, peripheral: peripheral)
                    .environmentObject(keyboardMonitor)
                 Spacer()
            }
                else {
                VStack{
                    Text("Smrdi ti pero")
                }
            }
            
        }.navigationTitle("Bluetooth Devices")
        .alert("Bluetooth is not available", isPresented: $bluetoothManager.showBluetoothAlert) {
            Button("OK", role: .cancel) { }
        }
    }
}

#Preview {
    ContentView()
}
