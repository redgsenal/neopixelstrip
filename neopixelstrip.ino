// Neopixel Christmas light - Arduino Nano V3.0 + WS2812B strip
// Button on D2 cycles through 5 animations.
// Potentiometer wiper on A0 controls animation speed (works with any pot value).
// Requires: Adafruit NeoPixel library (Library Manager).

#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define BUTTON_PIN  2
#define NUM_LEDS    60     // change to your strip length
#define BRIGHTNESS  80     // 0-255, keep <= 120 for 60 LEDs on 5V 2A
#define POT_PIN     A0

// Speed range, as a % of each animation's base interval.
// Knob fully "fast" -> SPEED_FAST_PCT, fully "slow" -> SPEED_SLOW_PCT.
// 25 = 4x faster, 280 = 2.8x slower. Adjust to taste.
#define SPEED_FAST_PCT  25
#define SPEED_SLOW_PCT  280

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint8_t NUM_MODES = 5;
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
  }
}
