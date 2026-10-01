# KORA Weather WOW 5.5 / ESP32-C3 + ILI9341

PL: Otwórz KoraWeatherWOW55.ino. Wpisz własne WIFI_SSID i WIFI_PASS. Cały folder musi pozostać razem ze wszystkimi plikami .h. ESP32C3 Dev Module, flash 4 MB, Huge APP (3MB No OTA/1MB SPIFFS), USB CDC On Boot Enabled. Biblioteki: Adafruit GFX, Adafruit ILI9341, Adafruit BusIO, ArduinoJson 7. Pogoda z Open-Meteo, bez klucza API. Cztery widoki: pogoda, 3 dni prognozy, ciśnienie/wiatr/wilgotność, zegar. PL/EN, cztery kraje, Snake, Breakout, Pac-Man, Space Invaders, Lucky Reels oraz generator Lotto. Przycisk enkodera: wybór/start/pauza; dodatkowy BAK: powrót. Ekran 240x320 pionowo.

EN: Open KoraWeatherWOW55.ino and enter your own WIFI_SSID / WIFI_PASS. Keep every .h file next to the sketch. Select ESP32C3 Dev Module, 4 MB flash, Huge APP, USB CDC On Boot Enabled. Install Adafruit GFX, Adafruit ILI9341, Adafruit BusIO and ArduinoJson 7. Open-Meteo requires no API key. Weather, three-day forecast, pressure/wind/humidity and local clock; PL/EN; four city presets and six games/tools.

## Wiring / Połączenia
TFT: VCC and LED 3.3V (verify your module), GND-GND, CS-GPIO10, RESET-GPIO5, DC-GPIO4, SDI/MOSI-GPIO7, SCK-GPIO6, SDO/MISO-GPIO2 (optional if SDO absent). SD card is not used.
Encoder: A/CLK-GPIO0, B/DT-GPIO1, SW-GPIO9, common-GND. If encoder module requires VCC use 3.3V. Back button: GPIO8 to GND. Release SW and BAK during power-up/reset; GPIO2/8/9 are boot strapping pins.

PL: Schemat na stronie dotyczy wersji USB. Obudowa ma miejsce na koszyk 18650, lecz model modułu zasilania baterii nie został jeszcze potwierdzony. Nie podłączaj ogniwa bezpośrednio do 3V3. Zdjęcia strony to opisany prototyp i podglądy programowe z przykładowymi danymi. Model STL nie był fizycznie drukowany.
EN: Wiring covers USB power. Enclosure fits an 18650 holder; the battery power/charging module is not yet specified. Do not connect a cell directly to 3V3. Software previews use sample weather data. STL geometry was verified; no physical test print yet.

Compile: ESP32 package 3.3.12, flash 2242309 bytes (71%), globals 41456 bytes (12%). The public version only replaces Wi-Fi credentials and documentation; rendering code is unchanged. Photo sources: ZRODLA.md. Software-rendered previews are not measured display performance.

## SM5308 — provisional / identyfikacja wstępna
Single-cell holder → module BAT terminals; module USB-A 5V output → ESP32 USB. Module USB-C is the charge input. Verify your board markings and polarity first. Exact module identity is not confirmed. / Koszyk do BAT, USB-A 5V do USB ESP32, USB-C modułu do ładowania. Najpierw sprawdź oznaczenia własnej płytki.


## Pełna instrukcja / Full illustrated setup guide
https://stapowiczmarcin-sys.github.io/projects/kora-weather-arcade/?lang=pl#setup

Open-Meteo: https://open-meteo.com/en/docs — no API key / bez klucza. Wi-Fi 2.4 GHz.

Tools: ESP32C3 Dev Module; USB CDC On Boot Enabled; CPU 160MHz (WiFi); Flash 4MB; Huge APP (3MB No OTA/1MB SPIFFS); default QIO / 80MHz; Upload Speed 115200; Core Debug None; Erase All Flash Disabled; Port = your board USB port, not a fixed COM number.

First upload / pierwsze wgranie: hold BOOT, press and release RESET, release BOOT, select the new USB port, Upload, then RESET. / Trzymaj BOOT, naciśnij i puść RESET, puść BOOT, wybierz port USB, Wgraj, potem RESET.

Boards manager URL: https://espressif.github.io/arduino-esp32/package_esp32_index.json
Package / pakiet: esp32 by Espressif Systems (tested / testowano 3.3.12).
Libraries / biblioteki: Adafruit GFX Library, Adafruit ILI9341, Adafruit BusIO, ArduinoJson 7.x.
