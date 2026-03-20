#include <BluetoothAudio.h>

PWMAudio pwm(14, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;
bool connected = false;

void setup() {
    set_sys_clock_khz(250000, true);
    Serial.begin(115200);
    delay(3000);

    a2dp.setName("PicoSpeaker");
    a2dp.setConsumer(&consumer);
    consumer.setVolume(255);
    a2dp.onConnect([](void *cbData, bool isConnected) {
        connected = isConnected;
        Serial.println(isConnected ? "Phone connected!" : "Phone disconnected.");
    }, nullptr);

    a2dp.onPlaybackStatus([](void *cbData, A2DPSink::PlaybackStatus status) {
        Serial.print("Playback status: ");
        Serial.println((int)status);
    }, nullptr);

    bool ok = a2dp.begin();
    Serial.print("begin(): ");
    Serial.println(ok ? "true" : "false");
}

void loop() {
    Serial.println(connected ? "Connected" : "Waiting...");
    delay(1000);
}
