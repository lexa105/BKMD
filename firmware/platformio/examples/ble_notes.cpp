#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "NimBLEDevice.h"

#define LED_PIN     21     // Change if your RGB is on a different GPIO
#define LED_COUNT   1      // Single RGB LED

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
void setLED(uint8_t r, uint8_t g, uint8_t b);
void setDebugState(uint8_t r, uint8_t g, uint8_t b);

enum DebugState {
  INIT,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  ERROR_STATE,
  BLE_CONNECTED
};

void setDebugState(DebugState s) {
  switch (s) {
    case INIT:
      setLED(255, 255, 0);   // Yellow
      break;
    case WIFI_CONNECTING:
      setLED(0, 0, 255);     // Blue
      break;
    case WIFI_CONNECTED:
      setLED(0, 255, 0);     // Green
      break;
    case ERROR_STATE:
      setLED(255, 0, 0);     // Red
      break;
    case BLE_CONNECTED:
      setLED(255, 0, 255);   // Magenta
      break;
  }
}

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}


void setup() {
  Serial.begin(115200);
  while(Serial.available() == 0);
  led.begin();
  led.show();
  
  //TEST OF DEBUG LED
  setDebugState(INIT);
  delay(1000);
  setDebugState(WIFI_CONNECTING);
  delay(2000);
  // pretend WiFi connected
  setDebugState(WIFI_CONNECTED);
  delay(2000);
  // pretend error
  setDebugState(ERROR_STATE);
  delay(2000);
  
  //BLE init
  NimBLEDevice::init("Advertised Name");//:: syntax cuz its one global object, no need for instances
  //. by tam byla kdyby to byl objekt - instance 
  //-> kdyby to bylo pres pointer = dynamicky alokovany objekt pomoci funkce "new"

  //bud je server nebo client 
  //createServer() vraci pointer a ten priradime k proměne typu NimBLEServer 
  NimBLEServer *pServer = NimBLEDevice::createServer();
  //pServer pak pouzivame pres -> cuz je to pointer
  //vytvorime service ktery server hosti. a ma dane UUID ABCD
  NimBLEService *pService = pServer->createService("ABCD");
  //charasteric of sevice
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
    "1234",
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  ); //zaroven jsme dali defaultni Property - coz je read write
  pService->start();
  
  pCharacteristic->setValue("Hello BLE");


  //Service a Char. identifikovane pres UUID
  //vytvorene na strane Serveru a oba do nich muzou zapisovat
  //UUID typu public (dane standartem - 16bits) a private 

  
  //advertising - jaky je v tom rozdil?  
  //SERVER posila adveritse packety aby o nem ostatni vedeli, CLIENT packety zachytavaji
  //CLIENT se pripoji na SERVER a zjisti si services, charactericts, pro komunikaci
  //services sdruzuje vice characterictis do "skupin"
  //lze si dat funkci notify - na vybranou char.
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("ABCD"); // advertise the UUID of our service
  pAdvertising->setName("NimBLE"); // advertise the device name
  pAdvertising->start(); 
  
  Serial.println("zkus to prepsat");
  Serial.println(String("Prectena hodnota: ") + pCharacteristic->getValue());
}

void loop() {
  led.setPixelColor(0, led.Color(100, 0, 0));  // Red
  led.show();

  
}
