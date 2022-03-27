#include <FastLED.h>
#include <lib8tion.h>
#include <Adafruit_FreeTouch.h>
#include <Wire.h>

#define SUCCESS 0
#define ERROR -1
#define CAPTOUCH_PIN 2
#define LED_PIN 1
#define LED_COUNT 5
#define CHANGE_COLOR 8
#define MAX_STATIONARY 248
#define FREQ 8
#define FPS 30
#define MEASURE_TOUCH 250
#define AVG 5

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

typedef enum {
    UP = 0,
    DOWN = 1,
} Direction;

typedef enum {
    NONE = 0,
    SWIRLUP = 1,
    SWIRLDOWN = 2,
    LIGHTNING = 3,
    RANDOMBREATHING = 4,
} light_style;

typedef struct  {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    uint8_t offset;
    uint8_t speed;
    light_style set_style;
} firectx;

typedef struct  {
    bool delaying;
    bool flashing;
    bool finished[LED_COUNT];
    uint8_t finished_cnt;
    uint8_t selected_led;
    uint8_t delay;
    uint8_t delay_cnt;
} lightning_ctx;


firectx ctx[LED_COUNT] = {0};
lightning_ctx l_ctx = {0};
CRGB leds[LED_COUNT];
Adafruit_FreeTouch capButton =
    Adafruit_FreeTouch(
        CAPTOUCH_PIN,
        OVERSAMPLE_4,
        RESISTOR_50K,
        FREQ_MODE_NONE
    );
long base_touch_measurement = 0.0;
long previous_measurement = 0.0;
light_style current_style = NONE;
uint8_t lightning_lut[LED_COUNT][2][2] = {
    {{1,4},{2,3}},
    {{0,2},{3,4}},
    {{1,3},{4,0}},
    {{4,2},{1,0}},
    {{3,0},{1,2}}
};

void lightning(firectx* ctx, uint8_t led);
void swirlUp(firectx* ctx);
void swirlDown(firectx* ctx);
void randomBreathing(firectx* ctx);
void sinWave(firectx* ctx, Direction dir);
void setPixelColor(firectx* ctx, uint8_t color);
void setBaseTouch();
bool checkPress();

void setup() {
    // Need to use TinyUSB stack for this to work.
    Serial.begin(115200);
    int8_t error = 0;
    if (!capButton.begin()) {
        Serial.println("Error with free touch init.");
    }
    setBaseTouch();
    Serial.println("Captouch setup");
    FastLED.delay(2000);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
    Serial.println("Setup LED strip");
    FastLED.clear(true);
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        ctx[idx].index = idx;
        swirlUp(&ctx[idx]);
    }
    current_style = SWIRLUP;

    FastLED.show();
}

void loop() {
    int8_t error = 0;
    uint8_t color = 0;
    bool change_lights = false;
    if (checkPress()) {
        change_lights = true;
    }
    if (change_lights) {
        current_style = light_style(current_style + 1);
        if (current_style > RANDOMBREATHING) {
            current_style = NONE;
        }
        if (current_style)
        change_lights = false;
    } else {
        switch (current_style)
        {
        case SWIRLUP:
            for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
                swirlUp(&ctx[idx]);
            }
            break;

        case SWIRLDOWN:
            for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
                swirlDown(&ctx[idx]);
            }
            break;

        case LIGHTNING:
            if(!l_ctx.flashing) {
                if (l_ctx.delaying) {
                    l_ctx.delay_cnt++;
                    if (l_ctx.delay <= l_ctx.delay_cnt) {
                        l_ctx.delaying = false;
                    }
                } else {
                    l_ctx.selected_led = random8(LED_COUNT);
                    l_ctx.flashing = true;
                    l_ctx.delay = 0;
                    l_ctx.delaying = false;
                    l_ctx.finished_cnt = 0;
                    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
                        l_ctx.finished[idx] = false;
                        ctx[idx].set_style = NONE;
                    }
                }
            } else {
                for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
                    if (!l_ctx.finished[idx]) {
                        lightning(&ctx[idx], l_ctx.selected_led);
                        if (ctx[idx].brightness <= CHANGE_COLOR) {
                            l_ctx.finished[idx] = true;
                            l_ctx.finished_cnt++;
                        }
                    }
                }
                if (l_ctx.finished_cnt >= LED_COUNT) {
                    l_ctx.flashing = false;
                    l_ctx.delay = random8(1000/FPS);
                    l_ctx.delay_cnt = 0;
                    l_ctx.delaying = true;
                }
            }
            break;

        case RANDOMBREATHING:
            for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
                randomBreathing(&ctx[idx]);
            }
            break;

        default:
            FastLED.clear();
            break;
        }
    }

    FastLED.show();
    FastLED.delay(1000/FPS);
}

