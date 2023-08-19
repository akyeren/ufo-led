
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
#define PULSE_SPEED_DEFAULT 50

int gPulseSpeed = 50;   // 0 .. 600 Pulse Speed, > 600 no Pulse
int gBrightness = 170;  // 0 .. 255

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

// ***************************************************************
// ***************************************************************
// ***************************************************************

#define LED_BRIGHTNESS 255
#define L_MAX 255
#define SEGMENT_SIZE 3

#define LED_PIN_LARGE_RING 4
#define LED_PIN_SMALL_RING 6

#define NUM_LEDS_LARGE_RING 30
#define NUM_LEDS_SMALL_RING 7

#define POTI_PULSE_SPEED A0
#define POTI_LARGE_COLOR A2
#define POTI_SMALL_COLOR A4
#define POTI_BRIGHTNESS A6

#define LED_TYPE WS2811
#define COLOR_ORDER GRB
CRGB ledsLargeRing[NUM_LEDS_LARGE_RING];
CRGB ledsSmallRing[NUM_LEDS_SMALL_RING];

// **************************************************************************************************************
void setup() {
    delay(3000);  // power-up safety delay

    FastLED.addLeds<LED_TYPE, LED_PIN_SMALL_RING, COLOR_ORDER>(
        ledsSmallRing, NUM_LEDS_SMALL_RING);  //.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(LED_BRIGHTNESS);

    FastLED.addLeds<LED_TYPE, LED_PIN_LARGE_RING, COLOR_ORDER>(
        ledsLargeRing, NUM_LEDS_LARGE_RING);  //.setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(LED_BRIGHTNESS);

    Serial.begin(9600);
}

// **************************************************************************************************************
void setSmallRing(int iSpeed) {
    static int iRing[] = {0, 0, 100, 0, 0, 0, 0};
    static int iPos = 2;

    if (iRing[iPos % NUM_LEDS_SMALL_RING] >= 200) {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] = 0;
        iPos++;
    }

    if (iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] > 0) {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] -= iSpeed;
    } else {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] = 0;
    }

    if (iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] > 0) {
        iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] -= iSpeed;
    } else {
        iRing[(iPos - 1) % NUM_LEDS_SMALL_RING] = 0;
    }

    iRing[(iPos + 0) % NUM_LEDS_SMALL_RING] += iSpeed;

    // Ring belegen
    for (int k = 0; k < NUM_LEDS_SMALL_RING; k++) {
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
    for (int k = 0; k < NUM_LEDS_LARGE_RING; k++) {
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

    for (int k = 0; k < NUM_LEDS_SMALL_RING; k++) {
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
void showRunningLights(int speed, int i, int j) {
    int a, b, c;
    int sat = 0;
    const int smallRingSpeed = max(1, speed / 5);

    // Large Ring
    a = j;
    b = j + (L_MAX / 3);
    c = j + (L_MAX / 3) * 2;

    if (gHueLargeRing >= 1) {
        sat = 255;
    }

    for (int k = 0; k < SEGMENT_SIZE; k++) {
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 0) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(L_MAX - c, gBrightness));
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 1) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(L_MAX - b, gBrightness));
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 2) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(L_MAX - a, gBrightness));
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 3) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(c, gBrightness));
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 4) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(b, gBrightness));
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) + 5) % NUM_LEDS_LARGE_RING] =
            CHSV(gHueLargeRing, sat, min(a, gBrightness));
    }

    // Small ring
    setSmallRing(smallRingSpeed);

    for (int k = 0; k < SEGMENT_SIZE; k++) {
        ledsLargeRing[((NUM_LEDS_LARGE_RING + i) + (k * NUM_LEDS_LARGE_RING / SEGMENT_SIZE)) % NUM_LEDS_LARGE_RING] =
            CRGB::Black;
    }

    FastLED.show();
    delay(10);
}

// **************************************************************************************************************
void loop() {
    int speed = 1;

    for (int i = 0; i < NUM_LEDS_LARGE_RING; i++) {
        for (int j = 0; j < L_MAX / 3; j += speed) {
            if (gUsePots) {
                gPulseSpeed = 1024 - analogRead(POTI_PULSE_SPEED);
                gHueSmallRing = 252 - map(analogRead(POTI_SMALL_COLOR), 0, 1024, 0, 255);
                gHueLargeRing = 252 - map(analogRead(POTI_LARGE_COLOR), 0, 1024, 0, 255);
                gBrightness = 254 - map(analogRead(POTI_BRIGHTNESS), 0, 1024, 0, 255);
            }

            if (gPulseSpeed > 600) {
                showSolid();
                goto out;
            } else {
                speed = max(1, map(gPulseSpeed, 0, PULSE_SPEED_MAX, 0, 150));
                showRunningLights(speed, i, j);
            }
        }
    }

out:
    delay(1);
}
