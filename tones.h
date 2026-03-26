#include <BluetoothAudio.h>

inline constexpr int BUZZER_PIN = 5;

inline void buzz(int frequency, int duration, int padding=0) {
  tone(BUZZER_PIN, frequency);
  delay(duration);
  noTone(BUZZER_PIN);
  delay(padding);
}

void setup_tones(A2DPSink& a2dp) {
  a2dp.onVolume([](void *cbData, int volume) {
    buzz(400 + volume * 2, 20);
  }, nullptr);

  a2dp.onConnect([](void *cbData, bool connected) {
    if (connected) {
      buzz(750, 250, 25);
      buzz(800, 200, 30);
      buzz(820, 350);
    } else {
      buzz(800, 350, 20);
      buzz(700, 200, 20);
      buzz(600, 500);
    }
  }, nullptr);
}