void foreverError() {
    while (1){
        // Serial.println("Error with free touch init.");
        yield();
    }
}


void sinWave(firectx* ctx, Direction dir) {
    if (dir == UP) {
        ctx->phase += ctx->speed;
    } else {
        ctx->phase -= ctx->speed;
    }
    ctx->brightness = quadwave8(ctx->phase + ctx->offset);
    if (ctx->brightness > MAX_STATIONARY) {
        ctx->brightness = MAX_STATIONARY;
    } else if (ctx->brightness <= CHANGE_COLOR) {
        ctx->brightness = 0;
    }
}

void swirlUp(firectx* ctx) {
    if (ctx->set_style != SWIRLUP) {
        ctx->offset = ctx->index * 32;
        ctx->brightness = 0;
        ctx->phase = 0;
        ctx->speed = 4;
        ctx->color = random8(HUE_PURPLE - 8, HUE_PURPLE + 8);
        ctx->set_style = SWIRLUP;
    }
    sinWave(ctx, UP);
    if (ctx->brightness <= CHANGE_COLOR) {
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);
    }
    setPixelColor(ctx, ctx->color);
}

void swirlDown(firectx* ctx) {
    if (ctx->set_style != SWIRLDOWN) {
        ctx->offset = ctx->index * 32;
        ctx->brightness = 0;
        ctx->phase = 0;
        ctx->speed = 4;
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);
        ctx->set_style = SWIRLDOWN;
    }
    sinWave(ctx, DOWN);
    if (ctx->brightness <= CHANGE_COLOR) {
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);
    }
    setPixelColor(ctx, ctx->color);
}

void lightning(firectx* ctx, uint8_t led) {
    if (ctx->set_style != LIGHTNING) {
        ctx->offset = 0;
        ctx->phase = 0;
        ctx->set_style = LIGHTNING;
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);

        if (led == ctx->index) {
            ctx->brightness = MAX_STATIONARY;
            ctx->speed = 3;
        } else if ((ctx->index == lightning_lut[led][0][0])
         || (ctx->index == lightning_lut[led][0][1]))
        {
            ctx->brightness = 186;
            ctx->speed = 4;
        } else if ((ctx->index == lightning_lut[led][1][0])
         || (ctx->index == lightning_lut[led][1][1]))
        {
            ctx->brightness = 124;
            ctx->speed = 4;
        }
    } else {
        ctx->brightness -= ctx->speed;
        if (ctx->brightness <= CHANGE_COLOR) {
            ctx->brightness = 0;
        }
        setPixelColor(ctx, ctx->color);
    }
}

void randomBreathing(firectx* ctx) {
    if (ctx->set_style != RANDOMBREATHING) {
        ctx->offset = random8((ctx->index + 1) * 8);
        ctx->brightness = 0;
        ctx->phase = random8((ctx->index + 1) * 8);
        ctx->speed = random8(2, 7);
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);
        ctx->set_style = RANDOMBREATHING;
    }
    sinWave(ctx, UP);
    if (ctx->brightness <= CHANGE_COLOR) {
        ctx->color = random8(HUE_PURPLE - 16, HUE_PURPLE + 8);
        ctx->offset = random8((ctx->index + 1) * 8);
        ctx->phase = random8((ctx->index + 1) * 8);
        ctx->speed = random8(2, 7);
    }
    setPixelColor(ctx, ctx->color);
}

void setPixelColor(firectx* ctx, uint8_t color) {
    ctx->color = color;
    // TODO: I want to break this out at some point...
    leds[ctx->index] = CHSV(ctx->color, 32, ctx->brightness);
}

bool checkPress() {
    bool touched = false;
    long measurement = capButton.measure();
    // Only check if we've seen a change in the measurement
    if (measurement > (base_touch_measurement + MEASURE_TOUCH)
     && previous_measurement < (base_touch_measurement + MEASURE_TOUCH)
    ) {
        FastLED.delay(250);
        measurement = capButton.measure();
        if (measurement > (base_touch_measurement + MEASURE_TOUCH)) {
            touched = true;
        }
    }
    return touched;
}

void setBaseTouch() {
    for (uint8_t i = 0; i < AVG; i++) {
        base_touch_measurement += capButton.measure();
    }
    base_touch_measurement /= AVG;
}
