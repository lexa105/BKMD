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

//ble server Constructor
BleServer::BleServer(QueueHandle_t rxQueue)
: _rxQueue(rxQueue)
, _serverCallbacks(*this)
, _dataCallbacks(rxQueue)   
, _utilCallbacks(rxQueue)
{}

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
    pUtilCharacteristic->setValue("01");
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

    /** Store an advertising instance and add the services to the advertised data */
    _adv = NimBLEDevice::getAdvertising();
    _adv->setName("BLE-Dongle");
    _adv->addServiceUUID(pService->getUUID());
    _adv->enableScanResponse(true);
    _adv->start();

    _started = true;


  //option to turn of advert after connection ? 
  //LOG("advertising started")
  Serial.printf("Advertising Started\n");
   
}

 // Keep stack initialized; do not deinit/free. This is "soft" stop.
void BleServer::soft_stop(bool disconnectClient) {
  _advEnabled = false;
  if (!_started) return;

  NimBLEDevice::stopAdvertising();
  if (_adv) _adv->stop();

  if (disconnectClient && _connected && pServer && _connHandle != 0xFFFF) {
    pServer->disconnect(_connHandle);
  }
  
}


void BleServer::resume() {
  // Only makes sense if BLE stack was started before
  if (!_started) return;

  // If we are already connected, do not advertise
  if (_connected) return;

  _advEnabled = true;
  // Make sure we have an advertising object
  if (_adv == nullptr) {
    _adv = NimBLEDevice::getAdvertising();
    if (_adv == nullptr) return;
  }

  // Start advertising again (idempotent enough for typical NimBLE usage)
  _adv->start();
  // Alternatively:
  // NimBLEDevice::startAdvertising();
}
