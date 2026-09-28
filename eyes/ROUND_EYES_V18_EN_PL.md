# Kora V18 — 2 × GC9A01 / ESP32-S3 N16R8
Marcin Stapowicz / Kreatywny Morrcin — https://stapowiczmarcin-sys.github.io/eyes/

## Polski
Trzy kompletne szkice: ludzkie z fragmentami metalu, metalowe i potwór.
Wybierz JEDEN ZIP. Rozpakuj i otwórz .ino w folderze o identycznej nazwie.
Wszystkie tekstury są w kodzie. Nie potrzebujesz VisualAssets.h ani dodatkowych bitmap.

Arduino IDE: ESP32S3 Dev Module, Flash 16 MB, OPI PSRAM, USB CDC On Boot Enabled.
Sprawdzana konfiguracja: esp32 3.3.12; partycje 16M Flash (3MB APP/9.9MB FATFS),
identyfikator app3M_fat9M_16MB, z obsługą OTA.
Biblioteki: Adafruit GFX 1.12.6, Adafruit GC9A01A 1.1.1, ESP32Servo 3.2.1 oraz zależności.
Zachowany jeden współdzielony framebuffer RGB565 w PSRAM (115200 B), używany kolejno dla obu oczu,
oraz dotychczasowy cache tekstury białka. Tablica odcieni zajmuje dodatkowo tylko 512 B.

| Sygnał | Lewy GPIO | Prawy GPIO |
|---|---:|---:|
| SCK | 12 | 12 |
| MOSI | 11 | 11 |
| CS | 10 | 7 |
| DC | 9 | 6 |
| RST | 8 | 5 |
| Uszy | 15 | 16 |
| Buzia | 17 | 18 |
| FSR | 1 | 2 |

Piny, serwa, Wi-Fi, HTTP i dotychczasowe komendy pozostają zachowane.
Ludzkie oczy zachowują ręczne mrugnięcie 500 ms z paczki na stronie; pozostałe warianty zachowują własne animacje.
Domyślnie IRIS ORIGINAL i PUPIL ANIMATED: wcześniejszy wygląd i oddychanie źrenic.

Serial 115200, zakończenie Newline, jedna komenda na linię:
```text
IRIS BLUE
IRIS GREEN
IRIS GREY
IRIS HAZEL
IRIS RED
IRIS PURPLE
IRIS ORIGINAL
PUPIL AUTO
LIGHT 80
PUPIL MANUAL
PUPIL 40
PUPIL ANIMATED
STATUS
```
IRIS działa na obie tęczówki, zachowuje teksturę i jasne neutralne refleksy; nie barwi źrenic, skóry ani białek.
PUPIL 0..100 płynnie ustawia wielkość w ograniczonym zakresie (promień ok. 9.36–20.21 px przy tęczówce 44.89 px),
a nie surowy promień w pikselach. PUPIL MANUAL zatrzymuje ostatnią wielkość; liczba przełącza na tryb ręczny.
PUPIL AUTO zastępuje dotychczasowe oddychanie źrenic i ustawia jednakowy rozmiar w obu oczach.
Jasno oznacza zwężenie, ciemno rozszerzenie. Filtr światła ~350 ms, histereza 2 punkty procentowe,
zwężanie ~350 ms, rozszerzanie ~1100 ms (stałe czasowe, nie czas całego ruchu).

### Czujniki światła
Nie przypisano GPIO: PIN_LIGHT_L i PIN_LIGHT_R wynoszą -1. Oba muszą zostać podane przed uruchomieniem ADC.
Kod sprawdza konflikty pinów i używa ADC1, zachowując konfigurację FSR.
LIGHT_DARK_L / LIGHT_BRIGHT_L i odpowiedniki R to kalibracja każdego czujnika, na razie 0/4095.
Wpisz rzeczywiste odczyty w ciemności i jasnym otoczeniu. Można odwrócić wartości, gdy dzielnik działa odwrotnie.
Kod normalizuje oba odczyty do 0..100, a następnie liczy ich średnią co 50 ms.
Przyjęty jest analogowy sygnał z czujników/dzielników. Dla czujników cyfrowych potrzebny jest ich model.

