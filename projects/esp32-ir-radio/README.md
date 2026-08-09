# ESP32 Internet Radio — preview before publication

This is a deliberately small reference implementation reconstructed for Marcin's portfolio.
It is **not published** and is **not yet hardware-tested**.

## Reference hardware

- ESP32-WROOM-32 development board (`ESP32 Dev Module`)
- PCM5102A I²S DAC
- 3 simple digital IR sensors powered from 3.3 V
- addressable WS2812B/NeoPixel-style **RGB** strip
- default `LED_COUNT`: 82

## IR mapping

- GPIO32: next station
- GPIO33: radio ON/OFF
- GPIO14: LEDs ON/OFF

The reconstructed version deliberately avoids GPIO12, a boot-strapping pin on the
classic ESP32, to reduce the chance of a sensor preventing the board from booting.

The code intentionally contains only Wi-Fi connection, station cycling, radio
ON/OFF and LED ON/OFF. It is a tutorial-sized sketch, not a framework.

## Audio and LEDs

- PCM5102A: BCK GPIO27, LCK/LRCK GPIO26, DIN GPIO25
- WS2812B DATA: GPIO4 through a 330–470 ohm series resistor
- LED byte order: `NEO_GRB`; change to `NEO_RGB` if colours are swapped
- use `NEO_GRBW` only for an RGBW strip with a separate white channel

## Arduino IDE

- Board: `ESP32 Dev Module`
- Flash size: match the physical module (commonly 4 MB, but verify)
- Partition Scheme: `Huge APP (3 MB No OTA / 1 MB SPIFFS)`
- Serial Monitor: 115200 baud

Libraries:

- `ESP32-audioI2S` 4.0.0 by schreibfaul1
- `Adafruit NeoPixel` 1.15.5

Use the `Audio.h` supplied by ESP32-audioI2S. A different Arduino/SAM library also
named `Audio` is incompatible.

## Before uploading

1. Enter `WIFI_SSID` and `WIFI_PASSWORD` in the sketch.
2. Set `LED_COUNT` to the real pixel count.
3. Confirm that the strip is addressable 5 V WS2812B/NeoPixel-style RGB, not a
   non-addressable 4-wire 12 V RGB strip.
4. Check the exact PCM5102A breakout power label before connecting VIN.
5. Power a long LED strip from a separate regulated 5 V supply and join grounds.
