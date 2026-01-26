#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ble_server.h"

#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "display.h"

#define PIN_BTN1 0

//nclude "utils/Logger.h"

#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

static QueueHandle_t bleRxQ;
static BleServer* ble = nullptr;

static TaskHandle_t decoderTaskHandle = nullptr;
static TaskHandle_t displayTaskHandle = nullptr;
static TaskHandle_t buttonTaskHandle = nullptr;

//to clean?
void startTasks();
void DecoderTask(void*);
void DisplayTask(void*);
void ButtonTask(void*);

//to learn
UiState gUi;
SemaphoreHandle_t uiMtx;
EventGroupHandle_t uiEv;
enum : EventBits_t {
  UI_EV_STATE  = (1 << 0),
  UI_EV_TEXT   = (1 << 1),
  UI_EV_DEBUG  = (1 << 2),
  UI_EV_ALL    = UI_EV_STATE | UI_EV_TEXT | UI_EV_DEBUG
};

USBHIDKeyboard keyboard;
USBHIDMouse mouse;

//to clean - move to dedicated class/header
bool hid_decode(BlePacket pkt);
void util_decode(BlePacket pkt);
static void ui_set_debug(const char* s);
static void ui_set_state(const char* s);

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
  uiMtx = xSemaphoreCreateMutex();
  uiEv  = xEventGroupCreate();

  xTaskCreatePinnedToCore(
    DecoderTask,        // task function
    "decoder",          // name
    6144,               // stack bytes (start with 6 KB)
    nullptr,            // param
    18,                 // priority (higher than UI, lower than GPIO)
    &decoderTaskHandle, // handle (optional)
    1                   // core: 0 or 1
  );

  xTaskCreatePinnedToCore(
    DisplayTask,
    "display",
    6144,     // display libs often need stack
    nullptr,
    5,        // low priority
    &displayTaskHandle,
    1         // core 1
  );

  xTaskCreatePinnedToCore(
  ButtonTask,        // task function
  "button",
  2048,              // stack (small task)
  nullptr,           // param
  8,                 // priority (below decoder, above idle)
  &buttonTaskHandle, // handle (optional)
  1                  // core 1
  );
}


//Decodes BLE packet from queue. Appplies HID. Changes Display and Util data.
void DecoderTask(void*) {
  BlePacket pkt;
  static uint32_t last = 0;
  for (;;) {
    if (xQueueReceive(bleRxQ, &pkt, portMAX_DELAY) == pdTRUE) {
      // decode pkt - t_ms,len,data
      if (pkt.callback == 0) {//data characteristic 
        bool ok = hid_decode(pkt);
        
        // 2. Update UI state
        if (ok) {
          ui_set_state("CONNECTED");
          ui_set_debug("RX OK");
        } else {
          ui_set_state("ERROR");
          ui_set_debug("BAD PACKET");
        }




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

void DisplayTask(void* arg) {
  Display disp;
  disp.display_init();

  //struct with all display data
  UiState snap;

  for (;;) {
    // Wait until something changes OR timeout for periodic refresh
    EventBits_t bits = xEventGroupWaitBits(
      uiEv,
      UI_EV_ALL,
      pdTRUE,     // clear bits on exit
      pdFALSE,    // wait for any bit
      pdMS_TO_TICKS(1000) // periodic refresh every 1s (set to portMAX_DELAY to be pure event-driven)
    );

    // Snapshot state quickly
    xSemaphoreTake(uiMtx, portMAX_DELAY);
    snap = gUi;
    xSemaphoreGive(uiMtx);

    // Render only what changed (if timeout, bits==0 => you choose what to refresh)
    if (bits & UI_EV_STATE) disp.display_show_state(snap.big);
    if (bits & UI_EV_TEXT)  disp.display_show_text(snap.text);
    if (bits & UI_EV_DEBUG) disp.display_show_debug(snap.debug);

    // If you want: always update a tiny status line/counters here on timeout too
  }
}

void ButtonTask(void*) {
  pinMode(PIN_BTN1, INPUT_PULLUP);

  const TickType_t period = pdMS_TO_TICKS(5);
  const uint32_t debounce_ms = 25;

  bool lastRead = digitalRead(PIN_BTN1);
  bool stable   = lastRead;
  uint32_t lastChangeMs = millis();

  for (;;) {
    vTaskDelay(period);

    bool r = digitalRead(PIN_BTN1);
    uint32_t now = millis();

    if (r != lastRead) {
      lastRead = r;
      lastChangeMs = now;
    }

    if ((now - lastChangeMs) >= debounce_ms && r != stable) {
      stable = r;

      if (stable == LOW) {            // pressed (pullup)
        ui_set_state("BTN PRESSED");
      } else {                        // released
        ui_set_state("BTN RELEASED");
      }
    }
  }
}


void loop() {
  //vTaskDelay(pdMS_TO_TICKS(1000));
}


//TO CHANGE - WAITING FOR LEXA CODE
//combine mouse nad keyboad packet? why not
//keyboard send from PC both PRESS and RELASE PACKET
bool hid_decode(BlePacket pkt){
  uint8_t usageID = pkt.data[0];
  
  //tft.display_show_debug(String(usageID).c_str());

  if(usageID != 0){
    keyboard.pressRaw(usageID);
    //delay(10); //tohle zmizi az se bude posilat i release
    vTaskDelay(10); 
    keyboard.releaseRaw(usageID);
  }
  return true;
  //tft.display_show_debug("unknown data");
  
  //mouse handle 

}
//clipboard feature


void util_decode(BlePacket pkt){

}


//to learn
//
static void ui_set_state(const char* s) {
  xSemaphoreTake(uiMtx, portMAX_DELAY);
  strncpy(gUi.big, s, sizeof(gUi.big)-1);
  gUi.big[sizeof(gUi.big)-1] = '\0';
  xSemaphoreGive(uiMtx);

  //wake up display task
  xEventGroupSetBits(uiEv, UI_EV_STATE);
}

static void ui_set_debug(const char* s) {
  xSemaphoreTake(uiMtx, portMAX_DELAY);
  strncpy(gUi.debug, s, sizeof(gUi.debug)-1);
  gUi.debug[sizeof(gUi.debug)-1] = '\0';
  xSemaphoreGive(uiMtx);

  xEventGroupSetBits(uiEv, UI_EV_DEBUG);
}

