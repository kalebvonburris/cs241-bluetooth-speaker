#include <BLE.h>

#define BUZZER_PIN 15

// Custom BLE service and characteristic
BLEService soundService(BLEUUID("19B10000-E8F2-537E-4F6C-D104768A1214"));
BLECharacteristic noteChar(BLEUUID("19B10001-E8F2-537E-4F6C-D104768A1214"), BLEWrite, "Note");

void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Start BLE with device name
  BLE.begin("PicoSpeaker");

  // Add characteristic to service
  soundService.addCharacteristic(&noteChar);

  // Add service to server
  BLE.server()->addService(&soundService);

  // Set initial value
  noteChar.setValue((uint8_t)0);

  // Callback fires every time iPhone writes a value
  noteChar.onWrite([](BLECharacteristic * c) {
    // get raw bytes
    const uint8_t* data = (const uint8_t*)c->value().c_str();
    int len = c->value().length();

    if (len < 4) {
      Serial.println("Bad packet, ignoring");
      return;
    }

    uint16_t frequency = ((uint16_t)data[0] << 8) | data[1];
    uint16_t duration  = ((uint16_t)data[2] << 8) | data[3];

    // clamp to safe ranges
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

  // Start advertising
  BLE.startAdvertising();

  Serial.println("PicoSpeaker ready!");
  Serial.println("Waiting for iPhone...");
}

void loop() {
  // Flash LED slowly while waiting for connection
  // connected LED handled via BLE callbacks
  delay(1000);
}
```

---

## Differences in syntax From Before using the BLE on arduino IDE 

| Old (wrong) | New (correct) |
|---|---|
| `#include <ArduinoBLE.h>` | `#include <BLE.h>` |
| `BLE.setLocalName()` | `BLE.begin("PicoSpeaker")` |
| `BLE.addService()` | `BLE.server()->addService(&service)` |
| `BLE.advertise()` | `BLE.startAdvertising()` |
| Polling in loop | `noteChar.onWrite()` callback |

---

## Test It
Flash this, open Serial Monitor at 9600 baud, you should see:
```
PicoSpeaker ready!
Waiting for iPhone...
