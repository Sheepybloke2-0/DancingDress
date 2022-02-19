# 1 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino"

# 3 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino" 2
# 4 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino" 2
# 5 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino" 2
# 6 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino" 2
# 7 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino" 2
// #include <Adafruit_NeoPixel.h>
# 26 "c:\\Users\\rtl55\\OneDrive\\PersonalProjs\\danceStuff\\DancingDress\\dress\\dress.ino"
  // Required for Serial on Zero based boards



typedef struct {
    uint8_t index;
    uint8_t brightness;
    uint8_t color;
    uint8_t phase;
    uint8_t freq;
    uint8_t speed;
    bool toggledColor;
} firectx;

typedef struct {
    float accel_readings[5];
    uint8_t index;
} moving_avg;

firectx ctx[1] = {0};
CRGB strip[1];
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB+NEO_KHZ800);
Adafruit_LIS3DH accel = Adafruit_LIS3DH();
float max_sensor_value = 0.0;
moving_avg avg = {0};

int setContext(
    firectx* ctx,
    uint8_t default_brightness,
    uint8_t color,
    uint8_t index
);
int sinWave(firectx* ctx);
int setPixelColor(firectx* ctx, uint8_t color);
float calculateMagnitude();

void setup() {
    // Need to use TinyUSB stack for this to work.
    // Serial.begin(115200);
    // Serial.println("Starting flora...");
    // int8_t error = 0;
    // strip.begin();
    // strip.setBrightness(255);
    // strip.fill(strip.Color(255,255,0), 0, LED_COUNT);
    // strip.show();
    // if (!accel.begin(ACCEL_ADDR)) {
    //     while(true){
    //         Serial.println("Couldn't find the ACCELEROMETER!");
    //         FastLED.delay(500);
    //     }
    // }
    // accel.setRange(ACCEL_RANGE);
    // if (ACCEL_RANGE == LIS3DH_RANGE_2_G) {
    //    max_sensor_value = GRAVITY * 2;
    // } else if (ACCEL_RANGE == LIS3DH_RANGE_4_G) {
    //    max_sensor_value = GRAVITY * 4;
    // } else if (ACCEL_RANGE == LIS3DH_RANGE_8_G) {
    //    max_sensor_value = GRAVITY * 8;
    // } else if (ACCEL_RANGE == LIS3DH_RANGE_16_G) {
    //    max_sensor_value = GRAVITY * 16;
    // } else {
    //    max_sensor_value = GRAVITY;
    // }
    FastLED.addLeds<NEOPIXEL, 1>(strip, 1);
    FastLED.setBrightness(255);
    FastLED.clear();
    // for (uint8_t i = 0; i < LED_COUNT; i++) {

        // strip[i] = CRGB::Red;
        // error = setContext(
        //     &ctx[i],
        //     MAX_STATIONARY/2,
        //     random(HUE_RED, HUE_ORANGE),
        //     i
        // );
        // if (error < SUCCESS) {
        //     foreverError();
        // }
    // }

    FastLED.show();
}

void loop() {
    // int8_t error = 0;
    // uint8_t color = 0;
    // uint8_t speed = MAX_FREQ;
    // uint8_t freq = MAX_FREQ;

    // float mag = calculateMagnitude();
    // float mag = 0.10;
    for (uint8_t idx = 0; idx < 1; idx++) {
        strip[idx] = CRGB::Red;
    //     error = sinWave(&ctx[idx]);
    //     if (error < SUCCESS) foreverError();

    //     if (ctx[idx].brightness <= CHANGE_COLOR && ctx[idx].toggledColor == false) {
    //         color = uint8_t(random(HUE_RED, HUE_ORANGE));
    //         // if (mag < 0.10) {
    //         //     speed = uint8_t(MAX_SPEED/3);
    //         //     freq = uint8_t(MAX_FREQ/3);
    //         // } else if (mag < 0.20) {
    //         //     speed = uint8_t(MAX_SPEED * (2/3));
    //         //     freq = uint8_t(MAX_FREQ * (2/3));
    //         // } else {
    //         //     speed = MAX_SPEED;
    //         //     freq = MAX_FREQ;
    //         // }
    //         ctx[idx].speed = uint8_t(random(1,speed));
    //         ctx[idx].freq = uint8_t(random(1,freq));
    //         ctx[idx].toggledColor = true;
    //     } else {
    //         color = ctx[idx].color;
    //         ctx[idx].toggledColor = false;
    //     }
    //     error = setPixelColor(&ctx[idx], color);
    //     if (error < SUCCESS) foreverError();
    }

    FastLED.show();
    FastLED.delay(1000/10);
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
    ctx->phase = uint8_t(random(24));
    ctx->speed = uint8_t(random(1,24));
    ctx->freq = uint8_t(random(1,24));
    ctx->toggledColor = false;
    setPixelColor(ctx, color);
    return 0;
}

int sinWave(firectx* ctx) {
    ctx->phase += ctx->speed;
    ctx->brightness = quadwave8(ctx->freq*ctx->phase);
    if (ctx->brightness > 248) {
        ctx->brightness = 248;
    } else if (ctx->brightness < 24) {
        ctx->brightness = 24;
    }
    Serial.println(ctx->brightness);
    return 0;
}

int setPixelColor(firectx* ctx, uint8_t color) {
    ctx->color = color;
    // TODO: I want to break this out at some point...
    strip[ctx->index] = CHSV(ctx->color, 255, ctx->brightness);
    return 0;
}

float calculateMagnitude() {
    sensors_event_t event = {0};
    accel.getEvent(&event);
    float cur_avg = 0;
    cur_avg += ((event.acceleration.x / max_sensor_value)>0?(event.acceleration.x / max_sensor_value):-(event.acceleration.x / max_sensor_value));
    // avg += abs(event.acceleration.y / max_sensor_value);
    cur_avg += ((event.acceleration.z / max_sensor_value)>0?(event.acceleration.z / max_sensor_value):-(event.acceleration.z / max_sensor_value));
    cur_avg /= 2;
    avg.accel_readings[avg.index] = cur_avg;
    avg.index++;
    if (avg.index >= 5) {
        avg.index = 0;
    }
    cur_avg = 0;
    for (uint8_t i = 0; i < 5; i++) {
        cur_avg += avg.accel_readings[i];
    }
    cur_avg /= 5;

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
