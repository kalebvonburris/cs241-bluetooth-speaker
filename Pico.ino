#include <BLE.h>

#define BUZZER_PIN 15

BLEService soundService(BLEUUID("19B10000-E8F2-537E-4F6C-D104768A1214"));
BLECharacteristic noteChar(BLEUUID("19B10001-E8F2-537E-4F6C-D104768A1214"), BLEWrite, "Note");

void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  BLE.begin("PicoSpeaker");

  soundService.addCharacteristic(&noteChar);
  BLE.server()->addService(&soundService);
  noteChar.setValue((uint8_t)0);

  noteChar.onWrite([](BLECharacteristic * c) {
    // correct public methods from header file
    size_t len = c->valueLen();
    const uint8_t* data = (const uint8_t*)c->valueData();

    if (len < 4) {
      Serial.println("Bad packet, ignoring");
      return;
    }

    uint16_t frequency = ((uint16_t)data[0] << 8) | data[1];
    uint16_t duration  = ((uint16_t)data[2] << 8) | data[3];

    if (duration  > 3000) duration  = 3000;
    if (frequency > 5000) frequency = 5000;

    Serial.print("Freq: ");
    Serial.print(frequency);
    Serial.print("hz  Dur: ");
    Serial.print(duration);
    Serial.println("ms");

    if (frequency == 0) {
      noTone(BUZZER_PIN);
      delay(duration);
    } else {
      tone(BUZZER_PIN, frequency, duration);
      delay(duration);
      noTone(BUZZER_PIN);
    }
  });

  BLE.startAdvertising();

  Serial.println("PicoSpeaker ready!");
  Serial.println("Waiting for iPhone...");
}

void loop() {
  delay(1000);
}
