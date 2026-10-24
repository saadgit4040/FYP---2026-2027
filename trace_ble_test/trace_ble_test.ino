// TRACE BLE connectivity test (no sensors needed)
// Flash the same sketch on all 4 ESP32 boards, changing only DEVICE_NAME.
// As soon as a phone connects, the board streams a readable text message at 10 Hz.
// In nRF Connect: connect -> open the service -> tap the notify (triple-arrow) icon.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===== CHANGE PER BOARD (keep the "TRACE-" prefix): =====
//   TRACE-FOOT-L   TRACE-KNEE-L   TRACE-FOOT-R   TRACE-KNEE-R
#define DEVICE_NAME "TRACE-KNEE-L"
// ========================================================

#define SEND_HZ    10
#define PERIOD_MS  (1000 / SEND_HZ)

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DATA_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *dataChar;
volatile bool connected = false;
uint32_t seq = 0, t0 = 0, lastSend = 0;

class SrvCb : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override {
    connected = true;
    seq = 0;
    t0 = millis();
    lastSend = t0;
    Serial.println("Phone connected");
  }
  void onDisconnect(BLEServer *s) override {
    connected = false;
    Serial.println("Phone disconnected, advertising again");
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init(DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new SrvCb());

  BLEService *svc = server->createService(SERVICE_UUID);
  dataChar = svc->createCharacteristic(
      DATA_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  dataChar->addDescriptor(new BLE2902());
  dataChar->setValue("waiting...");
  svc->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.printf("%s advertising\n", DEVICE_NAME);
}

void loop() {
  if (connected) {
    uint32_t now = millis();
    if (now - lastSend >= PERIOD_MS) {
      lastSend += PERIOD_MS;
      if (now - lastSend > 5 * PERIOD_MS) lastSend = now;

      // e.g. "FOOT-L 000123 t=12.3"  (max 20 chars so it fits the default BLE packet)
      char msg[21];
      snprintf(msg, sizeof(msg), "%s %06lu t=%.1f",
               DEVICE_NAME + 6,                     // skips the "TRACE-" prefix
               (unsigned long)seq++,
               (now - t0) / 1000.0);

      dataChar->setValue((uint8_t *)msg, strlen(msg));
      dataChar->notify();
    }
  }
  delay(1);
}
