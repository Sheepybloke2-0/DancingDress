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
#define ACCEL_RANGE LIS3DH_RANGE_4_G
#define ACCEL_READINGS 5
#define GRAVITY 9.8
#define CHANGE_COLOR 8
#define MAX_STATIONARY 248
#define MAX_SPEED 24
#define MAX_FREQ 24
#define FPS 10

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

typedef struct  {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    uint8_t freq;
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
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
);
void sinWave(firectx* ctx);
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
            MAX_STATIONARY/2,
            random(HUE_RED, HUE_ORANGE),
            i
        );
    }

    FastLED.show();
}

void loop() {
    int8_t error = 0;
    uint8_t color = 0;
    uint8_t speed = 0;
    uint8_t freq = 0;

    float mag = calculateMagnitude();
    for (uint8_t idx = 0; idx < LED_COUNT; idx++) {
        sinWave(&ctx[idx]);

        if ((ctx[idx].brightness <= CHANGE_COLOR || mag >= 0.20 )
          && ctx[idx].toggledColor == false) {
            color = uint8_t(random(HUE_RED, HUE_ORANGE));
            if (mag < 0.10) {
                speed = uint8_t(MAX_SPEED/3);
                freq = uint8_t(MAX_FREQ/3);
            } else if (mag < 0.20) {
                speed = uint8_t(MAX_SPEED * (2/3));
                freq = uint8_t(MAX_FREQ * (2/3));
            } else {
                speed = MAX_SPEED;
                freq = MAX_FREQ;
            }
            ctx[idx].speed = uint8_t(random(3,speed));
            ctx[idx].freq = uint8_t(random(3,freq));
            ctx[idx].toggledColor = true;
        } else {
            color = ctx[idx].color;
            ctx[idx].toggledColor = false;
        }
        setPixelColor(&ctx[idx], color);
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
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
) {
    ctx->index = index;
    ctx->brightness = default_brightness;
    ctx->phase = uint8_t(random(24));
    ctx->speed = uint8_t(random(1,24));
    ctx->freq = uint8_t(random(1,24));
    ctx->toggledColor = false;
    setPixelColor(ctx, color);
}

void sinWave(firectx* ctx) {
    ctx->phase += ctx->speed;
    ctx->brightness = quadwave8(ctx->freq*ctx->phase);
    if (ctx->brightness > MAX_STATIONARY) {
        ctx->brightness = MAX_STATIONARY;
    } else if (ctx->brightness < CHANGE_COLOR) {
        ctx->brightness = CHANGE_COLOR;
    }
    Serial.println(ctx->brightness);
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
    cur_avg += abs(event.acceleration.x / max_sensor_value);
    // avg += abs(event.acceleration.y / max_sensor_value);
    cur_avg += abs(event.acceleration.z / max_sensor_value);
    cur_avg /= 2;
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

    // Serial.print("X:");
    // Serial.print(event.acceleration.x / max_sensor_value);
    // Serial.print("\n");
    // Serial.print("Y:");
    // Serial.print(event.acceleration.y / max_sensor_value);
    // Serial.print("\n");
    // Serial.print("Z:");
    // Serial.print(event.acceleration.z / max_sensor_value);
    // Serial.print("\n");
    // Serial.print("avg:");
    // Serial.print(cur_avg);
    // Serial.print("\n");
    return cur_avg;
}

