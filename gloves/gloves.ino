#include <FastLED.h>
// #include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>
#include <Wire.h>

#define SUCCESS 0
#define ERROR -1
#define CAPTOUCH_PIN 2
#define LED_PIN 1
#define LED_COUNT 1
#define CHANGE_COLOR 0
#define MAX_STATIONARY 248
#define SPEED 2
#define FREQ 8
#define FPS 10
#define MEASURE_TOUCH 250
#define AVG 5

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

typedef struct  {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    bool toggledColor;
} firectx;

firectx ctx[LED_COUNT] = {0};
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

int setContext(
    firectx* ctx,
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
);
int sinWave(firectx* ctx);
int setPixelColor(firectx* ctx, uint8_t color);
void setBaseTouch();
bool checkPress();

void setup() {
    // Need to use TinyUSB stack for this to work.
    Serial.begin(115200);
    int8_t error = 0;
    if (!capButton.begin()) {
        Serial.println("Error with free touch init.");
        foreverError();
    }
    setBaseTouch();
    Serial.println("Captouch setup");
    FastLED.delay(2000);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, LED_COUNT);
    Serial.println("Setup LED strip");
    FastLED.clear(true);
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        Serial.print("Adding ");
        Serial.print(i);
        Serial.print("\n");
        error = setContext(
            &ctx[i],
            MAX_STATIONARY,
            uint8_t(random(HUE_RED, HUE_YELLOW - 16)),
            i
        );
        if (error < SUCCESS) {
            Serial.println("The fuck");
        }
    }

    FastLED.show();
}

void loop() {
    int8_t error = 0;
    uint8_t color = 0;
    bool update_color = false;
    if (checkPress()) {
        update_color = true;
    }
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        error = sinWave(&ctx[idx]);
        if (error < SUCCESS) foreverError();

        if (update_color) {
            color = uint8_t(random(HUE_RED, HUE_YELLOW - 16));
        } else {
            color = ctx[idx].color;
        }
        error = setPixelColor(&ctx[idx], color);
        if (error < SUCCESS) foreverError();
    }
    if(update_color) {
        update_color = false;
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
    Serial.print("Idx ");
    Serial.print(ctx->index);
    Serial.print(" brightness ");
    Serial.print(ctx->brightness);
    Serial.print(" phase ");
    Serial.print(ctx->phase);
    return SUCCESS;
}

int setPixelColor(firectx* ctx, uint8_t color) {
    ctx->color = color;
    Serial.print("Color ");
    Serial.print(ctx->color);
    Serial.print("\n");
    // TODO: I want to break this out at some point...
    leds[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);
    return SUCCESS;
}

bool checkPress() {
    bool touched = false;
    long measurement = capButton.measure();
    // Serial.println(measurement);
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
