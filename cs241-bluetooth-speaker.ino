#include <BluetoothAudio.h>
#include <hardware/pwm.h>

PWMAudio pwm(14, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;
bool connected = false;

void setup() {
    Serial.begin(115200);

    a2dp.setName("PicoSpeaker");

    a2dp.setConsumer(&consumer);
    consumer.setVolume(255);

    a2dp.begin();

    // Invert pin 14 for more power to the buzzer
    // Does this work? Who knows!
    uint slice = pwm_gpio_to_slice_num(14);
    pwm_set_output_polarity(slice, false, true);
}

void loop() {}
