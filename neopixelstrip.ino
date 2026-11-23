// Neopixel Christmas light - Arduino Nano V3.0 + WS2812B strip
// Button on D2 cycles through 6 animations.
// Potentiometer wiper on A0 controls animation speed (works with any pot value).
// Requires: Adafruit NeoPixel library (Library Manager).

#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define BUTTON_PIN  2
#define NUM_LEDS    120    // 2 strips x 60 LEDs, chained (strip 1 DO -> strip 2 DIN)
#define BRIGHTNESS  80     // 0-255. 80 = ~2.3 A worst case for 120 LEDs; keep <= ~100 with a 3 A fuse (5V 5A supply)
#define POT_PIN     A0

// Speed range, as a % of each animation's base interval.
// Knob fully "fast" -> SPEED_FAST_PCT, fully "slow" -> SPEED_SLOW_PCT.
// 25 = 4x faster, 280 = 2.8x slower. Adjust to taste.
#define SPEED_FAST_PCT  25
#define SPEED_SLOW_PCT  280

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint8_t NUM_MODES = 6;
uint8_t mode = 0;
bool lastBtn = HIGH;
unsigned long lastPress = 0, lastStep = 0, step = 0;

// Reads the pot. Reads twice and keeps the second value, because a
// high-resistance pot (like 400k) needs a moment to settle the ADC.
// Light smoothing removes jitter. Works with any pot value (5V to GND).
int readPot() {
  static int smooth = 512;
  analogRead(POT_PIN);
  smooth = (smooth * 7 + analogRead(POT_PIN)) / 8;
  return smooth;
}

// Any color the LED can show: random hue + random saturation (so pastels
// as well as vivid colors), gamma corrected for natural-looking mid-tones.
uint32_t randomColor() {
  return strip.gamma32(strip.ColorHSV(random(65536), random(110, 256), 255));
}

bool due(unsigned long interval) {
  // knob turned right = faster (swap the pot's outer legs if reversed)
  long pct = SPEED_FAST_PCT + (long)(1023 - readPot()) * (SPEED_SLOW_PCT - SPEED_FAST_PCT) / 1023;
  unsigned long scaled = interval * pct / 100;
  if (millis() - lastStep >= scaled) { lastStep = millis(); return true; }
  return false;
}

void checkButton() {
  bool b = digitalRead(BUTTON_PIN);
  if (lastBtn == HIGH && b == LOW && millis() - lastPress > 200) {
    lastPress = millis();
    mode = (mode + 1) % NUM_MODES;
    step = 0;
    strip.clear();
    strip.show();
  }
  lastBtn = b;
}

// 0: blinking, a new random color each blink
void blinkRGB() {
  if (!due(400)) return;
  if (step % 2) strip.clear();
  else strip.fill(randomColor());
  strip.show(); step++;
}

// 1: running color
void running() {
  if (!due(60)) return;
  strip.clear();
  strip.setPixelColor(step % NUM_LEDS, strip.gamma32(strip.ColorHSV((uint16_t)(step * 600))));
  strip.show(); step++;
}

// 2: random colors
void randomLights() {
  if (!due(120)) return;
  strip.setPixelColor(random(NUM_LEDS), randomColor());
  strip.setPixelColor(random(NUM_LEDS), 0);
  strip.show();
}

// 3: fade in/out, a new random color each cycle
uint16_t fadeHue = 0;
uint8_t fadeSat = 255;
void fadeInOut() {
  if (!due(15)) return;
  uint16_t v = step % 510;
  if (v == 0) { fadeHue = random(65536); fadeSat = random(110, 256); }
  uint8_t level = v < 255 ? v : 510 - v;
  strip.fill(strip.gamma32(strip.ColorHSV(fadeHue, fadeSat, level)));
  strip.show(); step++;
}

// 4: color wave
void colorWave() {
  if (!due(20)) return;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint16_t hue = (uint16_t)(step * 256) + (uint32_t)i * 65536UL / NUM_LEDS;
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  strip.show(); step++;
}

#define MAX_DROPS    10
#define RAIN_REVERSE false   // true = drops fall from the last LED toward LED 0

struct Drop { int16_t pos; uint8_t speed; uint16_t hue; uint8_t sat; bool on; };
Drop drops[MAX_DROPS];
uint8_t rainBuf[NUM_LEDS][3];

// 5: rain, each drop has its own random color and fading tail
void rain() {
  if (step == 0) {                       // first run after the button press
    memset(rainBuf, 0, sizeof(rainBuf));
    for (uint8_t i = 0; i < MAX_DROPS; i++) drops[i].on = false;
    step = 1;
  }
  if (!due(40)) return;

  // fade every tail a little
  for (int i = 0; i < NUM_LEDS; i++)
    for (uint8_t c = 0; c < 3; c++)
      rainBuf[i][c] = ((uint16_t)rainBuf[i][c] * 190U) >> 8;

  // sometimes start a new drop in a free slot
  if (random(100) < 15) {
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
      if (!drops[i].on) {
        drops[i].pos   = 0;
        drops[i].speed = random(6, 17);          // 6..16 sixteenths of an LED per frame
        drops[i].hue   = random(65536);
        drops[i].sat   = random(110, 256);
        drops[i].on    = true;
        break;
      }
    }
  }

  // move each drop and draw its head
  for (uint8_t i = 0; i < MAX_DROPS; i++) {
    if (!drops[i].on) continue;
    drops[i].pos += drops[i].speed;
    int p = drops[i].pos >> 4;
    if (p >= NUM_LEDS) { drops[i].on = false; continue; }
    uint32_t col = strip.gamma32(strip.ColorHSV(drops[i].hue, drops[i].sat, 255));
    rainBuf[p][0] = col >> 16;
    rainBuf[p][1] = col >> 8;
    rainBuf[p][2] = col;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    int led = RAIN_REVERSE ? NUM_LEDS - 1 - i : i;
    strip.setPixelColor(led, rainBuf[i][0], rainBuf[i][1], rainBuf[i][2]);
  }
  strip.show();
}

void setup() {
  randomSeed(analogRead(A1));   // A1 is unconnected, so noise gives a different seed each power-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
}

void loop() {
  checkButton();
  switch (mode) {
    case 0: blinkRGB();     break;
    case 1: running();      break;
    case 2: randomLights(); break;
    case 3: fadeInOut();    break;
    case 4: colorWave();    break;
    case 5: rain();         break;
  }
}
