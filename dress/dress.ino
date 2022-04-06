#define ARDUINO_GEMMA_M0 1
#include <FastLED.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define LED_PIN 1
#define LED_COUNT 20
#define ACCEL_ADDR 0x18
#define ACCEL_SCL 2
#define ACCEL_SDA 0
#define ACCEL_RANGE LIS3DH_RANGE_2_G
#define SMALL_MOVEMENT_THRESHOLD 3.0
#define LARGE_MOVEMENT_THRESHOLD 6.0
#define CHANGE_COLOR 8
#define FPS 10

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

typedef enum {
    UP = 0,
    DOWN = 1,
} Direction;

typedef struct  {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    uint8_t offset;
    uint8_t speed;
    bool toggledColor;
} firectx;

firectx ctx[LED_COUNT] = {0};
CRGB strip[LED_COUNT];
Adafruit_LIS3DH accel = Adafruit_LIS3DH();
double old_mag = 0.0;

void setContext(
    firectx* ctx,
    uint8_t color,
    uint8_t index
);
void sinWave(firectx* ctx, Direction direction, uint8_t max_brightness);
void setPixelColor(firectx* ctx, uint8_t color);
double calculateMagnitude();

void setup() {
    // Need to use TinyUSB stack for this to work.
    Serial.begin(115200);
    Serial.println("Starting gemma...");
    if (!accel.begin(ACCEL_ADDR)) {
        while(true){
            Serial.println("Couldn't find the ACCELEROMETER!");
            FastLED.delay(500);
        }
    }
    accel.setRange(ACCEL_RANGE);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(strip, LED_COUNT);
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        strip[i] = CRGB::Red;
        setContext(
            &ctx[i],
            random(HUE_RED, HUE_ORANGE - 8),
            i
        );
    }

    FastLED.show();
}

void loop() {
    int8_t error = 0;
    uint8_t color = 0;
    uint8_t speed = 0;
    uint8_t max_brightness = 0;

    double mag = calculateMagnitude();
    double mag_diff = abs(mag - old_mag);
    if (mag_diff >  LARGE_MOVEMENT_THRESHOLD) {
        speed = 12;
    } else if (mag_diff > SMALL_MOVEMENT_THRESHOLD) {
        speed = 8;
    } else {
        speed = 4;
    }
    old_mag = mag;
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        ctx[idx].speed = speed;
        sinWave(&ctx[idx], UP, max_brightness);
        if (ctx[idx].brightness <= CHANGE_COLOR) {
            ctx[idx].color = random8(HUE_RED, HUE_ORANGE - 16);
            ctx[idx].phase = random8(2, 254);
            ctx[idx].offset = random8(1, 32);

            if (mag_diff >  LARGE_MOVEMENT_THRESHOLD) {
                max_brightness = 255;
            } else if (mag_diff > SMALL_MOVEMENT_THRESHOLD) {
                max_brightness = 192;
            } else {
                max_brightness = 128;
            }
        }
        setPixelColor(&ctx[idx], ctx[idx].color);
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

void setContext(
    firectx* ctx,
    uint8_t color,
    uint8_t index
) {
    ctx->index = index;
    ctx->brightness = 0;
    ctx->phase = random8((index + 1) * 8);
    ctx->offset = random8((index + 1) * 8);
    ctx->speed = random8(2,7);
    ctx->toggledColor = false;
    setPixelColor(ctx, color);
}

void sinWave(firectx* ctx, Direction dir, uint8_t max_brightness) {
    if (dir == UP) {
        ctx->phase += ctx->speed;
    } else {
        ctx->phase -= ctx->speed;
    }
    ctx->brightness = quadwave8(ctx->phase + ctx->offset);
    if (ctx->brightness > max_brightness) {
        ctx->brightness = max_brightness;
    } else if (ctx->brightness <= CHANGE_COLOR) {
        ctx->brightness = 0;
    }
}

void setPixelColor(firectx* ctx, uint8_t color) {
    ctx->color = color;
    // TODO: I want to break this out at some point...
    strip[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);
}

double calculateMagnitude() {
    sensors_event_t event = {0};
    accel.getEvent(&event);
    double mag = event.acceleration.y * event.acceleration.y;
    mag += event.acceleration.x * event.acceleration.x;
    mag += event.acceleration.z * event.acceleration.z;
    mag = sqrt(mag);

    Serial.print("new mag:");
    Serial.print(mag);
    Serial.print("\n");
    return mag;
}