Gdy czujniki są na C6: C6 powinno uśredniać skalibrowane pomiary i wysyłać LIGHT 0..100 około 10 razy/s.
0 = ciemno, 100 = jasno; to nie surowe ADC 0..4095. LIGHT nie włącza AUTO samoistnie — wyślij PUPIL AUTO.
Komenda korzysta z obecnego kanału Serial, może ją przekazywać Pi. Nie przypisano nowego UART ani jego pinów.
Po 2 s bez LIGHT kod wraca do lokalnych czujników, jeśli są skonfigurowane. W przeciwnym razie utrzymuje
ostatni rozmiar źrenicy. STATUS i /api/state podają źródło i null/NA przy braku aktualnego pomiaru.
Wybór koloru i trybu jest bieżący — po restarcie wracają ORIGINAL / ANIMATED.

### Dotychczasowe sterowanie
WAKE, SLEEP, TALK ON/OFF, AUDIO 0..100, MOUTH 0..100, MOOD, LIST, M i pozostały protokół Pi pozostają w kodzie.
Ludzkie oczy: zamknięcie, 5 s podtrzymania, wygaszenie. Metal/potwór: CRT.
Serwa wymagają wspólnej masy i odpowiedniego osobnego zasilania; zachowano wcześniejsze kalibracje mechanizmu.
W publicznych ZIP-ach wpisz własne AP_PASS i OTA_PASSWORD zamiast placeholderów przed wgraniem.
To jedyna różnica ustawień sieci względem prywatnych szkiców. Publiczne HTTP zachowuje dotychczasowy brak logowania.

### Sprawdzenie
Wyniki kompilacji i testów znajdują się w BUILD_V18.txt. Nie wykonano testów na fizycznej Korze.
Nie deklarujemy pomierzonego FPS ESP32 na podstawie testów komputera. STATUS pokazuje fps i renderMs do porównania na urządzeniu.
Podglądy są wygenerowane z renderera, nie są fotografiami działających wyświetlaczy.

## English
Choose ONE of the three complete single-file sketches: human/metal fragments, full metal, or monster.
Use ESP32S3 Dev Module / 16 MB flash / OPI PSRAM / USB CDC enabled. The tested board package,
library versions, wiring and commands are listed above. No external image headers are required.
One 115200-byte framebuffer remains shared between both displays; the existing sclera cache remains unchanged.

IRIS BLUE/GREEN/GREY/HAZEL/RED/PURPLE recolours the iris texture on both eyes; ORIGINAL restores the source.
Skin, sclera, pupils and bright neutral reflections remain unchanged. Default pupil mode ANIMATED preserves old animation.
PUPIL AUTO uses mean ambient light; PUPIL MANUAL holds the current size; PUPIL 0..100 smoothly selects a bounded size.
PUPIL ANIMATED restores the original behavior. All previous Pi, servo, network and sleep commands remain available.

Both light sensor GPIOs are intentionally -1. Configure actual ADC1 pins and measured dark/bright endpoints for BOTH sensors.
Each input is normalized before averaging at 20 Hz. Default calibration numbers are placeholders, not measured lux.
Analog sensors are assumed; digital sensors need their model-specific driver.
Optional LIGHT 0..100 takes an already averaged, normalized C6 reading over the existing Serial command channel;
send roughly 10 updates/s and explicitly enable PUPIL AUTO. No additional UART wiring is assumed.
After 2 seconds without remote data, use configured local sensors or hold the pupil. STATUS /api/state expose stale/no-data state.
Ambient filtering, 2-point hysteresis and asymmetric pupil adaptation avoid abrupt changes.
Colour/mode settings reset to ORIGINAL/ANIMATED at boot.

Public downloads have AP/OTA password placeholders; replace them before use. Private local copies retain existing settings.
Human/metal keeps the site's manual 500 ms blink and eyelid sleep. Metal and monster keep CRT sleep and their servo logic.
BUILD_V18.txt reports verification. No physical hardware/FPS testing was performed.

Use and adapt with credit to Marcin Stapowicz / Kreatywny Morrcin and link to the project. Third-party libraries retain their licences.
ADC reference: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
