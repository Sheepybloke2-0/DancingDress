#include <FastLED.h>

#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5
#define LED_PIN 8
#define LED_COUNT 10
#define MAX_MOVING 248
#define MAX_STATIONARY 184
#define CHANGE_COLOR 16
#define RANGE 8
#define FPS 30

typedef struct color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

typedef struct firectx {
    uint8_t index;
    uint8_t brightness;
    color_t color;
    bool accending;
} firectx;
firectx ctx[LED_COUNT] = {0};

CRGB strip[LED_COUNT];
color_t orange = {255, 254*.1, 0};
color_t redish = {255, 254*.2, 0};
color_t red = {255, 0, 0};

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
        ctx->brightness = random(ctx->brightness - RANGE, ctx->brightness);
    }
    strip[ctx->index].fadeLightBy(ctx->brightness);

    // If the brightness is small enough, choose a different color.
    if (ctx->brightness < CHANGE_COLOR) {
        setPixelColors(ctx);
        ctx->accending = ~ctx->accending;
        ctx->brightness = CHANGE_COLOR + 1;
    } else if (ctx->brightness > MAX_STATIONARY) {
        ctx->accending = ~ctx->accending;
    }

}

void setPixelColors(firectx* ctx) {
    #define RED_RANGE 40
    #define REDISH_RANGE 60
    #define ORANGE_RANGE 100
    #define TOTAL_RANGE ORANGE_RANGE
    uint32_t color = 0;
    uint8_t value = random(TOTAL_RANGE);
    if (value < RED_RANGE) {
        ctx->color = red;
    } else if (value < REDISH_RANGE) {
        ctx->color = redish;
    } else {
        ctx->color = orange;
    }
    strip[ctx->index].r = ctx->color.r;
    strip[ctx->index].g = ctx->color.g;
    strip[ctx->index].b = ctx->color.b;
}
