#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ble_server.h"

//nclude "utils/Logger.h"

#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

static QueueHandle_t bleRxQ;

static BleServer* ble = nullptr;

static TaskHandle_t decoderTaskHandle = nullptr;


void hid_decode(BlePacket pkt);
void util_decode(BlePacket pkt);
void startTasks();
void DecoderTask(void*);
USBHIDKeyboard keyboard;
USBHIDMouse mouse;

void setup() {
  Serial.begin(115200);

  //loger::begin(); // creates logQ + logger task

  //Create BLE recieved queue
  bleRxQ = xQueueCreate(16, sizeof(BlePacket));

  // start BLE
  ble = new BleServer(bleRxQ);
  ble->start();

  keyboard.begin();
  USB.begin(); // tohle nejak vypne serial1 - uart ne ? takze logger task bude na serial2

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


//TO CHANGE - WAITING FOR LEXA CODE
//combine mouse nad keyboad packet? why not
//keyboard send from PC both PRESS and RELASE PACKET
void hid_decode(BlePacket pkt){
  uint8_t usageID = pkt.data[0];
  
  //tft.display_show_debug(String(usageID).c_str());

  if(usageID != 0){
    keyboard.pressRaw(usageID);
    //delay(10); //tohle zmizi az se bude posilat i release
    vTaskDelay(10); 
    keyboard.releaseRaw(usageID);
  }

  //tft.display_show_debug("unknown data");
  
  //mouse handle 

}
//clipboard feature


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