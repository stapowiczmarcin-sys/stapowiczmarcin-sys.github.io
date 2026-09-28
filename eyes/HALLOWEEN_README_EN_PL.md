# Kora Halloween Eye V1.1 — EN / PL

Marcin Stapowicz / Kreatywny Morrcin
https://stapowiczmarcin-sys.github.io/eyes/#halloween

## English
One spooky eye on one ILI9341 240 × 320 SPI display, in portrait orientation, driven by ESP32-S3. This is a separate sketch from the two-round-display photographic eyes.

Source: Kora_Halloween_Eye_S3_V1.1_PIR_EDGE_SLEEP.ino, 27 September 2026. Firmware bytes are unchanged. The folder and sketch were renamed together for Arduino IDE; this guide was added.

1. Unzip and open Kora_Halloween_Eye_S3_V1_1_PUBLIC/Kora_Halloween_Eye_S3_V1_1_PUBLIC.ino.
2. Select ESP32S3 Dev Module with Arduino-ESP32 core 3.x. Install Adafruit GFX Library and Adafruit ILI9341, including their Library Manager dependencies.
3. Use the GPIO table below. Match the display, PIR and backlight supply arrangements to your actual modules; GPIO signals use 3.3 V logic. Never apply 5 V to an ESP32 GPIO.
4. Compile and upload; Serial Monitor is 115200 baud. This publishing session did not compile the ESP32 firmware or test hardware.

| Signal | ESP32-S3 GPIO |
| --- | --- |
| SCK / SCL | 6 |
| MOSI / SDA | 7 |
| MISO (reserved for touch; not needed by TFT) | 2 |
| TFT CS | 10 |
| TFT DC | 3 |
| TFT RST | 4 |
| TFT BL control | 5 |
| Touch CS (held HIGH, touch unused) | 8 |
| PIR OUT | 12 |

The firmware starts asleep. The PIR must first return LOW; a new LOW-to-HIGH edge wakes the eye. A held HIGH does not keep it awake. The eye starts closing 3–4 seconds after the last new edge; closing and fading take additional time. Re-arming requires LOW before another HIGH. The screen receives DISPOFF and SLPIN at sleep, and PWM controls the backlight.

It uses a 240 × 220 RGB565 canvas (105,600 bytes) and a 12 MHz SPI setting. FRAME_MS=45 is a scheduling target, not a measured frame rate. There are no external bitmap files, Wi-Fi settings or servo control in this sketch. The webpage preview is a still rendered from the drawing functions with host-side graphics primitives, not a photograph or a hardware performance test.

## Polski
Jedno upiorne oko na jednym ekranie SPI ILI9341 240 × 320 w pionie, sterowane przez ESP32-S3. To osobny program od realistycznych oczu na dwóch okrągłych ekranach.

Źródło: Kora_Halloween_Eye_S3_V1.1_PIR_EDGE_SLEEP.ino z 27 września 2026. Bajty programu pozostają bez zmian. Folder i szkic otrzymały zgodną nazwę dla Arduino IDE; dodano tę instrukcję.

1. Rozpakuj ZIP i otwórz Kora_Halloween_Eye_S3_V1_1_PUBLIC/Kora_Halloween_Eye_S3_V1_1_PUBLIC.ino.
2. Wybierz ESP32S3 Dev Module i rdzeń Arduino-ESP32 3.x. Zainstaluj Adafruit GFX Library oraz Adafruit ILI9341 wraz z zależnościami.
3. Podłącz sygnały według tabeli GPIO powyżej. Zasilanie ekranu, PIR i sterowanie podświetleniem dopasuj do konkretnych modułów. GPIO używa logiki 3,3 V — nie podawaj na nie 5 V.
4. Skompiluj i wgraj; monitor portu: 115200 baud. W tej sesji publikacji nie kompilowano firmware’u ESP32 ani nie wykonywano testu sprzętowego.

Program startuje we śnie. PIR musi najpierw wrócić do LOW; nowe zbocze LOW→HIGH wybudza oko. Stały HIGH nie przedłuża czuwania. Oko zaczyna zamykanie 3–4 sekundy po ostatnim nowym zboczu; zamykanie i wygaszanie trwają dodatkowo. Kolejne wybudzenie wymaga LOW przed nowym HIGH. Przy zasypianiu ekran otrzymuje DISPOFF i SLPIN, a podświetlenie jest sterowane PWM.

Canvas RGB565 ma 240 × 220 pikseli (105 600 bajtów), SPI ustawiono na 12 MHz. FRAME_MS=45 to założony odstęp, nie zmierzona liczba klatek. Szkic nie wymaga plików bitmap i nie zawiera Wi-Fi ani sterowania serwami. Podgląd na stronie to nieruchomy render funkcji rysujących z użyciem komputerowych odpowiedników prymitywów graficznych, nie zdjęcie sprzętu ani test wydajności.

Use and adapt the sketch; credit Marcin Stapowicz / Kreatywny Morrcin and link to the project. Third-party libraries retain their licences.
Używaj i dostosowuj kod; zachowaj podpis Marcin Stapowicz / Kreatywny Morrcin oraz link do projektu. Biblioteki zewnętrzne zachowują swoje licencje.
