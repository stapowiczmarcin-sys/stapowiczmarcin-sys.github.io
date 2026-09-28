# Kora round eyes — V13 BLINK500, public copy

Marcin Stapowicz / Kreatywny Morrcin
https://stapowiczmarcin-sys.github.io/eyes/

## English
This is the dual round-display release, derived from KoraEyesCyber_S3_V13_BLINK500.zip (27 September 2026). It is different from the rectangular ST7789 V7.4 sketch.

Hardware: ESP32-S3 N16R8, two 240 × 240 GC9A01 SPI displays. Arduino board: ESP32S3 Dev Module; 16 MB flash; OPI PSRAM enabled; ESP32 Arduino core 3.x. Libraries: Adafruit GFX, Adafruit GC9A01A, ESP32Servo compatible with ESP32-S3 and their dependencies.

1. Extract the ZIP and open KoraEyesCyber_S3_V13_PUBLIC/KoraEyesCyber_S3_V13_PUBLIC.ino. Keep the folder and sketch names identical.
2. Before upload, replace AP_PASS and OTA_PASSWORD with your own different passwords (12–63 characters for AP_PASS). The public file contains placeholders, not the original passwords.
3. Connect the displays using the table below. Supply voltage and backlight wiring depend on your actual display module; GPIO signals are 3.3 V.
4. Compile and upload. Use Serial Monitor at 115200 baud with newline. Try STATUS, WAKE, M, MOOD happy, SLEEP, LIST.
5. To configure home Wi-Fi, send WIFI network_name|network_password over Serial Monitor. The firmware also offers a setup access point when no network is stored. Keep the HTTP control service on a trusted local network; it has no login.

| Signal | Left screen GPIO | Right screen GPIO |
| --- | --- | --- |
| SCK / SCL | 12 | 12 |
| MOSI / SDA | 11 | 11 |
| CS | 10 | 7 |
| DC | 9 | 6 |
| RST / RES | 8 | 5 |

The sketch includes Kora-specific optional hardware: ears GPIO15/16, mouth GPIO17/18 and FSR inputs GPIO1/2. Servo signals initialize at startup. For an eyes-only test leave servos disconnected; do not attach unknown linkages. External servo power must share ground. Calibrate before connecting a mechanism.

Manual blink is 500 ms. This is the BLINK500 variant, not AUTOSLEEP120: startup stays awake, SLEEP closes the lids, holds 5 seconds, fades for 1.2 seconds and sends display sleep. With hardwired backlight LEDs, software cannot physically switch their power off.

Photographic eye textures are embedded in the sketch; no extra image files are needed. The renderer uses PSRAM. Different looks shown in project videos may come from different firmware revisions; this download is explicitly V13 BLINK500.

Public-copy changes: replace AP and OTA passwords with placeholders; correct the README filename. Display/motion logic is unchanged. This public copy has not been compiled or tested on hardware in the publishing session. Verify on your own setup. Libraries retain their licences. Please credit Marcin Stapowicz / Kreatywny Morrcin and link to the project.

## Polski
To wersja na dwa okrągłe wyświetlacze, na podstawie KoraEyesCyber_S3_V13_BLINK500.zip z 27 września 2026. Jest osobnym programem od V7.4 na jeden prostokątny ST7789.

Sprzęt: ESP32-S3 N16R8 i dwa ekrany GC9A01 SPI 240 × 240. W Arduino IDE: ESP32S3 Dev Module, flash 16 MB, włączona pamięć OPI PSRAM, rdzeń ESP32 3.x. Biblioteki: Adafruit GFX, Adafruit GC9A01A, ESP32Servo zgodne z S3 oraz ich zależności.

1. Rozpakuj ZIP i otwórz KoraEyesCyber_S3_V13_PUBLIC/KoraEyesCyber_S3_V13_PUBLIC.ino. Nazwy folderu i szkicu muszą być takie same.
2. Przed wgraniem ustaw własne, różne hasła AP_PASS i OTA_PASSWORD (AP_PASS: 12–63 znaki). Publiczny plik zawiera symbole zastępcze, a nie oryginalne hasła.
3. Podłącz wyświetlacze według tabeli powyżej. SCK/SCL i MOSI/SDA są wspólne. CS, DC i RST są osobne. To numery GPIO, nie numery fizycznych pinów. Zasilanie i podświetlenie zależą od modułu; logika GPIO ma 3,3 V.
4. Skompiluj i wgraj. Monitor portu: 115200 baud, koniec linii: nowa linia. Komendy: STATUS, WAKE, M, MOOD happy, SLEEP, LIST.
5. Domowe Wi-Fi skonfigurujesz komendą WIFI nazwa_sieci|hasło. Gdy brak zapisanej sieci, program uruchamia punkt dostępowy konfiguracji. API HTTP nie ma logowania — używaj go tylko w zaufanej sieci lokalnej.

Kod zawiera też opcjonalne elementy Kory: uszy GPIO15/16, buzię GPIO17/18 oraz czujniki FSR GPIO1/2. Sygnały serw uruchamiają się przy starcie. Przy teście samych oczu pozostaw serwa odłączone; przed podłączeniem mechaniki sprawdź kalibrację i odpowiednie zewnętrzne zasilanie ze wspólną masą.

Ręczne mrugnięcie trwa 500 ms. To wariant BLINK500, nie AUTOSLEEP120: po starcie oczy pozostają otwarte, a SLEEP zamyka powieki, czeka 5 sekund, wygasza obraz przez 1,2 sekundy i usypia sterownik. Podświetlenia podłączonego na stałe do zasilania program nie odetnie fizycznie.

Tekstury oczu są zapisane w szkicu. Renderer wymaga PSRAM. Różne wyglądy z filmów mogą pochodzić z różnych wersji programu; ta paczka zawiera konkretnie V13 BLINK500.

Zmiany kopii publicznej: zastąpienie haseł AP/OTA symbolami zastępczymi i poprawienie nazwy instrukcji. Logika wyświetlania i ruchu pozostaje bez zmian. W tej sesji publikacji nie kompilowano ani nie testowano tej kopii na sprzęcie. Sprawdź ją na własnym układzie. Biblioteki zachowują swoje licencje. Zachowaj podpis Marcin Stapowicz / Kreatywny Morrcin i link do projektu.
