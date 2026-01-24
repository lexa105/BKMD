#include "NimBLEDevice.h"


#define SERVER_NAME "BLE Universal Dongle"
NimBLEUUID serviceUUID("B00B");
NimBLEUUID utilUUID("1234");
NimBLEUUID dataUUID("1235");

#define LED_PIN     21     
// Change if your RGB is on a different GPIO
#define LED_COUNT   1      
// Single RGB LED
#define LED_BLINK_DELAY 250

enum DeviceState {
 SETUP_MODE,
 KEYBOARD_MODE,
 MOUSE_MODE,
 MK_MODE,
 AIRDROP_MODE,
 BLE_CONNECTION_LOST,
 BLE_CONNECTED
};

//state = modes + connection states
void setLED(uint8_t r, uint8_t g, uint8_t b);
void setLEDState(DeviceState s);
void setDisplayState(DeviceState s);
void setMode_fromByte(u_int8_t mode);
void setState(DeviceState s);

void display_recieved_data(const char* value, size_t valueLen);
void keyboard_data_handle(const char* value, size_t valueLen);
void mouse_data_handle(const char* value, size_t valueLen);
void keyboard_usageID_test(uint8_t usageID);