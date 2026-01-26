#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ble_server.h"
//nclude "utils/Logger.h"

static QueueHandle_t bleRxQ;

static BleServer* ble = nullptr;

static TaskHandle_t decoderTaskHandle = nullptr;


void hid_decode(BlePacket pkt);
void util_decode(BlePacket pkt);
void startTasks();
void DecoderTask(void*);

void setup() {
    Serial.begin(115200);

    //loger::begin(); // creates logQ + logger task

    //Create BLE recieved queue
    bleRxQ = xQueueCreate(16, sizeof(BlePacket));

    // start BLE
    ble = new BleServer(bleRxQ);
    ble->start();

    //start decoder task
    startTasks();
    
}

void startTasks() {
  xTaskCreatePinnedToCore(
    DecoderTask,        // task function
    "decoder",          // name
    6144,               // stack bytes (start with 6 KB)
    nullptr,            // param
    18,                 // priority (higher than UI, lower than GPIO)
    &decoderTaskHandle, // handle (optional)
    1                   // core: 0 or 1
  );
}

//should i have two seperate queues for data and util? - for each characteritics
void DecoderTask(void*) {
  BlePacket pkt;
  static uint32_t last = 0;
  for (;;) {
    if (xQueueReceive(bleRxQ, &pkt, portMAX_DELAY) == pdTRUE) {
      // decode pkt - t_ms,len,data
      if (pkt.callback == 0) {//data characteristic 
        hid_decode(pkt);
      } else { //else if(callback == 1)
        util_decode(pkt);
      }
    }
    if (millis() - last > 5000) {
      last = millis();
      Serial.printf("Decoder stack HW=%u\n", uxTaskGetStackHighWaterMark(nullptr));
    }

  }
}

void loop() {
  //vTaskDelay(pdMS_TO_TICKS(1000));
}

void hid_decode(BlePacket pkt){
  
}

void util_decode(BlePacket pkt){

}
/*
threding debug:
vTaskDelay -- delay() ale special pro rtosz
uxTaskGetStackHighWaterMark(). remaining stack space
xPortGetCoreID()
vTaskGetRunTimeStats()
//task speed neco
*/