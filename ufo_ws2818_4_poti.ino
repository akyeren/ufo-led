
#include <FastLED.h>

// ********************************************************
//                    !!! Important !!!
//       Install the library!
//       Sketch -> include Library -> Manage Libraries
//       search for 'fastled'
//       install the library
//
//       by OneIdMONstr
// ********************************************************

// ***************************************************************
// ********* CONFIGURE *******************************************
// ***************************************************************

const bool gUsePots = true;  // false = manual mode, true = use potentiometers

#define PULSE_SPEED_MAX 600
#define PULSE_SPEED_INIT 50  // 0 .. 600 Pulse Speed, > 600 no Pulse
#define BRIGHTNESS_INIT 170  // 0 .. 255

// ***************************************************************
// ***************************************************************
// ***************************************************************
// Animation settings
#define LED_BRIGHTNESS 255
#define LEVEL_MAX 255

// Ring sizes
#define NUM_LEDS_LARGE_RING 30
#define NUM_LEDS_SMALL_RING 7
#define SEGMENT_SIZE 3  // # of groups to divide the large ring into

#define NUM_LEDS_LARGE_SEGMENT = NUM_LEDS_LARGE_RING / SEGMENT_SIZE

// Pin configuration
#define PIN_LED_LARGE_RING 4  // D4
#define PIN_LED_SMALL_RING 6  // D6
#define PIN_POT_PULSE_SPEED A0
#define PIN_POT_LARGE_COLOR A2
#define PIN_POT_SMALL_COLOR A4
#define PIN_POT_BRIGHTNESS A6

// LED HW settings
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

// Globals
CRGB ledsLargeRing[NUM_LEDS_LARGE_RING];
CRGB ledsSmallRing[NUM_LEDS_SMALL_RING];
int gPulseSpeed = PULSE_SPEED_INIT;
int gBrightness = BRIGHTNESS_INIT;
// 0   = White
// 32  = Orange
// 64  = Yellow
// 96  = Green
// 128 = Aqua
// 160 = Blue
// 192 = Purple
// 224 = Pink
int gHueSmallRing = 96;   // 0 .. 255
int gHueLargeRing = 192;  // 0 .. 255

// **************************************************************************************************************
void setup() {
    delay(3000);  // power-up safety delay

    FastLED.addLeds<LED_TYPE, PIN_LED_SMALL_RING, COLOR_ORDER>(
        ledsSmallRing, NUM_LEDS_SMALL_RING);  //.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(LED_BRIGHTNESS);

    FastLED.addLeds<LED_TYPE, PIN_LED_LARGE_RING, COLOR_ORDER>(
        ledsLargeRing, NUM_LEDS_LARGE_RING);  //.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(LED_BRIGHTNESS);

    Serial.begin(9600);
}

// **************************************************************************************************************
void setSmallRing(unsigned speed) {
    static int iRing[] = {0, 0, 100, 0, 0, 0, 0};
    static int iPos = 2;

    if (iRing[iPos % NUM_LEDS_SMALL_RING] >= 200) {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] = 0;
        iPos++;
    }

    if (iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] > 0) {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] -= speed;
    } else {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] = 0;
    }

    if (iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] > 0) {
        iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] -= speed;
    } else {
        iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] = 0;
    }

    iRing[(iPos + 0) % NUM_LEDS_SMALL_RING] += speed;

    // Set ring
    for (unsigned k = 0; k < NUM_LEDS_SMALL_RING; k++) {
        Serial.print(" ");
        Serial.print(iRing[k]);

        if (iRing[k] < 0) {
            iRing[k] = 0;
        }

        if (iRing[k] < 20) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            if (gBrightness <= 1) {
                ledsSmallRing[k] = CRGB::Black;
            } else {
                if (gHueSmallRing >= 1) {
                    ledsSmallRing[k] = CHSV(gHueSmallRing, 255, iRing[k]);
                } else {
                    ledsSmallRing[k] = CHSV(gHueSmallRing, 0, iRing[k]);
                }

                ledsSmallRing[k].fadeLightBy(255 - gBrightness);
            }
        }
    }

    Serial.println(" ");
    // delay(20);
    return;
}

