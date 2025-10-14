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

#include "bkmd_device.h"
#include "display.h" //works only for -D HAS_TFT



Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

USBHIDKeyboard keyboard;
USBHIDMouse mouse;

static NimBLEServer* pServer;

Display tft; // Display object

DeviceState currentState = SETUP_MODE;
//change variable and led color
void setState(DeviceState s) {
  currentState = s;
  setLEDState(s);
  setDisplayState(s);
}

void setDisplayState(DeviceState s) {
  switch (s) {
    case SETUP_MODE:
      tft.display_show_state("SETUP MODE");
      break;
    case MOUSE_MODE:
      tft.display_show_state("MOUSE MODE");
      break;
    case MK_MODE:
      tft.display_show_state("MOUSE & KEYBOARD MODE");
      break;
    case AIRDROP_MODE:
      tft.display_show_state("AIRDROP MODE");
      break;
    case KEYBOARD_MODE:
      tft.display_show_state("KEYBOARD MODE");
      break;
    case BLE_CONNECTION_LOST:
      tft.display_show_state("BLE CONNECTION LOST");
      break;
    case BLE_CONNECTED:
      tft.display_show_state("BLE CONNECTED");
      break;
  }
}

// Change the LED color based on the device state
void setLEDState(DeviceState s) {
  switch (s) {
    case SETUP_MODE:
      setLED(255, 255, 0);   // Yellow
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

//Change the LED color
void setLED(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}
/*
State bude nadmoznina pro mode
0x00 	(0) SETUP_MODE
0x01	(1) KEYBOARD_MODE
0x02	(2) MOUSE_MODE
0x03	(3) MnK_MODE
0x04	(4) AIRDROP_MODE
*/
//bude to fungovat
void setMode_fromByte(uint8_t mode) {
  
  switch (mode) {
    case 0:
      setState(SETUP_MODE);
      tft.display_show_debug("change mode to setup");
      break;
    case 1:
      setState(KEYBOARD_MODE);
      tft.display_show_debug("change mode to keyboard");
      break;
    case 2:
      setState(MOUSE_MODE);
      tft.display_show_debug("change mode to mouse");
      break;
    case 3:
      setState(MK_MODE);
      tft.display_show_debug("Changed mode to MnK");
      break;
    case 4:
      setState(AIRDROP_MODE);
      tft.display_show_debug("changed mode to airdrop");
      break;
    default:
      tft.display_show_debug("unknown mode");
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
        tft.display_show_debug("connected");
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

      //display_recieved_data(value.c_str(), valueLen); //show on display

      if(UUID == utilUUID){
       setMode_fromByte(value[0]); //first byte is mode
      }
      if(UUID == dataUUID){
        if(currentState == KEYBOARD_MODE){
          keyboard_data_handle(value.c_str(), valueLen);
        }
        else if(currentState == MOUSE_MODE){
          mouse_data_handle(value.c_str(), valueLen);
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
  
  //Display init
  tft.display_init();
  tft.display_show_debug("Setup...");


    
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

  tft.display_show_debug("Advertising");
  keyboard.begin();
  USB.begin();
}

void loop() {
  if(isConnected) {
    setLED(0,0,millis() - lastWrite < 250 ? 255 : 0); 
    setLEDState(currentState); 
  }
 
}

void mouse_data_handle(const char* value, size_t valueLen) {
 //first byte is buttons, second x, third y, fourth wheel
          if(valueLen >= 4){
            uint8_t buttons = value[0];
            int8_t x = (int8_t)value[1];
            int8_t y = (int8_t)value[2];
            int8_t wheel = (int8_t)value[3];
            Serial.printf("Mouse - Buttons: %02X, X: %d, Y: %d, Wheel: %d\n", buttons, x, y, wheel);
            mouse.move(x, y, wheel);
          }

}
void display_recieved_data(const char* value, size_t valueLen){
  tft.display_show_debug("joo tohle jsem jeste nedodelal");
}

//recieved data will be int32_t Usage ID of one key
void keyboard_data_handle(const char* value, size_t valueLen){
  //display lenght
  tft.display_show_debug("Data len:");
  tft.display_show_debug(String(valueLen).c_str());
  if(valueLen >= 4){
    int32_t usageID = *((int32_t*)value); //pouze prvni 4 byty
    tft.display_show_debug(String(usageID).c_str());
    if(usageID != 0){
      keyboard.press(usageID);
      delay(10); //tohle je potreba jinak to nefunguje spolehlive
      keyboard.release(usageID);
    }
  } else {
    tft.display_show_debug("unknown data");
  }
}
//poslu hex 10 00 00 00 a printe to 00 00 00 10 coz je naopak takze je to nejaky divny