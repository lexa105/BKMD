/*
- NimBLE wrapper to Start server/Stop(?) server and work with:
 --queues
 -- debug messages
- would like to make the code very reusable - class/
*/

#include <Arduino.h>
#include "NimBLEDevice.h"
#include "ble_server.h"


//#include "utils/Logger.h"   // optional

//ble server class
BleServer::BleServer(QueueHandle_t rxQueue) : _rxQueue(rxQueue) {}

//Methodes defined outside the class- not a fan
// -------------------- BLE setup --------------------
void BleServer::start() {

    NimBLEDevice::init(SERVER_NAME);
    NimBLEDevice::setMTU(185); // helps if you later use longer writes; actual depends on client
    pServer = NimBLEDevice::createServer();


    pServer->setCallbacks(&_serverCallbacks);

    //SERVICE SETUP
    pService = pServer->createService(SVC_UUID);

    //UTIL CHAR
    pUtilCharacteristic = pService->createCharacteristic(
    UTIL_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pUtilCharacteristic->setValue("Setup");
    pUtilCharacteristic->setCallbacks(&_utilCallbacks); 

    //DATA CHAR
    pDataCharacteristic = pService->createCharacteristic(
        DATA_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    ); 
    pDataCharacteristic->setValue("NULL");
    pDataCharacteristic->setCallbacks(&_dataCallbacks); //callbacks. muzu mit vic ruznych? tho

   
   
    /** Start the services when finished creating all Characteristics and Descriptors */
    pService->start();

    /** Create an advertising instance and add the services to the advertised data */
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("BLE-Dongle");
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

  //option to turn of advert after connection ? 
  //LOG("advertising started")
  Serial.printf("Advertising Started\n");
   
}