#include <BluetoothAudio.h>

#include "tones.h"

PWMAudio pwm(14, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;


void setup() {
    Serial.begin(115200);

    a2dp.setName("PicoSpeaker");

    setup_tones(a2dp);

    a2dp.setConsumer(&consumer);
    consumer.setVolume(255);

    a2dp.begin();
}

void loop() {}
