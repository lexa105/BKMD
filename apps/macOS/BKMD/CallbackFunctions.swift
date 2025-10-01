//
//  CallbackFunctions.swift
//  BKMD
//
//  Created by MacBook on 29.09.2025.
//

import Foundation
import Cocoa

class CallbackFunctions {
//    static func Handle_IOHIDInputValueCallback(context: UnsafeMutableRawPointer?, result: IOReturn, sender: UnsafeRawPointer?, value: IOHIDValue! ) {
//        let mySelf = Unmanaged<KeyboardMonitor>.fromOpaque(context!).takeUnretainedValue()
//        print(value)
//    }
    
    static let HandleIOHIDInputValueCallback: IOHIDValueCallback = { context, result, sender, device in
        print("Callback fired!")
        let mySelf = Unmanaged<KeyboardMonitor>.fromOpaque(context!).takeUnretainedValue()
        let elem: IOHIDElement = IOHIDValueGetElement(device)
        print(elem)
        print("UsagePage: \(IOHIDElementGetUsagePage(elem)), USAGE: \(IOHIDElementGetUsage(elem))")
        let pressed = IOHIDValueGetIntegerValue(device)
        print("Coje kurva getIntegerValue: ", pressed)
        
    }
}
