# Kora TFT Eyes V7.4 — single ST7789 display

**Beautiful eyes. No makeup required.** A RAM canvas keeps the construction of each eye off the visible display.

Guide: https://stapowiczmarcin-sys.github.io/eyes/  
Portfolio: https://stapowiczmarcin-sys.github.io/  
Creator: Marcin Stapowicz / Kreatywny Morrcin  
Release date: 25 September 2026

## What this download is

The V7.4 “NEON REAL MAX / CRT Edition” sketch supplied by Marcin, prepared for public sharing. It renders **two eyes on ONE rectangular ST7789V display**, using an ESP32-WROOM. The source identifies the display module as **GMT020-02-7P v1.3**, physical resolution **240 × 320**, landscape rotation **320 × 240**.

This is not the later two-round-GC9A01 version. That version is deferred until physical testing.

Features present in the source: moods, gaze, blinking, a sleep-first boot, sleep breathing, CRT-style wake/sleep transitions, serial commands and optional ear-servo control. Network features include a local HTTP API, Wi-Fi provisioning and OTA when explicitly enabled. `TALK` animates the eyes; the firmware does not synthesize speech. Presence commands come from a host; this sketch does not detect people by itself.

## What changed for public release

- The original fixed AP and OTA passwords were removed, including copies embedded in status messages.
- `ENABLE_WIFI` defaults to `false`. USB serial works without network configuration. This also prevents reuse of Wi-Fi credentials already saved on a previously used ESP32 until networking is enabled.
- `AP_PASS` and `OTA_PASSWORD` are empty. The setup AP requires a private password of 12–63 characters; OTA requires at least 12 characters. Without them the corresponding service remains disabled.
- The setup AP no longer prints its password to serial. The ready banner identifies the public copy.
- The GPIO map, eye renderer, CRT mask and `drawFace()` logic were preserved. The original uploaded source was not edited.

## Set up in Arduino IDE

1. Extract the ZIP and open `KORA_TFT_EYES_V7_4_PUBLIC/KORA_TFT_EYES_V7_4_PUBLIC.ino`. The folder and sketch names must match.
2. Install **esp32 by Espressif Systems 3.x** through Boards Manager. For a compatible ESP32-WROOM development board, select **ESP32 Dev Module**. The watchdog code targets the ESP-IDF 5.x API; do not assume compatibility with Arduino-ESP32 2.x, ESP8266, or ESP32-C3.
3. Install **Adafruit GFX Library**, **Adafruit ST7735 and ST7789 Library**, and **ESP32Servo**, accepting Library Manager dependencies such as Adafruit BusIO. WiFi, WebServer, Preferences, ESPmDNS and ArduinoOTA come with the ESP32 board package.
4. Check the signal map below against your physical board. It uses GPIO numbers, not header positions.
5. Use **Verify**, then upload over USB.
6. Open Serial Monitor at **115200 baud**, with **Newline** (or both CR and LF).
7. The firmware starts asleep. Send `WAKE 60000` to wake the eyes for one minute. Then try `MOOD happy`, `M`, `STATUS`, and `SLEEP`, one command per line.

## Signal map from the source

| Function | ESP32 GPIO |
| --- | --- |
| TFT SCLK / SCL | 18 |
| TFT MOSI / SDA | 23 |
| TFT DC | 16 |
| TFT reset / RES | 17 |
| TFT CS, if exposed | 27 |
| Left ear servo signal | 25 |
| Right ear servo signal | 26 |

The display uses SPI; a pin labelled SDA here is SPI data, not an I²C bus instruction. Some ST7789 modules have no exposed CS pin. Follow the actual module documentation rather than adding a nonexistent connection. The source does not specify a VCC or backlight voltage. Follow your module's supply and backlight ratings; ESP32 GPIO uses 3.3 V logic. Connect the grounds appropriately.

Ear servos are optional hardware, but the original firmware still attaches GPIO25/26 at boot. It uses centres of 90°, limits of 55–125°, 50 Hz and an attach pulse range of 500–2400 µs. Those are source settings, not a promise that they are safe for every mechanism. Check your own mechanical travel before fitting horns. Use a suitable external servo supply and a common ground; do not power the servo load through the ESP32.

## Serial examples

```text
WAKE 60000
MOOD happy
M
STATUS
LIST
SLEEP
```

Send each command separately. `WAKE` without a number defaults to 8 seconds. `LIST` prints available expression names. `MOOD angry`, `MOOD curious` and `MOOD thinking` select other expressions. `TALK ON` and `THINK ON` require awake mode. `EYESCFG blinkmin=1800 blinkmax=5200` changes and saves the blink timing.

## Optional Wi-Fi, setup AP and OTA

The download works over USB with networking off. If you need networking:

