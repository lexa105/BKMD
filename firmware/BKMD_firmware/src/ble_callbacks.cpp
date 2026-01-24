#include "ble_callbacks.h"
#include "ble_server.h"
#include <NimBLEDevice.h>
//#include "logging"

/** CALLBACKS overwriten methods - */

//SERVER
//Herited classes from NimBLE Callbacks
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
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

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - start advertising\n");

        NimBLEDevice::startAdvertising();
        //isConnected = false;

    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }
} ; // instance of ServerCallback, derivered from NimBLES...

//DATA
//WRITE CALL BACK
/** Handler class for characteristic actions */
class CharacteristicDataCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    /*Write callback + logic - is it ok? ----------------------------*/
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        //NimBLEUUID UUID = pCharacteristic->getUUID();
        //std::string value =  pCharacteristic->getValue();//string as byte array - je to ascii - 1 byte 1(string) = 49(dec)
        //size_t valueLen = pCharacteristic->getLength();

        //Create Ble Packet and send to queue
        std::string value = pCharacteristic->getValue();   // bytes
        size_t n = value.size();

        BlePacket pkt;
        pkt.seq  = __atomic_add_fetch(&_seq, 1, __ATOMIC_RELAXED);
        pkt.t_ms = millis();
        pkt.len  = (n > BLE_MAX_PAYLOAD) ? BLE_MAX_PAYLOAD : (uint16_t)n;
        memcpy(pkt.data, value.data(), pkt.len);

        if (xQueueSend(_q, &pkt, 0) != pdTRUE) {
            __atomic_add_fetch(&_drop, 1, __ATOMIC_RELAXED);
        }
    
    }
    
    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }

    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
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
} ;

//WRITE CALL BACK
//for UTIL char
/** Handler class for characteristic actions */
class CharacteristicUtilCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    /*Write callback + logic - is it ok? ----------------------------*/
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      NimBLEUUID UUID = pCharacteristic->getUUID();
      std::string value =  pCharacteristic->getValue();//string as byte array - je to ascii - 1 byte 1(string) = 49(dec)
      size_t valueLen = pCharacteristic->getLength();

      Serial.printf("%s : onWrite(), value: %s, len: %d\n",
                      UUID.toString().c_str(),
                      value.c_str(),
                      valueLen);
      lastWrite = millis(); //for led blink

      //display_recieved_data(value.c_str(), valueLen); //show on display

      if(UUID == utilUUID){
       setMode_fromByte(value[0]); //first byte is mode
      }
      if(UUID == dataUUID){
        if(currentState == KEYBOARD_MODE){
          keyboard_data_handle(value.c_str(), valueLen);
        }
        else if(currentState == MOUSE_MODE){
          mouse_data_handle(value.c_str(), valueLen);
        }
      } 
    }
    

    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        Serial.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }

    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
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
} ;