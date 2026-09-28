# Kora Animal Eyes V14 — EN / PL

## English
Photographic animal eyes with amber irises and vertical pupils, for ESP32-S3 N16R8 and two round GC9A01 240 × 240 displays. Textures are embedded in the sketch; no SD card is required. The preview shows the embedded open-eye texture, mirrored for the pair, not a hardware photograph or a live animation.

Open `KoraAnimalEyesV14_S3_PUBLIC.ino` from its matching folder. Use Arduino ESP32 core 3.x, ESP32S3 Dev Module, 16 MB flash and OPI PSRAM; choose a partition with enough application space (Huge APP). Libraries: Adafruit GFX, Adafruit GC9A01A, ESP32Servo compatible with S3, and their dependencies.

| Signal | GPIO |
|---|---|
| Shared SCK / MOSI | 12 / 11 |
| Left CS / DC / RST | 10 / 9 / 8 |
| Right CS / DC / RST | 7 / 6 / 5 |
| Left / right ear | 15 / 16 |
| Left / right FSR | 1 / 2 |
| Left / right mouth servo | 17 / 18 |

Use suitable external servo power and a shared ground; do not power servos from ESP32 GPIO. Calibrate mouth endpoints with the linkage disconnected. This firmware includes servo control; some eye commands also trigger the ears.

USB Serial: 115200 baud, newline. `M` requests a blink; `SLEEP` closes the lids, holds for 5 seconds, fades for 1.2 seconds, then sleeps the display. `WAKE` / `WAKE UP` wakes the eyes. Hardwired backlight LEDs cannot be switched off by software. `STATUS` reports state. `TALK ON` uses fallback mouth animation; `AUDIO 0..100` supplies audio level while talking; `TALK OFF` ends talking.

Public-copy changes: embedded AP/OTA passwords and password log output were removed. Set your own `AP_PASS` (12–63 characters) and `OTA_PASSWORD` (at least 12 characters). Until both are configured, Wi-Fi stays off and USB control remains available. The original HTTP control API has no login; use it only on a trusted local network.

Animation, textures and GPIO mapping are unchanged. This publication was not recompiled or tested on hardware. This is a separate V14 download; the three V18 releases remain available.

## Polski
Fotograficzne oczy zwierzęce z bursztynowymi tęczówkami i pionowymi źrenicami, dla ESP32-S3 N16R8 i dwóch okrągłych ekranów GC9A01 240 × 240. Tekstury są w kodzie; karta SD nie jest potrzebna. Podgląd przedstawia zapisaną w kodzie teksturę otwartego oka, odbitą dla pary — nie zdjęcie sprzętu ani animację na żywo.

Otwórz `KoraAnimalEyesV14_S3_PUBLIC.ino` w folderze o tej samej nazwie. Arduino ESP32 core 3.x, ESP32S3 Dev Module, flash 16 MB, PSRAM OPI; partycja z wystarczającym miejscem na aplikację (Huge APP). Biblioteki: Adafruit GFX, Adafruit GC9A01A, ESP32Servo zgodna z S3 oraz ich zależności. Mapa GPIO znajduje się w tabeli powyżej.

Serwa wymagają odpowiedniego zewnętrznego zasilania i wspólnej masy; nie zasilaj ich z GPIO. Zakres ust ustawiaj przy odłączonym cięgnie. Kod obsługuje serwa; część komend oczu uruchamia również uszy.

Monitor portu: 115200 baud, nowa linia. `M` wywołuje mrugnięcie. `SLEEP` zamyka powieki, utrzymuje je przez 5 sekund, wygasza obraz przez 1,2 sekundy i usypia ekran. `WAKE` / `WAKE UP` wybudza oczy. Program nie wyłączy podświetlenia podłączonego na stałe do zasilania. `STATUS` pokazuje stan. `TALK ON` uruchamia zastępczą animację ust; `AUDIO 0..100` podaje poziom dźwięku podczas mówienia; `TALK OFF` kończy mówienie.

Zmiany w kopii publicznej: usunięto zapisane hasła AP/OTA i ich wypisywanie w logach. Ustaw własne `AP_PASS` (12–63 znaki) i `OTA_PASSWORD` (minimum 12 znaków). Do tego czasu Wi-Fi pozostaje wyłączone; sterowanie USB działa. Oryginalne API HTTP nie ma logowania — używaj go wyłącznie w zaufanej sieci lokalnej.

Animacja, tekstury i mapa GPIO pozostały bez zmian. Przy tej publikacji nie powtarzano kompilacji ani testów sprzętowych. To osobna wersja V14; trzy wersje V18 nadal są dostępne.

Marcin Stapowicz / Kreatywny Morrcin
https://stapowiczmarcin-sys.github.io/eyes/
