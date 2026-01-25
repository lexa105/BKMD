#include "ble_callbacks.h"
#include "ble_server.h"
#include <NimBLEDevice.h>

//#include "logging"

/** CALLBACKS overwriten methods - */

//SERVER
//Herited classes from NimBLE Callbacks
//only method implementation - class definition in header
void ServerCallbacks :: onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());
    
    //isConnected = true;

    //LOGD("BLE", 0, "Connected");
        
    /**
     *  We can use the connection handle here to ask for different connection parameters.
     *  Args: connection handle, min connection interval, max connection interval
     *  latency, supervision timeout.
     *  Units; Min/Max Intervals: 1.25 millisecond increments.
     *  Latency: number of intervals allowed to skip.
     *  Timeout: 10 millisecond increments.
     */
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
}

void ServerCallbacks :: onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)  {
    Serial.printf("Client disconnected - start advertising\n");

    NimBLEDevice::startAdvertising();
    //isConnected = false;

}

void ServerCallbacks :: onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo)  {
    Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
} // instance of ServerCallback, derivered from NimBLES...

//DATA
//WRITE CALL BACK
/** Handler class for characteristic actions */

    //constructor - abych dostal pointer na queue teto class

void CharacteristicDataCallbacks :: onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  {
    Serial.printf("%s : onRead(), value: %s\n",
                    pCharacteristic->getUUID().toString().c_str(),
                    pCharacteristic->getValue().c_str());
}

/*Write callback + logic - is it ok? ----------------------------*/
void CharacteristicDataCallbacks :: onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  {
    //NimBLEUUID UUID = pCharacteristic->getUUID();
    //std::string value =  pCharacteristic->getValue();//string as byte array - je to ascii - 1 byte 1(string) = 49(dec)
    //size_t valueLen = pCharacteristic->getLength();

    std::string value = pCharacteristic->getValue();
    const size_t n = value.size();
    if (n == 0) return;

    //Create Ble Packet and send to queue
    BlePacket pkt;
    pkt.t_ms = millis();
    pkt.len  = (n > BLE_MAX_PAYLOAD) ? BLE_MAX_PAYLOAD : (uint16_t)n;
    memcpy(pkt.data, value.data(), pkt.len);

    // non-blocking; if full, drop silently (or set a flag)
    (void)xQueueSend(_q, &pkt, 0);

}

    /**
     *  The value returned in code is the NimBLE host return code.
     */
void CharacteristicDataCallbacks :: onStatus(NimBLECharacteristic* pCharacteristic, int code)  {
    Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
}

/** Peer subscribed to notifications/indications */
void CharacteristicDataCallbacks :: onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue)  {
    std::string str  = "Client ID: ";
    str             += connInfo.getConnHandle();
    str             += " Address: ";
    str             += connInfo.getAddress().toString();
    if (subValue == 0) {
        str += " Unsubscribed to ";
    } else if (subValue == 1) {
        str += " Subscribed to notifications for ";
    } else if (subValue == 2) {
        str += " Subscribed to indications for ";
    } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
    }
    str += std::string(pCharacteristic->getUUID());

    Serial.printf("%s\n", str.c_str());
}


//WRITE CALL BACK
//for UTIL char
/** Handler class for characteristic actions */

void CharacteristicUtilCallbacks :: onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    Serial.printf("%s : onRead(), value: %s\n",
                    pCharacteristic->getUUID().toString().c_str(),
                    pCharacteristic->getValue().c_str());
}

    /*Write callback + logic - is it ok? ----------------------------*/
void CharacteristicUtilCallbacks :: onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  {
    NimBLEUUID UUID = pCharacteristic->getUUID();
    std::string value =  pCharacteristic->getValue();//string as byte array - je to ascii - 1 byte 1(string) = 49(dec)
    size_t valueLen = pCharacteristic->getLength();

    Serial.printf("%s : onWrite(), value: %s, len: %d\n",
                    UUID.toString().c_str(),
                    value.c_str(),
                    valueLen);

    //QUEUE add
}
    

    /**
     *  The value returned in code is the NimBLE host return code.
     */
void CharacteristicUtilCallbacks ::  onStatus(NimBLECharacteristic* pCharacteristic, int code)  {
    Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
}

/** Peer subscribed to notifications/indications */
void CharacteristicUtilCallbacks :: onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue)  {
    std::string str  = "Client ID: ";
    str             += connInfo.getConnHandle();
    str             += " Address: ";
    str             += connInfo.getAddress().toString();
    if (subValue == 0) {
        str += " Unsubscribed to ";
    } else if (subValue == 1) {
        str += " Subscribed to notifications for ";
    } else if (subValue == 2) {
        str += " Subscribed to indications for ";
    } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
    }
    str += std::string(pCharacteristic->getUUID());

    Serial.printf("%s\n", str.c_str());
}