1. Set `ENABLE_WIFI = true` near the top of the sketch.
2. Choose your own `AP_PASS` of 12–63 characters and `OTA_PASSWORD` of at least 12 characters. Leave them private. An empty or too-short value disables the associated AP/OTA feature.
3. Recompile and upload over USB.
4. In Serial Monitor, send `WIFI your-network-name|your-network-password`. These are placeholders; use your own values. The sketch saves them in ESP32 Preferences and restarts. Alternatively, join the `Kora-Eyes-Setup` AP if you configured its password, then use the setup address printed over serial.
5. The board prints its local IP. The source attempts the mDNS name `kora-eyes.local`; availability depends on your network.

The inherited HTTP API has no authentication, and its setup form uses HTTP. Use it only on a trusted local network; do not expose it with router port forwarding. The OTA password protects OTA, not the HTTP API. Prefer serial provisioning so Wi-Fi credentials are not put into a setup URL. Turning networking back off does not erase previously stored credentials.

Example local API paths from the source: `/api/state`, `/api/wake?ms=60000`, `/api/mood?name=happy`, `/api/sleep`. Append them to your own device's reported address.

## How the canvas works here

`GFXcanvas16` stores RGB565 pixels in RAM: 2 bytes per pixel. V7.4 tries **160 × 240 = 76,800 bytes (75 KiB)** for one eye, then reuses that buffer for the other eye. A full 320 × 240 image would require 153,600 bytes for pixels alone.

The fallback sizes are 152 × 224, 144 × 208 and 128 × 180. The source checks `getBuffer()` after allocation; if no allocation succeeds, it skips drawing and prints an error.

The sequence in `drawFace()` is:

1. Render the left eye and CRT mask in RAM.
2. Copy the completed region to the left part of the TFT with `drawRGBBitmap()`.
3. Render the right eye into the same RAM.
4. Copy it to the right part of that same TFT.

This hides intermediate drawing and reduces erase/redraw flicker. It is not a full-screen double-buffer swap or a guarantee of zero tearing. The two updates occur sequentially. If adapting this design to asynchronous DMA, do not overwrite the buffer until its transfer is finished.

For comparison only, a 240 × 240 round-display canvas would use 115,200 bytes. At an assumed 40 MHz SPI clock, its payload alone would take 23.04 ms, or 46.08 ms for two such images. This is a calculation, not a measured frame rate. The single-screen V7.4 code does not explicitly request 40 MHz. Drawing, driver overhead, network handling and the actual clock affect speed. The 33 ms guard in `drawFace()` limits when drawing starts; it cannot guarantee 30 fps.

## Validation and known limits

Build verification is recorded in `BUILD.txt` included with this release.

No ESP32, TFT or servos were connected in the publication environment, so a successful compile is not a physical test. The public configuration needs a check on your own hardware. Small position drift is inherited from V7.4: when adapting the renderer, inspect update borders for stale pixels and cover the old area if needed. This release does not change Kora's Pi services, legs, Servo2040 or navigation.

## Use and credit

Marcin shares this eyes sketch free for other builders to use and adapt in their own projects. Please retain credit to **Marcin Stapowicz / Kreatywny Morrcin** and the guide link. This permission is for this eyes sketch, not a licence for the entire portfolio. Third-party libraries retain their own licences and are installed separately.

Official references:

- https://learn.adafruit.com/adafruit-gfx-graphics-library/minimizing-redraw-flicker
- https://adafruit.github.io/Adafruit-GFX-Library/html/class_g_f_xcanvas16.html
- https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_SPITFT.cpp
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/mem_alloc.html

---

# Oczy Kory V7.4 — jeden wyświetlacz ST7789

**Piękne oczy bez makeupu.** Canvas w RAM pozwala złożyć obraz oka poza widocznym ekranem.

Poradnik po polsku: https://stapowiczmarcin-sys.github.io/eyes/?lang=pl

To publiczna kopia dostarczonego kodu V7.4 „CRT Edition”: **ESP32-WROOM i jeden ST7789V 240 × 320**, po obrocie **320 × 240**. Dwoje oczu mieści się na jednym ekranie. Wersja na dwa okrągłe GC9A01 będzie osobnym wydaniem po testach sprzętowych — nie jest częścią tej paczki.

## Uruchomienie

1. Rozpakuj ZIP. Otwórz `KORA_TFT_EYES_V7_4_PUBLIC.ino` w folderze o tej samej nazwie.
2. W Arduino IDE zainstaluj pakiet **esp32 by Espressif Systems 3.x**. Dla zgodnej płytki ESP32-WROOM wybierz **ESP32 Dev Module**. Kod watchdoga używa API ESP-IDF 5.x; pakiet 2.x, ESP8266 i ESP32-C3 nie są tym samym celem kompilacji.
3. Zainstaluj **Adafruit GFX Library**, **Adafruit ST7735 and ST7789 Library**, **ESP32Servo** oraz ich zależności.
4. Sprawdź połączenia według tabeli powyżej: SCLK/SCL **18**, MOSI/SDA **23**, DC **16**, RST/RES **17**, CS **27**, jeśli moduł ma taki pin. Uszy: sygnały **25/26**. Są to numery GPIO, nie pozycje na złączu. SDA w tym module oznacza dane SPI.
5. VCC i podświetlenie dobierz według dokumentacji swojego ekranu; kod nie określa tych napięć. GPIO ESP32 używa logiki 3,3 V. Połącz właściwie masy.
6. Sprawdź kompilację i wgraj przez USB. Monitor portu: **115200 baud**, zakończenie **nowa linia**.
7. Oczy startują we śnie. Wyślij `WAKE 60000`, potem osobno `MOOD happy`, `M`, `STATUS` lub `SLEEP`. `LIST` wypisuje nastroje. Samo `WAKE` oznacza 8 sekund.

