#include <FastLED.h>

#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5
#define LED_PIN 8
#define LED_COUNT 10
#define MAX_MOVING 248
#define MAX_STATIONARY 184
#define CHANGE_COLOR 36
#define RANGE 8
#define FPS 10

typedef struct firectx {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    bool accending;
} firectx;
firectx ctx[LED_COUNT] = {0};

CRGB strip[LED_COUNT];

void setup() {
    randomSeed(analogRead(9)); // Just make sure that this is unconnected.
    FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, LED_COUNT);
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        ctx[idx].index = idx;
        ctx[idx].brightness = MAX_STATIONARY/2;
        ctx[idx].accending = false;
        setLed(&ctx[idx]);
    }
    FastLED.show();
}

void loop() {
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        setLed(&ctx[idx]);
    }
    FastLED.show();
    FastLED.delay(1000/FPS);
}

void setLed(firectx* ctx) {
    // Start by setting the brightness.
    if (ctx->accending) {
        ctx->brightness = random(ctx->brightness, ctx->brightness + RANGE);
    } else {
        ctx->brightness = random(ctx->brightness - RANGE, ctx->brightness + 1);
    }
    strip[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);

    // If the brightness is small enough, choose a different color.
    if (ctx->brightness < CHANGE_COLOR) {
        setPixelColors(ctx);
        ctx->accending = ~ctx->accending;
        ctx->brightness = CHANGE_COLOR + 1;
    } else if (ctx->brightness > MAX_STATIONARY) {
        ctx->accending = ~ctx->accending;
    } else {
        uint8_t val = random(10);
        if (val == 0) {
            ctx->accending = ~ctx->accending;
        }
    }
}

void setPixelColors(firectx* ctx) {
    #define RED_RANGE 80
    #define ORANGE_RANGE 100
    #define TOTAL_RANGE ORANGE_RANGE
    uint32_t color = 0;
    uint8_t value = random(TOTAL_RANGE);
    if (value < RED_RANGE) {
        ctx->color = random(HUE_RED, HUE_ORANGE);
    } else if (value < ORANGE_RANGE) {
        ctx->color = random(HUE_ORANGE, HUE_YELLOW - 16);
    }
    strip[ctx->index] = CHSV(ctx->color, random(100,256), ctx->brightness);
}
