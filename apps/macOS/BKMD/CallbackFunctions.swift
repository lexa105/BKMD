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
        
        let monitor = Unmanaged<KeyboardMonitor>
            .fromOpaque(context!)
            .takeUnretainedValue()
        
        let elem: IOHIDElement = IOHIDValueGetElement(device)
//        print(elem)
        let isDown = IOHIDValueGetIntegerValue(device) != 0 //Returns 0 released and 1 pressed
        let scancode = Int(IOHIDElementGetUsage(elem))
        
        if isDown {
            //Temporary debug variable
            let pressedKeys = monitor.pressedKeys.insert(scancode)
            if pressedKeys.inserted {
                monitor.pressedKeySequence.append(scancode)
                print(monitor.pressedKeys)
            }
        } else {
            monitor.pressedKeys.remove(scancode)
            monitor.pressedKeySequence.removeAll { $0 == scancode }
            
        }
        
    }
}

