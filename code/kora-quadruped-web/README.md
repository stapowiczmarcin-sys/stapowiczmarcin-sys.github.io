# Kora Quadruped — free ESP32 code

This is the public release of the four-legged Kora controller used by Marcin Stąpowicz. It runs on an ESP32-WROOM-32 and controls 12 leg servos through a PCA9685 board.

The release includes:

- phone control through a local web panel;
- per-servo web calibration saved in ESP32 NVS;
- forward, reverse, turning and body-position controls;
- dance, wave and combined trick sequences;
- saved automatic-movement sequences;
- optional OLED 128×64 status display;
- optional dual VL53L1X obstacle sensing;
- optional Bluetooth, UART and ESP-NOW control.

## Download

[Download KORA_QUADRUPED_WEB_FREE_CODE.ino](KORA_QUADRUPED_WEB_FREE_CODE.ino)

## Before uploading

1. Open the `.ino` file in Arduino IDE.
2. Select an ESP32-WROOM-32 compatible board.
3. Replace `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD` with your own local Wi-Fi details.
4. Install the libraries used at the top of the file: Adafruit PWM Servo Driver, Adafruit VL53L1X, Adafruit GFX, Adafruit SSD1306 and the ESP32 board package.
5. Check `servo_channel[4][3]` against your wiring.
6. Upload the code and open Serial Monitor at 115200 baud. The ESP32 prints the web-panel IP address after connecting.
7. Lift the robot so the feet are clear of the floor, open the web panel and calibrate every servo before attempting a movement.

Do not copy Marcin's servo offsets blindly. Mechanical zero positions differ between builds. Power the servos from a suitable external supply and connect grounds correctly; do not feed the complete servo load through the ESP32.

## Parts used in Marcin's build

- [Servos](https://amzn.to/4iws8cT)
- [ESP32-WROOM-32, 38 pin](https://amzn.to/3SeHqIP)
- [ESP32 with terminal breakout base](https://amzn.to/3Urmwqs)
- [Female charging lead](https://amzn.to/4yatjU8)
- [OLED 128×64](https://amzn.to/4gAMxep)
- [ANYCUBIC Photon Mono 4 resin printer](https://amzn.to/4zN5Dqw)
- [ANYCUBIC resin used for Kora's printed legs](https://amzn.to/4gnNt71)

The Amazon links above are affiliate links. They do not change the buyer's price.

## Attribution

The inverse-kinematics and basic quadruped-motion foundation originated in the SunFounder four-legged Crawling Robot example (2015, `panerqiang@sunfounder.com`). Marcin's version has been extensively modified with ESP32 networking, PCA9685 control, web calibration, NVS storage, new movement sequences, sensors, OLED feedback and multiple control interfaces.

---

# Kora na czterech nogach — darmowy kod ESP32

To publiczna wersja sterownika czteronożnej Kory używanej przez Marcina Stąpowicza. Program działa na ESP32-WROOM-32 i steruje 12 serwami nóg przez płytkę PCA9685.

Najważniejsze funkcje to sterowanie z telefonu przez lokalny panel WWW, kalibracja każdego serwa zapisywana w pamięci ESP32, nowe ruchy i triki, sekwencje automatyczne oraz opcjonalna obsługa OLED, dwóch czujników VL53L1X, Bluetooth, UART i ESP-NOW.

Przed pierwszym ruchem ustaw własne dane Wi-Fi, sprawdź mapę kanałów, unieś robota nad podłogę i wykonaj pełną kalibrację. Serwa muszą mieć odpowiednie zewnętrzne zasilanie ze wspólną masą z ESP32.
