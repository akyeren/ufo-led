#include <FastLED.h>

#define PULSE_SPEED_MAX 600
#define PULSE_SPEED_INIT 100  // 0 .. 600 Pulse Speed, > 600 no Pulse
#define BRIGHTNESS_INIT 1     // 0 .. 255
#define DBG_PRINT             // printing debug to serial message

// Animation settings
#define LED_BRIGHTNESS 255
#define LEVEL_MAX 255

// Ring sizes
#define NUM_LEDS_LARGE_RING 72
#define NUM_LEDS_SMALL_RING 7

// Pin configuration
#define PIN_LED_LARGE_RING 4  // D4
#define PIN_LED_SMALL_RING 6  // D6

// LED HW settings
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Globals
CRGB ledsLargeRing[NUM_LEDS_LARGE_RING];
CRGB ledsSmallRing[NUM_LEDS_SMALL_RING];

struct Pots {
    word pulseSpeed = PULSE_SPEED_INIT;
    byte brightness = BRIGHTNESS_INIT;
    byte hueSmallRing = HUE_GREEN;
    byte hueLargeRing = HUE_BLUE;
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

/**
 * Initialize Arduino
 */
void setup() {
    FastLED.addLeds<LED_TYPE, PIN_LED_SMALL_RING, COLOR_ORDER>(
        ledsSmallRing, NUM_LEDS_SMALL_RING);

    FastLED.addLeds<LED_TYPE, PIN_LED_LARGE_RING, COLOR_ORDER>(
        ledsLargeRing, NUM_LEDS_LARGE_RING);
    FastLED.setBrightness(BRIGHTNESS_INIT);

    doPost();
    delay(1000);  // power-up safety delay

    Serial.begin(9600);
}

/**
 * Dim an LED on a given ring
 * @param ring The ring of LEDs
 * @param size Number of LEDs on the ring
 * @param index the current index
 * @param offset the offset
 * @param dec decrement
 */
void dimRingLed(int ring[], byte size, byte index, byte offset, byte dec) {
    const auto target = (size + index - offset) % size;
    ring[target] -= min(ring[target], dec);
}

/**
 * Set the LEDs on the small ring
 * @param speed The speed of variation
 * @param pots display configuration
 */
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

/**
 * Show solid color on all the LEDs
 * @param pots display configuration
 */
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

/**
 * Show running lights, a group of lit LEDs separated by a fixed unlit ones
 * @param speed The speed of variation
 * @param largeRingIndex The index of the LED on the large ring
 * @param level the current level of the animation
 * @param pots display configuration
 */
void showRunningLights(byte speed, byte largeRingIndex, byte level, const Pots& pots) {
    const auto SegmentSize = 3;  // # of groups to divide the large ring into
    const auto NumLedsLargeSegment = NUM_LEDS_LARGE_RING / SegmentSize;

    // Large Ring
    const auto a = level;
    const auto b = level + (LEVEL_MAX / 3);
    const auto c = level + (LEVEL_MAX / 3) * 2;
    const auto sat = pots.hueLargeRing < 1 ? 0 : 255;
    const auto NUM_LEDS = 6;
    const byte levels[] = {LEVEL_MAX - c, LEVEL_MAX - b, LEVEL_MAX - a,
                           c, b, a};  // NUM_LEDS

    for (auto seg = 0; seg < SegmentSize; ++seg) {
        const auto index = largeRingIndex + seg * NumLedsLargeSegment;
        for (auto offset = 0; offset < NUM_LEDS; ++offset) {
            ledsLargeRing[(index + offset) % NUM_LEDS_LARGE_RING] =
                CHSV(pots.hueLargeRing, sat, min(levels[offset], pots.brightness));
        }
    }

    // Small ring
    const auto smallRingSpeed = max(1, speed / 5);
    setSmallRing(smallRingSpeed, pots);

    for (auto seg = 0; seg < SegmentSize; ++seg) {
        ledsLargeRing[(largeRingIndex + seg * NumLedsLargeSegment) % NUM_LEDS_LARGE_RING] =
            CRGB::Black;
    }

    FastLED.show();
    delay(10);
}

/**
 * Set the LEDs on the large ring to a color and delay for some milliseconds
 * @param color the color of the LEDs to be set
 * @param delayMs delay in milliseconds
 */
void setLargeLeds(CRGB color, int delayMs) {
    for (auto k = 0; k < NUM_LEDS_LARGE_RING; ++k) {
        ledsLargeRing[k] = color;
    }
    FastLED.show();
    delay(delayMs);
}

/**
 * Scroll the color wheel
 * @param pos position of the wheel [0,255]
 */
CRGB colorScroll(int pos) {
    CRGB color(0, 0, 0);
    if (pos < 85) {
        color.g = 0;
        color.r = ((float)pos / 85.0f) * 255.0f;
        color.b = 255 - color.r;
    } else if (pos < 170) {
        color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
        color.r = 255 - color.g;
        color.b = 0;
    } else if (pos < 256) {
        color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
        color.g = 255 - color.b;
        color.r = 1;
    }
    return color;
}

/**
 * Perform an analog rainbow effect, each individual LED smoothly transitions
 */
void doRainbow() {
    const int LEVELS = 256;
    for (int j = 0; j < LEVELS; j++) {
        for (int i = 0; i < NUM_LEDS_LARGE_RING; i++) {
            ledsLargeRing[i] = colorScroll((i * LEVELS / NUM_LEDS_LARGE_RING + j) % 256);
        }

        FastLED.show();
        delay(4);
    }
}

/**
 * Perform a simple rainbow effect, a fixed pattern of rainbow colors moves in one direction
 */
void doRainbow2() {
    static const CRGB colorWheel[] = {
        CRGB::Red,
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Green,
        CRGB::Aqua,
        CRGB::Blue,
        CRGB::Purple,
        CRGB::Pink};
    const auto Colors = 8;
    const int LEVELS = 256;
    for (int k = 0; k < LEVELS; ++k) {
        for (int i = 0; i < NUM_LEDS_LARGE_RING; ++i) {
            ledsLargeRing[i] = colorWheel[(i + k) % Colors];
        }
        FastLED.show();
        delay(200);
    }
}

/**
 * Perform a tail effect, a series of brightness gradient LEDs moves along the
 * direction of the head (brightest LED).
 * The speed increases to max speed, then slows down to min speed and so on so forth.
 */
void doTail() {
    FastLED.setBrightness(10);
    const auto color = CRGB::LightBlue;
    const auto tailLength = 8;
    const auto maxBrightness = 255;
    const auto BRIT = maxBrightness / tailLength;
    const auto LEVELS = NUM_LEDS_LARGE_RING;

    const auto minSpeed = 0.1f;  // step per millisecond
    const auto maxSpeed = 4.0f;
    auto speed = minSpeed;
    auto acceleration = 0.1f;
    const auto minDelay = 1.0f;
    auto step = 1;
    auto rep = 0;

    for (auto delayMs = static_cast<int>(static_cast<float>(step) / speed);;) {
        for (auto k = 0; k < LEVELS; k += step) {
            for (auto i = 0; i < NUM_LEDS_LARGE_RING; ++i) {
                auto j = i + k;
                if (j > NUM_LEDS_LARGE_RING)
                    j -= NUM_LEDS_LARGE_RING;
                if (j <= tailLength) {
                    ledsLargeRing[i] = color;
                    ledsLargeRing[i].fadeLightBy(BRIT * j);
                } else {
                    ledsLargeRing[i] = CRGB::Black;
                }
            }
            FastLED.show();
            delay(delayMs);
        }

        // when it's fast enough and for a while
        if (speed >= maxSpeed) {
            // reverse
            speed = maxSpeed;
            if (++rep > 2) {
                if (acceleration > 0)
                    acceleration = -acceleration;
                rep = 0;
            } else {
                continue;
            }
        } else if (speed <= minSpeed) {
            // reverse
            speed = minSpeed;
            if (++rep > 1) {
                if (acceleration < 0)
                    acceleration = -acceleration;
                rep = 0;
            } else {
                continue;
            }
        }
        speed += acceleration;
        while (acceleration < 0 && static_cast<float>(step) / speed > minDelay && step > 2)  // step could be smaller
            --step;

        while (static_cast<float>(step) / speed < minDelay)  // step too small
            ++step;

        delayMs = static_cast<int>(static_cast<float>(step) / speed);
    }
}

/**
 * Show self test pattern
 */
void doPost() {
    doTail();
    // All LEDs for 1.5 seconds
    // setLargeLeds(CRGB::White, 1000);
    // setLargeLeds(CRGB::Black, 1000);

    // // flash all LEDs
    // for (auto i = 0; i < 6; ++i) {
    //     setLargeLeds(CRGB::Red, 250);
    //     setLargeLeds(CRGB::Yellow, 250);
    //     setLargeLeds(CRGB::Green, 250);
    //     setLargeLeds(CRGB::Blue, 250);
    // }

    // // Rainbow
    // for (auto i = 0; i < 20; ++i) {
    //     doRainbow2();
    // }

    // setLargeLeds(CRGB::Black, 400);
    // FastLED.setBrightness(LED_BRIGHTNESS);
}

/**
 * Main loop
 */
void loop() {
    auto speed = 1;
    bool finishLoop = false;
    Pots pots;
    for (auto largeRingIndex = 0; largeRingIndex < NUM_LEDS_LARGE_RING && !finishLoop; ++largeRingIndex) {
        for (auto level = 0; level < LEVEL_MAX / 3; level += speed) {
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
