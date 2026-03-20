#include <BLE.h>
#include <WiFi.h>

#define LED_PIN 15

BLEService ledService(BLEUUID("19B10000-E8F2-537E-4F6C-D104768A1214"));
BLECharacteristic ledChar(BLEUUID("19B10001-E8F2-537E-4F6C-D104768A1214"), BLEWrite, "LED");

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    BLE.begin("PicoLED");

    ledService.addCharacteristic(&ledChar);
    BLE.server()->addService(&ledService);

    ledChar.setValue((uint8_t)0);

    ledChar.onWrite([](BLECharacteristic *c) {
        if (c->valueLen() < 1) return;

        const uint8_t *data = (const uint8_t *) c->valueData();

        uint8_t value = data[0];
        digitalWrite(LED_PIN, HIGH);
        delay(value * 1000);
        digitalWrite(LED_PIN, LOW);
    });

    BLE.startAdvertising();
    Serial.println("PicoLED ready!");
}

void loop() {
    delay(1000);
}
