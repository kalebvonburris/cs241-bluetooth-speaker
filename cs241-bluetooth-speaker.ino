#include <BluetoothAudio.h>
#include <hardware/pwm.h>
#include "PicoSpeakerOLED+Tones.h"
#define PWM_PIN 14   // Audio PWM output
// OLED I2C: GP4 = SDA, GP5 = SCL
// Buzzer: GP22

PWMAudio pwm(PWM_PIN, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;

void setup() {
    Serial.begin(115200);

    a2dp.setName("PicoSpeaker");
    a2dp.setConsumer(&consumer);
    consumer.setVolume(255);

    setup_oled(a2dp);   // registers all callbacks including tones

    a2dp.begin();

    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_output_polarity(slice, false, true);

    Serial.println(F("PicoSpeaker ready"));
}

void loop() {
    loop_oled();
}