// **************************************************************************************************************
void showSolid(void) {
    for (unsigned k = 0; k < NUM_LEDS_LARGE_RING; k++) {
        if (gHueLargeRing >= 1) {
            ledsLargeRing[k] = CHSV(gHueLargeRing, 255, 255);
        } else {
            ledsLargeRing[k] = CHSV(gHueLargeRing, 0, 255);
        }

        if (gBrightness <= 1) {
            ledsLargeRing[k] = CRGB::Black;
        } else {
            ledsLargeRing[k].fadeLightBy(255 - gBrightness);
        }
    }

    for (unsigned k = 0; k < NUM_LEDS_SMALL_RING; k++) {
        if (gHueSmallRing >= 1) {
            ledsSmallRing[k] = CHSV(gHueSmallRing, 255, 255);
        } else {
            ledsSmallRing[k] = CHSV(gHueSmallRing, 0, 255);
        }

        if (gBrightness <= 1) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            ledsSmallRing[k].fadeLightBy(255 - gBrightness);
        }
    }

    FastLED.show();
    delay(100);

out:
    return;
}

// **************************************************************************************************************
void showRunningLights(unsigned speed, unsigned largeRingIndex, int level) {
    int a, b, c;
    int sat = 0;
    const unsigned smallRingSpeed = max(1, speed / 5);

    // Large Ring
    a = level;
    b = level + (LEVEL_MAX / 3);
    c = level + (LEVEL_MAX / 3) * 2;

    if (gHueLargeRing >= 1) {
        sat = 255;
    }

    for (unsigned seg = 0; seg < SEGMENT_SIZE; seg++) {
        const unsigned index = largeRingIndex + (seg * NUM_LEDS_LARGE_SEGMENT);
        ledsLargeRing[(index + 0) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(LEVEL_MAX - c, gBrightness));
        ledsLargeRing[(index + 1) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(LEVEL_MAX - b, gBrightness));
        ledsLargeRing[(index + 2) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(LEVEL_MAX - a, gBrightness));
        ledsLargeRing[(index + 3) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(c, gBrightness));
        ledsLargeRing[(index + 4) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(b, gBrightness));
        ledsLargeRing[(index + 5) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(a, gBrightness));
    }

    // Small ring
    setSmallRing(smallRingSpeed);

    for (unsigned seg = 0; seg < SEGMENT_SIZE; seg++) {
        ledsLargeRing[(largeRingIndex + (seg * NUM_LEDS_LARGE_SEGMENT)) % NUM_LEDS_LARGE_RING] =
            CRGB::Black;
    }

    FastLED.show();
    delay(10);
}

// **************************************************************************************************************
void loop() {
    int speed = 1;

    for (unsigned largeRingIndex = 0; largeRingIndex < NUM_LEDS_LARGE_RING; i++) {
        for (int level = 0; level < LEVEL_MAX / 3; level += speed) {
            if (gUsePots) {
                gPulseSpeed = 1024 - analogRead(PIN_POT_PULSE_SPEED);
                gHueSmallRing = 252 - map(analogRead(PIN_POT_SMALL_COLOR), 0, 1024, 0, 255);
                gHueLargeRing = 252 - map(analogRead(PIN_POT_LARGE_COLOR), 0, 1024, 0, 255);
                gBrightness = 254 - map(analogRead(PIN_POT_BRIGHTNESS), 0, 1024, 0, 255);
            }

            if (gPulseSpeed > PULSE_SPEED_MAX) {
                showSolid();
                goto out;
            } else {
                speed = max(1, map(gPulseSpeed, 0, PULSE_SPEED_MAX, 0, 150));
                showRunningLights(speed, largeRingIndex, level);
            }
        }
    }

out:
    delay(1);
}
