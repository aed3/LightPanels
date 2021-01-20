#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>

BLEServer* pServer = NULL;
bool controllerConnected = false;
bool oldControllerConnected = false;
bool cameraConnected = false;
bool matchingCamera = false;
int cameraID = -1;
BLECharacteristic* pCamTxCharacteristic = NULL;

bool sending = false;

BLEUUID SERVICE_UUID ("497b89d0-4a0e-11eb-b378-0242ac130002");
BLEUUID CHARACTERISTIC_UUID_CONTROLL_RX ("4f32d61c-4a0e-11eb-b378-0242ac130002");
BLEUUID CHARACTERISTIC_UUID_CAM_RX ("152fd65d-4b28-453f-9d6f-bee21db16e0b");
BLEUUID CHARACTERISTIC_UUID_CAM_TX ("0f9f307c-517f-11eb-ae93-0242ac130002");

esp_bd_addr_t CAMERA_BT_ADDR = {0x24, 0x62, 0xAB, 0xFC, 0xC3, 0xB6};

volatile bool resetCalled = false;
bool poweredOn = true;
const uint8_t powerOffData[8] = {16, 0, 0, 0, 0, 0, 0, 0};
const uint8_t powerOnData[8] = {16, 75, 75, 75, 0, 0, 0, 0};
const uint8_t resetPin = 33;

bool compareBtAddr(esp_bd_addr_t a, esp_bd_addr_t b) {
  return memcmp(a, b, 6) == 0;
}

void startMatchingCamera() {
  if (!matchingCamera && cameraConnected) {
    int val = 255;
    pCamTxCharacteristic->setValue(val);
    pCamTxCharacteristic->notify();
    matchingCamera = true;
  }
}

void stopMatchingCamera() {
  if (matchingCamera && cameraConnected) {
    int val = 0;
    pCamTxCharacteristic->setValue(val);
    pCamTxCharacteristic->notify();
    matchingCamera = false;
  }
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
    if (compareBtAddr(CAMERA_BT_ADDR, param->connect.remote_bda)) {
      cameraConnected = true;
      cameraID = param->connect.conn_id;
      if (!controllerConnected) {
        pServer->startAdvertising();
      }
    }
    else {
      controllerConnected = true;
      if (!cameraConnected) {
        pServer->startAdvertising();
      }
      if (!poweredOn) {
        resetCalled = true;
      }
    }
  };

  void onDisconnect(BLEServer* pServer) {
    auto deviceMap = pServer->getPeerDevices(true);
    Serial.println(deviceMap.size());
    if (cameraID != -1 && deviceMap.find(cameraID) == deviceMap.end()) {
      cameraConnected = false;
      cameraID = -1;
      delay(500);
      pServer->startAdvertising();
    }
    else {
      controllerConnected = false;
    }
  }
};

class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    if (!sending) {
      sending = true;
      std::string rxValue = pCharacteristic->getValue();
      const int length = rxValue.length();
      const uint8_t* data = (const uint8_t*)rxValue.c_str();

      if (length == 1 && data[0]) {
        switch (*data) {
          case 255: 
            resetCalled = true;
            break;
          case 0b0000100:
            stopMatchingCamera();
            break;
          case 0b1111000:
            startMatchingCamera() ;
            break;
        }
        sending = false;
        return;
      }

      stopMatchingCamera();
      Wire.beginTransmission(1);
      if (length == 1 && data[0] == 0) {
        Wire.write(poweredOn ? powerOffData : powerOnData, 8);
        poweredOn = !poweredOn;
      }
      else {
        Wire.write(data, length);
        poweredOn = true;
      }

      Wire.endTransmission();
      sending = false;
    }
  }
};

class CamCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    if (!sending) {
      sending = true;
      const uint8_t* data = pCharacteristic->getData();
      uint8_t dataWithFn[19];
      dataWithFn[0] = 0b1111100;

      for (int i = 1; i < 19; i++) {
        dataWithFn[i] = data[i - 1];
      }

      Wire.beginTransmission(1);
      Wire.write(dataWithFn, 19);
      Wire.endTransmission();
      sending = false;
    }
  }
};

void setup() {
  // Serial.begin(115200);
  Wire.begin();
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);

  BLEDevice::init("Light_Lights");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCamTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_CAM_TX, BLECharacteristic::PROPERTY_NOTIFY);

  pCamTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic* pCamRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_CAM_RX, BLECharacteristic::PROPERTY_WRITE);

  pCamRxCharacteristic->setCallbacks(new CamCallbacks());

  BLECharacteristic* pControlRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_CONTROLL_RX, BLECharacteristic::PROPERTY_WRITE);

  pControlRxCharacteristic->setCallbacks(new ControlCallbacks());

  pService->start();
  pServer->startAdvertising();
}

void loop() {
  if (!controllerConnected && oldControllerConnected) {
    delay(500);
    pServer->startAdvertising();
    oldControllerConnected = controllerConnected;
  }

  if (controllerConnected && !oldControllerConnected) {
    oldControllerConnected = controllerConnected;
  }

  if (resetCalled) {
    digitalWrite(resetPin, LOW);
    delay(500);
    digitalWrite(resetPin, HIGH);
    poweredOn = true;
    resetCalled = false;
  }
}
