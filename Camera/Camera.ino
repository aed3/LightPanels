#include "Arduino.h"
#include "esp_camera.h"

#include <BLEDevice.h>

BLEUUID SERVICE_UUID("497b89d0-4a0e-11eb-b378-0242ac130002");
BLEUUID CHARACTERISTIC_UUID_RX("0f9f307c-517f-11eb-ae93-0242ac130002");
BLEUUID CHARACTERISTIC_UUID_TX("152fd65d-4b28-453f-9d6f-bee21db16e0b");

//BLEAddress CAMERA_BT_ADDR("24:62:AB:FC:C3:B6");
BLEAddress CONTROL_BT_ADDR("40:f5:20:45:57:7e");

bool tryConnecting = false;
bool connected = false;

bool sendingColors = false;

BLERemoteCharacteristic* pRemoteRxCharacteristic;
BLERemoteCharacteristic* pRemoteTxCharacteristic;
BLEAdvertisedDevice* lightController;

void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    sendingColors = *pData ? true : false;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().equals(CONTROL_BT_ADDR)) {
      Serial.println("Connecting");
      BLEDevice::getScan()->stop();
      lightController = new BLEAdvertisedDevice(advertisedDevice);
      tryConnecting = true;
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("Connected");
    connected = true;
  }

  void onDisconnect(BLEClient* pclient) {
    Serial.println("Disonnected");
    connected = false;
    sendingColors = false;
    BLEDevice::getScan()->start(60, false);
  }
};

void sendColors(uint32_t* colors) {
  uint8_t data[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  for (uint8_t i = 0; i < 6; i++) {
    const uint8_t location = i * 3;

    data[location] = colors[i] >> 16;
    data[location + 1] = colors[i] >> 8;
    data[location + 2] = colors[i];
  }
  pRemoteTxCharacteristic->writeValue(data, 18);
}

bool connectToServer() {    
    BLEClient* pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    
    pClient->connect(lightController);
    
    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr) {
      pClient->disconnect();
      return false;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteRxCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_RX);
    pRemoteTxCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_TX);
    if (pRemoteRxCharacteristic == nullptr || pRemoteTxCharacteristic == nullptr) {
      pClient->disconnect();
      return false;
    }

    pRemoteRxCharacteristic->registerForNotify(notifyCallback);
    return true;
}

void bleStart() {
  BLEDevice::init("Light_C");
  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
 
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  Serial.println("Scan Started");
  pBLEScan->start(60, false);
  Serial.println("Scan Ended");
}

void setup() {
  Serial.begin(115200);
  bleStart();
  cameraStart();
}

void loop() {
  if (tryConnecting == true) {
    if (!connectToServer()) {
      BLEDevice::getScan()->start(60, false);
    }
    tryConnecting = false;
  }
  else if (connected && sendingColors) {
    uint32_t colors[6];
    seeColorsFromCamera(colors);
    sendColors(colors);
  }
}
