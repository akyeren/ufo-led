
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

#define READ_POTS  // undef = manual mode, def = use potentiometers
#define PULSE_SPEED_MAX 600
#define PULSE_SPEED_INIT 50  // 0 .. 600 Pulse Speed, > 600 no Pulse
#define BRIGHTNESS_INIT 170  // 0 .. 255
#define DBG_PRINT            // printing debug to serial message

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
word gPulseSpeed = PULSE_SPEED_INIT;
byte gBrightness = BRIGHTNESS_INIT;

// Hue
byte gHueSmallRing = HUE_GREEN;
byte gHueLargeRing = HUE_PURPLE;
// https://github.com/FastLED/FastLED/blob/master/src/pixeltypes.h#L109
// 0   = HUE_RED
// 32  = HUE_ORANGE
// 64  = HUE_YELLOW
// 96  = HUE_GREEN
// 128 = HUE_AQUA
// 160 = HUE_BLUE
// 192 = HUE_PURPLE
// 224 = HUE_PINK

// Macros
#define MakeHsv(h, v) CHSV(h, (h >= 1) ? 255 : 0, v)

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
    static byte iPos = 2;

    if (iRing[iPos % NUM_LEDS_SMALL_RING] >= 200) {
        iRing[(iPos - 2) % NUM_LEDS_SMALL_RING] = 0;
        ++iPos;
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
    for (byte k = 0; k < NUM_LEDS_SMALL_RING; ++k) {
#ifdef DBG_PRINT
        Serial.print(" ");
        Serial.print(iRing[k]);
#endif

        if (iRing[k] < 0) {
            iRing[k] = 0;
        }

        if (iRing[k] < 20) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            if (gBrightness <= 1) {
                ledsSmallRing[k] = CRGB::Black;
            } else {
                ledsSmallRing[k] = MakeHsv(gHueSmallRing, iRing[k]);
                ledsSmallRing[k].fadeLightBy(255 - gBrightness);
            }
        }
    }

#ifdef DBG_PRINT
    Serial.println(" ");
#endif
}

// **************************************************************************************************************
void showSolid() {
    for (byte k = 0; k < NUM_LEDS_LARGE_RING; ++k) {
        ledsLargeRing[k] = MakeHsv(gHueLargeRing, 255);
        if (gBrightness <= 1) {
            ledsLargeRing[k] = CRGB::Black;
        } else {
            ledsLargeRing[k].fadeLightBy(255 - gBrightness);
        }
    }

    for (byte k = 0; k < NUM_LEDS_SMALL_RING; ++k) {
        ledsSmallRing[k] = MakeHsv(gHueSmallRing, 255);
        if (gBrightness <= 1) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            ledsSmallRing[k].fadeLightBy(255 - gBrightness);
        }
    }

    FastLED.show();
    delay(100);
}

// **************************************************************************************************************
void showRunningLights(word speed, byte largeRingIndex, byte level) {
    // Large Ring
    const byte a = level;
    const byte b = level + (LEVEL_MAX / 3);
    const byte c = level + (LEVEL_MAX / 3) * 2;
    const byte sat = gHueLargeRing < 1 ? 0 : 255;
    const byte NUM_LEDS = 6;
    const byte levels[] = {LEVEL_MAX - c, LEVEL_MAX - b, LEVEL_MAX - a,
                           c, b, a};  // NUM_LEDS

    for (byte seg = 0; seg < SEGMENT_SIZE; ++seg) {
        const byte index = largeRingIndex + seg * NUM_LEDS_LARGE_SEGMENT;
        for (byte offset = 0; offset < NUM_LEDS; ++offset) {
            ledsLargeRing[(index + offset) % NUM_LEDS_LARGE_RING] =
                CHSV(gHueLargeRing, sat, min(levels[offset], gBrightness));
        }
    }

    // Small ring
    const word smallRingSpeed = max(1, speed / 5);
    setSmallRing(smallRingSpeed);

    for (byte seg = 0; seg < SEGMENT_SIZE; ++seg) {
        ledsLargeRing[(largeRingIndex + seg * NUM_LEDS_LARGE_SEGMENT) % NUM_LEDS_LARGE_RING] =
            CRGB::Black;
    }

    FastLED.show();
    delay(10);
}

void readPots() {
    gPulseSpeed = 1024 - analogRead(PIN_POT_PULSE_SPEED);
    gHueSmallRing = 252 - map(analogRead(PIN_POT_SMALL_COLOR), 0, 1024, 0, 255);
    gHueLargeRing = 252 - map(analogRead(PIN_POT_LARGE_COLOR), 0, 1024, 0, 255);
    gBrightness = 254 - map(analogRead(PIN_POT_BRIGHTNESS), 0, 1024, 0, 255);
}

// **************************************************************************************************************
void loop() {
    word speed = 1;
    bool finishLoop = false;

    for (byte largeRingIndex = 0; largeRingIndex < NUM_LEDS_LARGE_RING && !finishLoop; ++largeRingIndex) {
        for (byte level = 0; level < LEVEL_MAX / 3; level += speed) {
#ifdef READ_POTS
            readPots();
#endif
            if (gPulseSpeed > PULSE_SPEED_MAX) {
                showSolid();
                finishLoop = true;
                break;
            } else {
                speed = max(1, map(gPulseSpeed, 0, PULSE_SPEED_MAX, 0, 150));
                showRunningLights(speed, largeRingIndex, level);
            }
        }
    }

    delay(1);
}
