//
//  CallbackFunctions.swift
//  BKMD
//
//  Created by MacBook on 29.09.2025.
//

import Foundation
import Cocoa

class CallbackFunctions {
    static let HandleIOHIDInputValueCallback: IOHIDValueCallback = { context, result, sender, device in
        print("Callback fired!")
        let mySelf = Unmanaged<KeyboardMonitor>.fromOpaque(context!).takeUnretainedValue()
        let elem: IOHIDElement = IOHIDValueGetElement(device)
//        print(elem)
        let pressed = IOHIDValueGetIntegerValue(device)
        let scancode = IOHIDElementGetUsage(elem)
        print(scancode)
        
        
    }
}
