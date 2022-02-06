#include <FastLED.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <Wire.h>

#define ACCEL_INTERRUPT_PIN 27
#define ACCEL_ADDR 0x19
#define LEFT_BUTTON 4
#define RIGHT_BUTTON 5
#define LED_PIN 8
#define LED_COUNT 10
#define MAX_STATIONARY 248
#define CHANGE_COLOR 36
#define RANGE 8
#define FPS 10
#define SENSOR_SENSITIVITY LIS3DH_RANGE_4_G
#define GRAVITY 9.8

typedef struct firectx {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    bool accending;
} firectx;
firectx ctx[LED_COUNT] = {0};

CRGB strip[LED_COUNT];
Adafruit_LIS3DH lis = Adafruit_LIS3DH(&Wire1);
float max_sensor_value = 0;
float avg = 0.0;

void setup() {
    Serial1.begin(115200);
    randomSeed(analogRead(9)); // Just make sure that this is unconnected.
    if (! lis.begin(ACCEL_ADDR)) {   // Express has 0x19
        while (1) yield();
    }
    lis.setRange(SENSOR_SENSITIVITY);
    if (SENSOR_SENSITIVITY == LIS3DH_RANGE_2_G) {
       max_sensor_value = GRAVITY * 2;
    } else if (SENSOR_SENSITIVITY == LIS3DH_RANGE_4_G) {
       max_sensor_value = GRAVITY * 4;
    } else if (SENSOR_SENSITIVITY == LIS3DH_RANGE_8_G) {
       max_sensor_value = GRAVITY * 8;
    } else if (SENSOR_SENSITIVITY == LIS3DH_RANGE_16_G) {
       max_sensor_value = GRAVITY * 16;
    }
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
    avg = calculateMagnitude();
    // max_brightness = uint8_t(avg * (MAX_MOVING - MAX_STATIONARY)) + MAX_STATIONARY);
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        setLed(&ctx[idx]);
    }
    FastLED.show();
    FastLED.delay(1000/FPS);
}

float calculateMagnitude() {
    sensors_event_t event = {0};
    float pitch = 0;
    float roll = 0;
    lis.getEvent(&event);
    pitch = atan(
        event.acceleration.y / (
            sqrtf(
                pow(event.acceleration.x, 2.0) + pow(event.acceleration.z, 2.0)
                )
            )
        );
    roll = atan(
            -1* event.acceleration.x / event.acceleration.z
        );
    Serial1.print("X:");
    Serial1.print(event.acceleration.x / max_sensor_value);
    Serial1.print("\n");
    Serial1.print("Y:");
    Serial1.print(event.acceleration.y / max_sensor_value);
    Serial1.print("\n");
    Serial1.print("Z:");
    Serial1.print(event.acceleration.z / max_sensor_value);
    Serial1.print("\n");
    Serial1.print("Roll:");
    Serial1.print(roll);
    Serial1.print("\n");
    Serial1.print("Pitch:");
    Serial1.print(pitch);
    Serial1.print("\n");
    return pitch;
}

void setLed(firectx* ctx) {
    // Start by setting the brightness.
    if (ctx->accending) {
        ctx->brightness = random(ctx->brightness, ctx->brightness + RANGE);
    } else {
        ctx->brightness = random(ctx->brightness - RANGE, ctx->brightness + 1);
    }

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
    strip[ctx->index] = CHSV(ctx->color, 255, uint8_t(ctx->brightness*avg));
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
    // strip[ctx->index] = CHSV(ctx->color, random(100,256), ctx->brightness);
    strip[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);
}
