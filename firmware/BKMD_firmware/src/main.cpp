/* BKMD PROJECT- BLE Keymoard and Mouse Dongle
 * 
 * Creates BLE server with 2 characteristics - util and data.
 *    Util is for setting mode (keyboard, mouse, mk, airdrop)
 *    Data is for sending keyboard/mouse data
 * 
 * Modes:
 * Keyboard - sends recieved keyboard data - strokes to USB Host as HID Class
 * Mouse - sends received mouse data - movements and clicks - dx, dy, buttons, wheel
 * MK - sends received both keyboard and mouse data
 * Airdrop - Starts Wifi AP for file transfer, Mounts as USB Mass Storage, Recieved files are stored on SD and available on HOST as USB MSS
 * 
 * By DublerIno
 * 
 * Based on Adafruit Bluefruit LE example and NimBLE examples
 * used code from H2zero, Adafruit, NimBLE-Arduino
 * 
 * TODO:
 * combine all demos
 * better structure code
 * redirect serial to diffent UART - this wont work cuz usb hid
 * look into multicore procces, computing/data bottlenecks and timing
 * 
 */


#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "NimBLEDevice.h"

#include "USB.h"
#include "USBHIDMouse.h"
#include "USBHIDKeyboard.h"

/*TFT display enabled version*/
#include "display.h"



#define LED_PIN     21     // Change if your RGB is on a different GPIO
#define LED_COUNT   1      // Single RGB LED
#define LED_BLINK_DELAY 250

#define SERVER_NAME "BLE Universal Dongle"

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

NimBLEUUID serviceUUID("B00B");
NimBLEUUID utilUUID("1234");
NimBLEUUID dataUUID("1235");

USBHIDKeyboard keyboard;
USBHIDMouse mouse;

static NimBLEServer* pServer;

enum DeviceState {
 SETUP_MODE,
 DATA_RECIEVED,
 KEYBOARD_MODE,
 MOUSE_MODE,
 MK_MODE,
 AIRDROP_MODE,
 BLE_CONNECTION_LOST,
 BLE_CONNECTED
};

void setLED(uint8_t r, uint8_t g, uint8_t b);
void setLEDState(DeviceState s);
void setMode(int mode);
void setState(DeviceState s);

DeviceState currentState = SETUP_MODE;

void setState(DeviceState s) {
  currentState = s;
  setLEDState(s);
}

// Change the LED color based on the device state
void setLEDState(DeviceState s) {
  switch (s) {
    case SETUP_MODE:
      setLED(255, 255, 0);   // Yellow
      break;
    case DATA_RECIEVED:
      setLED(0, 0, 255);     // Blue
      break;
    case MOUSE_MODE:
      setLED(255, 0, 255);   // Magenta
      break;
    case MK_MODE:
      setLED(255, 255, 255);   // White
      break;
    case AIRDROP_MODE:
      setLED(255, 255, 255);   // White
      break;
    case KEYBOARD_MODE:
      setLED(0, 255, 255);     // Cyan
      break;
    case BLE_CONNECTION_LOST:
      setLED(255, 0, 0);     // Red
      break;
    case BLE_CONNECTED:
      setLED(0, 255, 0);  // Green
      break;
  }
}



/**
 * Change the LED color
 */
void setLED(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}
/*
State bude nadmoznina pro mode
0x00 	SETUP_MODE
0x01	KEYBOARD_MODE
0x02	MOUSE_MODE
0x03	MnK_MODE
0x04	AIRDROP_MODE
*/
//bude to fungovat
void setMode(int mode) {
  
  switch (mode) {
    case 0:
      setState(SETUP_MODE);
      Serial.println("SETUP_MODE");
      break;
    case 1:
      setState(KEYBOARD_MODE);
      display_show_state("Keyboard Mode");
      display_show_text("Waiting for data...");
      break;
    case 2:
      setState(MOUSE_MODE);
      Serial.println("MOUSE_MODE");
      display_show_text("Mouse Mode");
      display_init_crosshair(0,0);
      break;
    case 3:
      setState(MK_MODE);
      Serial.println("MK_MODE");
      display_show_text("MK Mode");
      break;
    case 4:
      setState(AIRDROP_MODE);
      Serial.println("AIRDROP_MODE");
      display_show_text("Airdrop Mode");
      break;
    default:
      Serial.printf("Unknown mode: %d\n", mode);
      setState(SETUP_MODE); // Default to SETUP_MODE for unknown values
      break;
  }


}
volatile bool isConnected = false; // Globální proměnná

uint32_t lastWrite = 0;

/** CALLBACKS - je to dobre reseni? */
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client address: %s\n", connInfo.getAddress().toString().c_str());
        setState(BLE_CONNECTED);
        isConnected = true;

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
        setState(BLE_CONNECTION_LOST);
        NimBLEDevice::startAdvertising();
        isConnected = false;

    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }
} serverCallbacks;

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
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
      if(UUID == utilUUID){
       setMode(value[0]); //first byte is mode
      }
      if(UUID == dataUUID){
        if(currentState == KEYBOARD_MODE){
          display_show_keyboard_text(value.c_str());
          for(size_t i = 0; i < valueLen; i++){
            Serial.printf("Key: %c\n", value[i]);
            keyboard.print(value[i]);
          }
        }
        if(currentState == MOUSE_MODE){
          //first byte is buttons, second x, third y, fourth wheel
          if(valueLen >= 4){
            uint8_t buttons = value[0];
            int8_t x = (int8_t)value[1];
            int8_t y = (int8_t)value[2];
            int8_t wheel = (int8_t)value[3];
            Serial.printf("Mouse - Buttons: %02X, X: %d, Y: %d, Wheel: %d\n", buttons, x, y, wheel);
            mouse.move(x, y, wheel);
            display_move_crosshair(x,y);
          }
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
} chrUtilCallback;

/** SETUP */
void setup() { 
  Serial.begin(115200);

  //while(Serial.available() == 0);
  led.begin();
  setState(SETUP_MODE); 
  
  //Display init
  display_init();
  display_show_state("BKMD Ready");
  display_show_text("Setup...");

  //SERVER SETUP
  NimBLEDevice::init(SERVER_NAME);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);
  //SERVICE SETUP
  NimBLEService *pService = pServer->createService(serviceUUID);
  //UTIL CHARS for configuration
  NimBLECharacteristic *pUtilCharacteristic = pService->createCharacteristic(
    utilUUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );
  pUtilCharacteristic->setValue("Setup");
  pUtilCharacteristic->setCallbacks(&chrUtilCallback); 
  //DATA CHAR for keyboard/mouse data
  NimBLECharacteristic *pDataCharacteristic = pService->createCharacteristic(
    dataUUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  ); 
  pDataCharacteristic->setValue("NULL");
  pDataCharacteristic->setCallbacks(&chrUtilCallback); //callbacks. muzu mit vic ruznych? tho

  /** Start the services when finished creating all Characteristics and Descriptors */
  pService->start();

   /** Create an advertising instance and add the services to the advertised data */
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("BLE-Dongle");
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();
  Serial.printf("Advertising Started\n");

  display_show_text("Advertising");
  keyboard.begin();
  USB.begin();
}

void loop() {
  if(isConnected) {
    setLED(0,0,millis() - lastWrite < 250 ? 255 : 0); 
    setLEDState(currentState); 
  }
 
}
