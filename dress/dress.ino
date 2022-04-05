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
#define ACCEL_READINGS 5
#define GRAVITY 9.8
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

typedef struct  {
    float accel_readings[ACCEL_READINGS];
    uint8_t index;
} moving_avg;

firectx ctx[LED_COUNT] = {0};
CRGB strip[LED_COUNT];
Adafruit_LIS3DH accel = Adafruit_LIS3DH();
float max_sensor_value = 0.0;
moving_avg avg = {0};

void setContext(
    firectx* ctx,
    uint8_t color,
    uint8_t index
);
void sinWave(firectx* ctx, Direction direction, uint8_t max_brightness);
void setPixelColor(firectx* ctx, uint8_t color);
float calculateMagnitude();

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
    if (ACCEL_RANGE == LIS3DH_RANGE_2_G) {
       max_sensor_value = GRAVITY * 2;
    } else if (ACCEL_RANGE == LIS3DH_RANGE_4_G) {
       max_sensor_value = GRAVITY * 4;
    } else if (ACCEL_RANGE == LIS3DH_RANGE_8_G) {
       max_sensor_value = GRAVITY * 8;
    } else if (ACCEL_RANGE == LIS3DH_RANGE_16_G) {
       max_sensor_value = GRAVITY * 16;
    } else {
       max_sensor_value = GRAVITY;
    }
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

    float mag = calculateMagnitude();
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        if (mag < 85.0) {
            ctx[idx].speed = 12;
            max_brightness = 255;
        } else if (mag < 125) {
            ctx[idx].speed = 4;
            max_brightness = 212;
        } else {
            ctx[idx].speed = 2;
            max_brightness = 164;
        }
        sinWave(&ctx[idx], UP, max_brightness);
        if (ctx[idx].brightness <= CHANGE_COLOR) {
            ctx[idx].color = random8(HUE_RED, HUE_ORANGE - 8);
            ctx[idx].phase = random8(2, 254);
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

float calculateMagnitude() {
    sensors_event_t event = {0};
    accel.getEvent(&event);
    float cur_avg = 0;
    cur_avg = 180 *
        atan(event.acceleration.y /
        sqrt(
            event.acceleration.x*event.acceleration.x
            + event.acceleration.z*event.acceleration.z
        )
    );
    avg.accel_readings[avg.index] = cur_avg;
    avg.index++;
    if (avg.index >= ACCEL_READINGS) {
        avg.index = 0;
    }
    cur_avg = 0;
    for (uint8_t i = 0; i < ACCEL_READINGS; i++) {
        cur_avg += avg.accel_readings[i];
    }
    cur_avg /= ACCEL_READINGS;

    Serial.print("X:");
    Serial.print(event.acceleration.x / max_sensor_value);
    Serial.print("\n");
    Serial.print("Y:");
    Serial.print(event.acceleration.y / max_sensor_value);
    Serial.print("\n");
    Serial.print("Z:");
    Serial.print(event.acceleration.z / max_sensor_value);
    Serial.print("\n");
    Serial.print("avg:");
    Serial.print(cur_avg);
    Serial.print("\n");
    return cur_avg;
}

