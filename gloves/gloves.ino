#include <FastLED.h>
#include <Wire.h>


#define SUCCESS 0
#define ERROR -1
// TODO: Update this for neopixel
#define LED_PIN 3
#define CLK_PIN 4
#define LED_COUNT 1
#define CHANGE_COLOR 0
#define MAX_STATIONARY 248
#define SPEED 2
#define FREQ 8
#define FPS 10

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

typedef struct  {
    CRGB* strip;
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    bool toggledColor;
} firectx;


firectx ctx[LED_COUNT] = {0};
CRGB strip[LED_COUNT];

int setContext(
    firectx* ctx,
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
);
int sinWave(firectx* ctx);
int setPixelColor(firectx* ctx, uint8_t color);

void setup() {
    // Need to use TinyUSB stack for this to work.
    Serial.begin(115200);
    int8_t error = 0;
    // FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, LED_COUNT);
    FastLED.addLeds<DOTSTAR, LED_PIN, CLK_PIN, GBR>(strip, LED_COUNT);
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        ctx->strip = strip;
        error = setContext(
            &ctx[i],
            MAX_STATIONARY/2,
            random(HUE_RED, HUE_YELLOW - 16),
            i
        );
        if (error < SUCCESS) {
            foreverError();
        }
    }

    FastLED.show();
}

void loop() {
    int8_t error = 0;
    uint8_t color = 0;
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        error = sinWave(&ctx[idx]);
        if (error < SUCCESS) foreverError();

        if (ctx[idx].brightness < CHANGE_COLOR && !ctx->toggledColor) {
            color = uint8_t(random(HUE_RED, HUE_YELLOW - 16));
            ctx->toggledColor = true;
        } else {
            color = ctx[idx].color;
            ctx->toggledColor = false;
        }
        error = setPixelColor(&ctx[idx], color);
        if (error < SUCCESS) foreverError();
    }

    FastLED.show();
    FastLED.delay(1000/FPS);
}

void foreverError() {
    while (1) yield();
}

int setContext(
    firectx* ctx,
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
) {
    ctx->index = index;
    ctx->brightness = default_brightness;
    ctx->phase = uint8_t(random(8));
    ctx->toggledColor = false;
    setPixelColor(ctx, color);
    return SUCCESS;
}

int sinWave(firectx* ctx) {
    ctx->phase += SPEED;
    ctx->brightness = quadwave8(ctx->phase);
    if (ctx->brightness > MAX_STATIONARY) {
        ctx->brightness = MAX_STATIONARY;
    } else if (ctx->brightness < CHANGE_COLOR) {
        ctx->brightness = 0;
    }
    Serial.println(ctx->brightness);
    return SUCCESS;
}

int setPixelColor(firectx* ctx, uint8_t color) {
    ctx->color = color;
    ctx->strip[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);
    return SUCCESS;
}
