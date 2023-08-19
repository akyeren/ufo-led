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

#define NUM_LEDS_LARGE_SEGMENT NUM_LEDS_LARGE_RING / SEGMENT_SIZE

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

struct Pots {
    word pulseSpeed = PULSE_SPEED_INIT;
    byte brightness = BRIGHTNESS_INIT;
    byte hueSmallRing = HUE_GREEN;
    byte hueLargeRing = HUE_PURPLE;
    // https://github.com/FastLED/FastLED/blob/master/src/pixeltypes.h#L109
    // 0   = HUE_RED
    // 32  = HUE_ORANGE
    // 64  = HUE_YELLOW
    // 96  = HUE_GREEN
    // 128 = HUE_AQUA
    // 160 = HUE_BLUE
    // 192 = HUE_PURPLE
    // 224 = HUE_PINK
};

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

void dimRingLed(int ring[], byte size, byte index, byte offset, byte dec) {
    const auto target = (size + index - offset) % size;
    ring[target] -= min(ring[target], dec);
}

// **************************************************************************************************************
void setSmallRing(byte speed, const Pots& pots) {
    static int ring[] = {0, 0, 100, 0, 0, 0, 0};
    static auto index = 2;  // 0 .. NUM_LEDS_SMALL_RING - 1
    static const auto MaxVal = 200;

    if (ring[index] >= MaxVal) {
        ring[(NUM_LEDS_SMALL_RING + index - 2) % NUM_LEDS_SMALL_RING] = 0;
        ++index;
        index %= NUM_LEDS_SMALL_RING;
    }

    dimRingLed(ring, NUM_LEDS_SMALL_RING, index, 1, speed);
    dimRingLed(ring, NUM_LEDS_SMALL_RING, index, 2, speed);

    if (ring[index] < MaxVal)
        ring[index] += speed;

    // Set ring
    for (auto k = 0; k < NUM_LEDS_SMALL_RING; ++k) {
#ifdef DBG_PRINT
        Serial.print(" ");
        Serial.print(ring[k]);
#endif
        if (ring[k] < 20) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            if (pots.brightness <= 1) {
                ledsSmallRing[k] = CRGB::Black;
            } else {
                ledsSmallRing[k] = MakeHsv(pots.hueSmallRing, ring[k]);
                ledsSmallRing[k].fadeLightBy(255 - pots.brightness);
            }
        }
    }

#ifdef DBG_PRINT
    Serial.println(" ");
#endif
}

// **************************************************************************************************************
void showSolid(const Pots& pots) {
    for (auto k = 0; k < NUM_LEDS_LARGE_RING; ++k) {
        ledsLargeRing[k] = MakeHsv(pots.hueLargeRing, 255);
        if (pots.brightness <= 1) {
            ledsLargeRing[k] = CRGB::Black;
        } else {
            ledsLargeRing[k].fadeLightBy(255 - pots.brightness);
        }
    }

    for (auto k = 0; k < NUM_LEDS_SMALL_RING; ++k) {
        ledsSmallRing[k] = MakeHsv(pots.hueSmallRing, 255);
        if (pots.brightness <= 1) {
            ledsSmallRing[k] = CRGB::Black;
        } else {
            ledsSmallRing[k].fadeLightBy(255 - pots.brightness);
        }
    }

    FastLED.show();
    delay(100);
}

// **************************************************************************************************************
void showRunningLights(byte speed, byte largeRingIndex, byte level, const Pots& pots) {
    // Large Ring
    const auto a = level;
    const auto b = level + (LEVEL_MAX / 3);
    const auto c = level + (LEVEL_MAX / 3) * 2;
    const auto sat = pots.hueLargeRing < 1 ? 0 : 255;
    const auto NUM_LEDS = 6;
    const byte levels[] = {LEVEL_MAX - c, LEVEL_MAX - b, LEVEL_MAX - a,
                           c, b, a};  // NUM_LEDS

    for (auto seg = 0; seg < SEGMENT_SIZE; ++seg) {
        const auto index = largeRingIndex + seg * NUM_LEDS_LARGE_SEGMENT;
        for (auto offset = 0; offset < NUM_LEDS; ++offset) {
            ledsLargeRing[(index + offset) % NUM_LEDS_LARGE_RING] =
                CHSV(pots.hueLargeRing, sat, min(levels[offset], pots.brightness));
        }
    }

    // Small ring
    const auto smallRingSpeed = max(1, speed / 5);
    setSmallRing(smallRingSpeed, pots);

    for (auto seg = 0; seg < SEGMENT_SIZE; ++seg) {
        ledsLargeRing[(largeRingIndex + seg * NUM_LEDS_LARGE_SEGMENT) % NUM_LEDS_LARGE_RING] =
            CRGB::Black;
    }

    FastLED.show();
    delay(10);
}

#ifdef READ_POTS
void readPots(Pots& pots) {
    pots.pulseSpeed = 1024 - analogRead(PIN_POT_PULSE_SPEED);
    pots.brightness = 254 - map(analogRead(PIN_POT_BRIGHTNESS), 0, 1024, 0, 255);
    pots.hueSmallRing = 252 - map(analogRead(PIN_POT_SMALL_COLOR), 0, 1024, 0, 255);
    pots.hueLargeRing = 252 - map(analogRead(PIN_POT_LARGE_COLOR), 0, 1024, 0, 255);
}
#endif

// **************************************************************************************************************
void loop() {
    auto speed = 1;
    bool finishLoop = false;
    Pots pots;
    for (auto largeRingIndex = 0; largeRingIndex < NUM_LEDS_LARGE_RING && !finishLoop; ++largeRingIndex) {
        for (auto level = 0; level < LEVEL_MAX / 3; level += speed) {
#ifdef READ_POTS
            readPots(pots);
#endif
            if (pots.pulseSpeed > PULSE_SPEED_MAX) {
                showSolid(pots);
                finishLoop = true;
                break;
            } else {
                speed = max(1, map(pots.pulseSpeed, 0, PULSE_SPEED_MAX, 0, 150));
                showRunningLights(speed, largeRingIndex, level, pots);
            }
        }
    }

    delay(1);
}
