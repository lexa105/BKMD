#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ble_server
//nclude "utils/Logger.h"

static QueueHandle_t bleRxQ;

static BleServer* ble = nullptr;

void setup() {
    Serial.begin(115200);

    //loger::begin(); // creates logQ + logger task

    //Create BLE recieved queue
    bleRxQ = xQueueCreate(16, sizeof(BlePacket));

    // start BLE
    ble = new BleServer(bleRxQ);
    ble->begin();

    // start app task(s), pass queues
    //ppTask::begin(bleRxQ /*, other queues... */);
    
}
void DecoderTask(void*) {
  BlePacket pkt;
  for (;;) {
    if (xQueueReceive(bleRxQ, &pkt, portMAX_DELAY) == pdTRUE) {
      // decode pkt.data[0..pkt.len-1]
      // emit GPIO commands to gpioCmdQ, update AppState, etc.
    }
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}



/*
threding debug:
uxTaskGetStackHighWaterMark(). remaining stack space
*/