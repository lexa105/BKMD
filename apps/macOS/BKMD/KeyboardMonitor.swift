//
//  oendode.swift
//  BKMD
//
//  Created by MacBook on 21.09.2025.
//

import Foundation
import IOKit.hid //Low level C API for communication with USB HID.

final class KeyboardMonitor {
    private var manager: IOHIDManager!
    
    init() {
        //Initiliaze IOHid Manager
        manager = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone))
        
        //2 Match Keyboards ??
        let matching: [String: Any] = [
            kIOHIDDeviceUsagePageKey as String: kHIDPage_GenericDesktop,
            kIOHIDDeviceUsageKey as String: kHIDUsage_GD_Keyboard
        ]
        
        IOHIDManagerSetDeviceMatching(manager, matching as CFDictionary)
        
        print("dick")
        print(matching)
        
        
    }
    
    let monitor = KeyboardMonitor()

}