Serwa uszu nie są wymagane do oglądania oczu, lecz program wystawia sygnały na GPIO25/26 od startu. Zakres w kodzie to 55–125° z centrum 90°, częstotliwość 50 Hz i zakres impulsów attach 500–2400 µs. Nie kopiuj tych nastaw bez sprawdzenia swojej mechaniki. Użyj odpowiedniego zewnętrznego zasilania serw i wspólnej masy; nie zasilaj obciążenia serw przez ESP32.

## Co zmieniono w kopii publicznej

Usunięto stałe hasła AP/OTA i ich kopie z komunikatów. `ENABLE_WIFI` ma domyślnie wartość `false`, więc działanie przez USB nie wymaga sieci i nie uruchamia zapisanej wcześniej konfiguracji Wi-Fi. Wygląd oczu, `renderEye()`, `applyCrtMask()`, `drawFace()` i mapa GPIO zostały zachowane. Oryginalny załącznik Marcina pozostał bez zmian.

Aby włączyć sieć, ustaw `ENABLE_WIFI = true`, własne `AP_PASS` (12–63 znaki) i `OTA_PASSWORD` (minimum 12 znaków). Wgraj ponownie. Przez monitor portu podaj `WIFI nazwa-sieci|haslo-sieci`. To wzór — wpisz własne dane. Zostaną zapisane w Preferences, po czym płytka się zrestartuje. Możesz też użyć punktu `Kora-Eyes-Setup`, jeśli ustawiłeś jego hasło, i adresu strony podanego w monitorze portu.

Oryginalne API HTTP nie ma logowania, a formularz konfiguracji używa HTTP. Używaj wyłącznie zaufanej sieci lokalnej, bez wystawiania portów do internetu. Hasło OTA nie zabezpiecza API HTTP. Konfiguracja przez port szeregowy nie umieszcza hasła Wi-Fi w adresie strony. Wyłączenie sieci nie kasuje zapisanych danych.

## Canvas bez niepotrzebnej magii

`GFXcanvas16` używa RGB565, czyli **2 bajtów na piksel**. Maksymalny bufor tej wersji to **160 × 240 = 76 800 bajtów**. Po wysłaniu lewego oka program używa tego samego bufora do prawego oka. Nie musi przechowywać całej klatki 320 × 240 w RAM. Przy braku pamięci próbuje kolejno 152 × 224, 144 × 208 i 128 × 180; jeśli alokacja nadal się nie uda, pomija rysowanie i zgłasza błąd.

Canvas ukrywa etapy rysowania i ogranicza migotanie, ale nie gwarantuje braku rozrywania klatki ani idealnie jednoczesnego odświeżenia obu oczu. Aktualizacje są kolejne. Przy przeróbce na asynchroniczne DMA poczekaj na zakończenie przesyłania przed ponownym użyciem bufora.

Przykładowe 240 × 240 to **115 200 bajtów**. Przy założonym SPI 40 MHz przesłanie samych danych trwa co najmniej 23,04 ms, a dwóch obrazów 46,08 ms. To wyliczenie, nie pomiar i nie ustawienie tego firmware’u. Warunek 33 ms nie gwarantuje 30 klatek/s. Pamięć reklamowana jako „520 KB SRAM” nie jest w całości dostępnym miejscem na canvas: liczą się wolna pamięć, największy wolny blok i wynik alokacji.

## Weryfikacja i udostępnianie

Wynik kompilacji zapisano w `BUILD.txt`. W środowisku publikacji nie było podłączonej płytki, wyświetlacza ani serw — kompilacja nie zastępuje testu fizycznego. Własną konstrukcję sprawdź na sprzęcie. Przy małym dryfie obrazu odziedziczonym z V7.4 obejrzyj krawędzie obszarów aktualizacji. Publikacja nie zmienia usług Pi, nóg, Servo2040 ani nawigacji Kory.

Kod oczu jest udostępniony bezpłatnie do używania i dostosowywania we własnych konstrukcjach. Zachowaj informację **Marcin Stapowicz / Kreatywny Morrcin** i link do poradnika. Dotyczy to tego szkicu, nie całego portfolio. Biblioteki zewnętrzne mają własne licencje i instaluje się je osobno.
