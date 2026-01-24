#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ble_callbacks.h"

//constexpr for initcialization during compile
constexpr size_t BLE_MAX_PAYLOAD = 128;

struct BlePacket {
  uint32_t seq;
  uint32_t t_ms;
  uint16_t len;
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
  void end();              // optional
  bool enqueueFromCallback(const uint8_t* data, size_t len); // called by callbacks

private:
  QueueHandle_t _rxQueue;

  NimBLEServer* pServer = nullptr;
  NimBLEService* pService = nullptr;
  NimBLECharacteristic* pDataCharacteristic = nullptr;
  NimBLECharacteristic* pUtilCharacteristic = nullptr;


  uint32_t _seq = 0;

  // callback objects live as members => safe lifetime
  class ServerCbs;
  class RxCharCbs;

  ServerCallbacks _serverCallbacks;
  CharacteristicDataCallbacks _dataCallbacks;
  CharacteristicUtilCallbacks _utilCallbacks;
};
