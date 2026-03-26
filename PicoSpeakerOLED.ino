#include <Arduino.h>
#include <BluetoothAudio.h>
#include <hardware/pwm.h>
#include <U8g2lib.h>
#include <Wire.h>

// ── Pin Assignments ──────────────────────────────────────────────
#define PWM_PIN 14   // Audio PWM output
// OLED I2C: GP4 = SDA, GP5 = SCL

// ── Bluetooth Audio ──────────────────────────────────────────────
PWMAudio pwm(PWM_PIN, false);
BluetoothAudioConsumerPWM consumer(pwm);
A2DPSink a2dp;

// ── OLED (_2_ paged mode saves ~700 bytes of RAM) ────────────────
U8G2_SSD1306_128X64_NONAME_2_HW_I2C oled{U8G2_R0};

// ── Shared state ─────────────────────────────────────────────────
volatile bool bt_connected  = false;
volatile bool bt_playing    = false;
volatile int  bt_volume     = 0;
volatile bool displayDirty  = true;
volatile bool trackDirty    = false;

char titlePending[64]  = "";
char artistPending[64] = "";
char titleDisplay[64]  = "";
char artistDisplay[64] = "";

// ── BT Callbacks ─────────────────────────────────────────────────
void cbConnect(void *, bool connected) {
    bt_connected = connected;
    if (!connected) {
        bt_playing       = false;
        titlePending[0]  = 0;
        titleDisplay[0]  = 0;
        artistPending[0] = 0;
        artistDisplay[0] = 0;
        trackDirty       = true;
    } else {
        // Fetch metadata for whatever is already playing when we pair
        trackDirty   = true;
        displayDirty = true;
    }
    displayDirty = true;
}

void cbPlayback(void *, A2DPSink::PlaybackStatus status) {
    bt_playing   = (status == A2DPSink::PLAYING);
    displayDirty = true;
}

void cbVolume(void *, int vol) {
    bt_volume    = vol;
    displayDirty = true;
}

void cbTrackChanged(void *) {
    // Just flag that a change happened — metadata arrives in a later packet
    // so we poll trackTitle() in loop() rather than reading here
    trackDirty   = true;
    displayDirty = true;
}

// ── OLED Draw ────────────────────────────────────────────────────
void drawDisplay() {
    // Promote pending strings when data has arrived
    if (titlePending[0]) {
        strncpy(titleDisplay,  titlePending,  sizeof(titleDisplay));
        strncpy(artistDisplay, artistPending, sizeof(artistDisplay));
        titlePending[0] = 0;   // clear so we don't re-promote next frame
        trackDirty = false;
    } else if (!titleDisplay[0]) {
        strncpy(titleDisplay, "No Track", sizeof(titleDisplay));
        trackDirty = false;
    }

    int volPct = (bt_volume * 100) / 127;

    oled.firstPage();
    do {
        // ── Row 1: Connection / playback status ───────────────────
        oled.setFont(u8g2_font_6x10_tr);
        if (!bt_connected) {
            oled.drawStr(0, 10, "Searching...");
        } else {
            oled.drawStr(0, 10, bt_playing ? "\x10 Playing" : "|| Paused");
        }

        // ── Row 2: Track title ────────────────────────────────────
        oled.setFont(u8g2_font_ncenB10_tr);
        char truncTitle[22];
        strncpy(truncTitle, titleDisplay, 21);
        truncTitle[21] = 0;
        oled.drawStr(0, 28, truncTitle);

        // ── Row 3: Artist ─────────────────────────────────────────
        oled.setFont(u8g2_font_6x10_tr);
        char truncArtist[22];
        strncpy(truncArtist, artistDisplay, 21);
        truncArtist[21] = 0;
        oled.drawStr(0, 42, truncArtist);

        // ── Row 4: Volume bar ─────────────────────────────────────
        char volBuf[10];
        snprintf(volBuf, sizeof(volBuf), "Vol %3d%%", volPct);
        oled.drawStr(0, 58, volBuf);

        const int barX = 55, barY = 51, barW = 70, barH = 7;
        oled.drawFrame(barX, barY, barW, barH);
        oled.drawBox(barX, barY, (barW * volPct) / 100, barH);

    } while (oled.nextPage());

    displayDirty = false;
}

// ── Setup ─────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    Wire.setSDA(4);
    Wire.setSCL(5);
    oled.begin();

    a2dp.setName("PicoSpeaker");
    a2dp.setConsumer(&consumer);
    a2dp.onConnect(cbConnect);
    a2dp.onPlaybackStatus(cbPlayback);
    a2dp.onVolume(cbVolume);
    a2dp.onTrackChanged(cbTrackChanged);
    consumer.setVolume(255);
    a2dp.begin();

    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_output_polarity(slice, false, true);

    drawDisplay();
    Serial.println(F("PicoSpeaker ready"));
}

// ── Loop ──────────────────────────────────────────────────────────
void loop() {
    // Poll for title/artist once the metadata packet has arrived
    if (trackDirty) {
        const char *t  = a2dp.trackTitle();
        const char *ar = a2dp.trackArtist();
        if (t && t[0]) {
            strncpy(titlePending,  t,          sizeof(titlePending)  - 1);
            strncpy(artistPending, ar ? ar : "", sizeof(artistPending) - 1);
            titlePending[sizeof(titlePending)   - 1] = 0;
            artistPending[sizeof(artistPending) - 1] = 0;
            displayDirty = true;
        }
    }

    if (displayDirty || trackDirty) {
        drawDisplay();
    }
}