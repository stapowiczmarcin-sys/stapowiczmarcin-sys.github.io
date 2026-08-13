/*
  Ręcznie wykonana lampa RGB / Hand-built RGB lamp
  ESP32 SuperMini + 2 niezależne strefy WS2812B

  Strefa dolna / Lower zone: 100 LED, GPIO 4
  Korona / Crown:             30 LED, GPIO 5

  Biblioteka / Library: FastLED
  Uwaga: dopasuj numery pinów do swojej płytki i okablowania.
  Note: adjust the pin numbers to match your board and wiring.
*/

#include <FastLED.h>

#define BOTTOM_PIN 4
#define CROWN_PIN 5

#define BOTTOM_LED_COUNT 100
#define CROWN_LED_COUNT 30
#define BRIGHTNESS 80

CRGB bottomLeds[BOTTOM_LED_COUNT];
CRGB crownLeds[CROWN_LED_COUNT];

uint8_t hue = 0;

void setup() {
  FastLED.addLeds<WS2812B, BOTTOM_PIN, GRB>(bottomLeds, BOTTOM_LED_COUNT);
  FastLED.addLeds<WS2812B, CROWN_PIN, GRB>(crownLeds, CROWN_LED_COUNT);

  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop() {
  // Dolna część: przesuwająca się tęcza.
  // Lower section: moving rainbow.
  fill_rainbow(bottomLeds, BOTTOM_LED_COUNT, hue, 3);

  // Korona: jeden kolor z łagodnym pulsowaniem jasności.
  // Crown: one colour with a gentle brightness pulse.
  uint8_t crownBrightness = beatsin8(20, 70, 255);
  fill_solid(
    crownLeds,
    CROWN_LED_COUNT,
    CHSV(hue + 128, 255, crownBrightness)
  );

  FastLED.show();
  hue++;
  delay(30);
}
