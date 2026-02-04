#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ble_callbacks.h"

//constexpr for initcialization during compile
//MTU max packet size
constexpr size_t BLE_MAX_PAYLOAD = 128;

//packet for queue inter communication
struct BlePacket {
  uint32_t t_ms;
  uint16_t len;
  uint8_t callback;
  uint8_t  data[BLE_MAX_PAYLOAD];
};



// Keep UUIDs in one place
static const NimBLEUUID SVC_UUID("B00B");
static const NimBLEUUID UTIL_UUID("1234");
static const NimBLEUUID DATA_UUID("1235");

constexpr const char* SERVER_NAME = "BLE Universal Dongle";


class BleServer {
public:
  explicit BleServer(QueueHandle_t rxQueue);
  void start();            // init + start advertising

  //soft start
  void soft_stop(bool disconnectClient /* = true */);
  void resume();
  void setConnected(uint16_t connHandle) { _connected = true; _connHandle = connHandle; }
  void setDisconnected() { _connected = false; _connHandle = 0xFFFF; }
  bool isStarted() const { return _started; }
  void setAdvEnabled(bool en) { _advEnabled = en; } //write private variable
  bool advEnabled() const { return _advEnabled; }//read private variable

private:
  QueueHandle_t _rxQueue;

  NimBLEServer* pServer = nullptr;
  NimBLEService* pService = nullptr;
  NimBLECharacteristic* pDataCharacteristic = nullptr;
  NimBLECharacteristic* pUtilCharacteristic = nullptr;


  uint32_t _seq = 0;

  // callback objects live as members => safe lifetime

  ServerCallbacks _serverCallbacks;
  CharacteristicDataCallbacks _dataCallbacks;
  CharacteristicUtilCallbacks _utilCallbacks;

  //soft stop implementation
  bool _started = false;
  bool _connected = false;
  uint16_t _connHandle = 0xFFFF;
  NimBLEAdvertising* _adv = nullptr;
  bool _advEnabled = true;   // when false, onDisconnect must NOT restart advertising

};
