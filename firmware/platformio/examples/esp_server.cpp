
/**
 *  NimBLE_Server Demo:
 *
 *  Demonstrates many of the available features of the NimBLE server library.
 *
 *  Created: on March 22 2020
 *      Author: H2zero
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

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
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }

    /********************* Security handled here *********************/
    uint32_t onPassKeyDisplay() override {
        Serial.printf("Server Passkey Display\n");
        /**
         * This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    }

    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
        Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            Serial.printf("Encrypt connection failed - disconnecting client\n");
            return;
        }

        Serial.printf("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
    }
} serverCallbacks;

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onWrite(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
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
} chrCallbacks;

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        std::string dscVal = pDescriptor->getValue();
        Serial.printf("Descriptor written value: %s\n", dscVal.c_str());
    }

    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
    }
} dscCallbacks;


void setup(void) {
    Serial.begin(115200);

    //wait for serial
    while(Serial.available() == 0);
    Serial.printf("Starting NimBLE Server\n");

    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE");

    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    
    //neivm
    NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC);

    //server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);
    
    NimBLEService*        pBkmbService = pServer->createService("B00B"); //name must be HEX - BKMD NOK
    //UTILS characteristics s hodnotami modu pouziti
    NimBLECharacteristic* pUtilCharacteristic =
        pBkmbService->createCharacteristic("0001",
                                           NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |
                                               /** Require a secure connection for read and write access */
                                               NIMBLE_PROPERTY::READ_ENC | // only allow reading if paired / encrypted
                                               NIMBLE_PROPERTY::WRITE_ENC  // only allow writing if paired / encrypted
        );

    pUtilCharacteristic->setValue("Setup");
    pUtilCharacteristic->setCallbacks(&chrCallbacks); 

    //another sevice - 
    NimBLEService*        pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic =
        pBaadService->createCharacteristic("F00D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

    pFoodCharacteristic->setValue("Fries");
    pFoodCharacteristic->setCallbacks(&chrCallbacks);

    /** Start the services when finished creating all Characteristics and Descriptors */
    pBkmbService->start();
    pBaadService->start();

    /** Create an advertising instance and add the services to the advertised data */
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("NimBLE-Server");
    pAdvertising->addServiceUUID(pBkmbService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    Serial.printf("Advertising Started\n");
}

void loop() {


    /** Loop here and send notifications to connected peers */
    delay(2000);
    if (pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
        if (pSvc) {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
            if (pChr) {
                pChr->notify();
            }
        }
    }
}