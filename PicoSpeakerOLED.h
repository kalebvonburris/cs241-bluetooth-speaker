#pragma once
#include <U8g2lib.h>
#include <Wire.h>
#include <BluetoothAudio.h>

// Pin Assignments 
#define OLED_SDA 4
#define OLED_SCL 5

// OLED (_2_ paged mode saves ~700 bytes of RAM) 
static U8G2_SSD1306_128X64_NONAME_2_HW_I2C oled{U8G2_R0};

// Shared state 
static volatile bool bt_connected  = false;
static volatile bool bt_playing    = false;
static volatile int  bt_volume     = 0;
static volatile bool displayDirty  = true;
static volatile bool trackDirty    = false;

static char titlePending[64]  = "";
static char artistPending[64] = "";
static char titleDisplay[64]  = "";
static char artistDisplay[64] = "";

// Internal reference to a2dp so callbacks can call trackTitle() etc.
static A2DPSink *_a2dp = nullptr;


// Keep retrying for up to 3 seconds after a track/playback event
// before falling back to "Unknown"
#define METADATA_TIMEOUT_MS 3000
static unsigned long metadataPollStart = 0;
static bool          metadataWaiting   = false;

static void cbConnect(void *, bool connected) {
    bt_connected = connected;
    if (!connected) {
        bt_playing        = false;
        titlePending[0]   = 0;
        titleDisplay[0]   = 0;
        artistPending[0]  = 0;
        artistDisplay[0]  = 0;
        trackDirty        = false;
        metadataWaiting   = false;
    } else {
        // Something may already be playing when we pair
        trackDirty   = true;
        displayDirty = true;
    }
    displayDirty = true;
}

static void cbPlayback(void *, A2DPSink::PlaybackStatus status) {
    bool wasPlaying = bt_playing;
    bt_playing = (status == A2DPSink::PLAYING);
    // Trigger a metadata fetch whenever playback starts.
    // Catches sources that do not fire trackChanged.
    if (bt_playing && !wasPlaying) {
        titleDisplay[0]  = 0;
        artistDisplay[0] = 0;
        trackDirty       = true;
    }

    displayDirty = true;
}

static void cbVolume(void *, int vol) {
    bt_volume    = vol;
    displayDirty = true;
}

static void cbTrackChanged(void *) {
    // Clear stale display data immediately
    titleDisplay[0]  = 0;
    artistDisplay[0] = 0;
    trackDirty       = true;
    displayDirty     = true;
}

// ── OLED Draw ─────────────────────────────────────────────────────
static void drawDisplay() {
    // Promote pending strings if they have arrived
    if (titlePending[0]) {
        strncpy(titleDisplay,  titlePending,  sizeof(titleDisplay));
        strncpy(artistDisplay, artistPending, sizeof(artistDisplay));
        titlePending[0]  = 0;
        artistPending[0] = 0;
    }

    // Fallbacks for display buffers still empty
    const char *showTitle  = titleDisplay[0]  ? titleDisplay  : "Unknown";
    const char *showArtist = artistDisplay[0] ? artistDisplay : "Unknown";

    int volPct = (bt_volume * 100) / 127;

    oled.firstPage();
    do {
        // Connection / playback status
        oled.setFont(u8g2_font_6x10_tr);
        if (!bt_connected) {
            oled.drawStr(0, 10, "Searching...");
        } else {
            oled.drawStr(0, 10, bt_playing ? "\x10 Playing" : "|| Paused");
        }

        // Row 2: Track title 
        oled.setFont(u8g2_font_ncenB10_tr);
        char truncTitle[22];
        strncpy(truncTitle, showTitle, 21);
        truncTitle[21] = 0;
        oled.drawStr(0, 28, truncTitle);

        // Row 3: Artist
        oled.setFont(u8g2_font_6x10_tr);
        char truncArtist[22];
        strncpy(truncArtist, showArtist, 21);
        truncArtist[21] = 0;
        oled.drawStr(0, 42, truncArtist);

        // Row 4: Volume label + bar 
        char volBuf[10];
        snprintf(volBuf, sizeof(volBuf), "Vol %3d%%", volPct);
        oled.drawStr(0, 58, volBuf);

        const int barX = 55, barY = 51, barW = 70, barH = 7;
        oled.drawFrame(barX, barY, barW, barH);
        oled.drawBox(barX, barY, (barW * volPct) / 100, barH);

    } while (oled.nextPage());

    displayDirty = false;
}

// Call once in setup() — pass in your a2dp instance.
// Registers all callbacks and draws the initial "Searching..." screen.
inline void setup_oled(A2DPSink &a2dp_ref) {
    _a2dp = &a2dp_ref;

    Wire.setSDA(OLED_SDA);
    Wire.setSCL(OLED_SCL);
    oled.begin();

    a2dp_ref.onConnect(cbConnect);
    a2dp_ref.onPlaybackStatus(cbPlayback);
    a2dp_ref.onVolume(cbVolume);
    a2dp_ref.onTrackChanged(cbTrackChanged);

    drawDisplay();   // show "Searching..." immediately
}

// Call once per loop() iteration — handles metadata polling and
// OLED refresh. Only redraws when something has actually changed.
inline void loop_oled() {
    // Metadata polling with timeout
    if (trackDirty && _a2dp) {
        if (!metadataWaiting) {
            metadataPollStart = millis();
            metadataWaiting   = true;
        }

        const char *t  = _a2dp->trackTitle();
        const char *ar = _a2dp->trackArtist();
        bool gotTitle  = (t  && t[0]);
        bool gotArtist = (ar && ar[0]);

        if (gotTitle || gotArtist) {
            strncpy(titlePending,
                    gotTitle  ? t  : "Unknown",
                    sizeof(titlePending)  - 1);
            strncpy(artistPending,
                    gotArtist ? ar : "Unknown",
                    sizeof(artistPending) - 1);
            titlePending[sizeof(titlePending)   - 1] = 0;
            artistPending[sizeof(artistPending) - 1] = 0;
            trackDirty      = false;
            metadataWaiting = false;
            displayDirty    = true;

        } else if (millis() - metadataPollStart > METADATA_TIMEOUT_MS) {
            strncpy(titlePending,  "Unknown", sizeof(titlePending));
            strncpy(artistPending, "Unknown", sizeof(artistPending));
            trackDirty      = false;
            metadataWaiting = false;
            displayDirty    = true;
        }
        // else: still within window, keep retrying next loop
    }

    if (displayDirty) {
        drawDisplay();
    }
}