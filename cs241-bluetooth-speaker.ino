#include <BluetoothAudio.h>
#include <hardware/pwm.h>
#include "PicoSpeakerOLED.h"
#define PWM_PIN 14  

// OLED I2C: GP4 = SDA, GP5 = SCL  (defined inside PicoSpeakerOLED.h)
PWMAudio pwm(PWM_PIN, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;

void setup() {
    Serial.begin(115200);

    a2dp.setName("PicoSpeaker");
    a2dp.setConsumer(&consumer);
    consumer.setVolume(255);

    // Hand a2dp to the OLED module 
    setup_oled(a2dp);

    a2dp.begin();

    // Invert complement PWM 
    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_output_polarity(slice, false, true);

    Serial.println(F("PicoSpeaker ready"));
}

void loop() {
    loop_oled();   // metadata polling and OLED refresh
}
