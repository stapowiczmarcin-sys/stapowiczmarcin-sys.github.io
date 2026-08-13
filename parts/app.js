"use strict";

const parts = [
  {
    "id": "rpi5-16",
    "name": "Raspberry Pi 5 — 16 GB",
    "short": "Główny komputer Kory",
    "category": "Komputery",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£182.90",
    "accent": "violet",
    "spec": "16 GB RAM • czterordzeniowy CPU • PCIe • 2× MIPI",
    "url": "https://thepihut.com/search?q=Raspberry%20Pi%205%2016GB",
    "youtubePl": "Głównym komputerem robota Kora jest Raspberry Pi 5 z 16 GB pamięci RAM.",
    "youtubeEn": "Kora's main computer is a Raspberry Pi 5 with 16 GB of RAM."
  },
  {
    "id": "ai-hat-2",
    "name": "Raspberry Pi AI HAT+ 2",
    "short": "Lokalne przetwarzanie modeli",
    "category": "Komputery",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£130.20*",
    "accent": "violet",
    "spec": "Akcelerator do obliczeń lokalnych • zestaw z taśmą MIPI 300 mm",
    "url": "https://thepihut.com/search?q=Raspberry%20Pi%20AI%20HAT%2B%202",
    "youtubePl": "Do lokalnego przetwarzania wykorzystuję Raspberry Pi AI HAT+ 2.",
    "youtubeEn": "I use the Raspberry Pi AI HAT+ 2 for local model processing."
  },
  {
    "id": "xvf3800",
    "name": "ReSpeaker XVF3800 4-Mic Array",
    "short": "Cztery mikrofony i kierunkowe audio",
    "category": "Audio",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£51.50",
    "accent": "pink",
    "spec": "4 mikrofony • XMOS XVF3800 • redukcja echa i szumu",
    "url": "https://thepihut.com/search?q=ReSpeaker%20XMOS%20XVF3800",
    "youtubePl": "Kora słucha przez czteromikrofonowy moduł ReSpeaker XVF3800 z redukcją echa i szumu.",
    "youtubeEn": "Kora listens through a four-microphone ReSpeaker XVF3800 array with echo and noise reduction."
  },
  {
    "id": "cam3",
    "name": "Raspberry Pi Camera Module 3",
    "short": "Kamera dzienna z autofokusem",
    "category": "Sensory",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£27.50",
    "accent": "cyan",
    "spec": "12 MP • autofocus • standardowe pole widzenia",
    "url": "https://thepihut.com/search?q=Raspberry%20Pi%20Camera%20Module%203%20Standard",
    "youtubePl": "Obraz dzienny zapewnia Raspberry Pi Camera Module 3 z autofokusem.",
    "youtubeEn": "Daytime vision is provided by a Raspberry Pi Camera Module 3 with autofocus."
  },
  {
    "id": "x1203",
    "name": "SupTronics X1203 UPS",
    "short": "Awaryjne zasilanie Raspberry Pi 5",
    "category": "Zasilanie",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "Mam",
    "price": "£39.70",
    "accent": "amber",
    "spec": "5.1 V / 5 A • ogniwa 18650 poza płytką • montaż na pogo pinach",
    "url": "https://thepihut.com/search?q=SupTronics%20X1203%20UPS%20Raspberry%20Pi%205",
    "youtubePl": "Raspberry Pi 5 jest podtrzymywane przez UPS SupTronics X1203 o wydajności 5,1 V / 5 A.",
    "youtubeEn": "The Raspberry Pi 5 is backed by a SupTronics X1203 UPS rated at 5.1 V / 5 A."
  },
  {
    "id": "psu45",
    "name": "Raspberry Pi 45 W USB-C PSU",
    "short": "Oficjalny zasilacz UK",
    "category": "Zasilanie",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£17.90",
    "accent": "amber",
    "spec": "45 W • USB-C PD • wtyczka UK",
    "url": "https://thepihut.com/search?q=Raspberry%20Pi%2045W%20USB-C%20Power%20Supply%20UK",
    "youtubePl": "Do zasilania i testów Raspberry Pi używam oficjalnego zasilacza USB-C 45 W.",
    "youtubeEn": "I use the official 45 W USB-C power supply for Raspberry Pi power and bench testing."
  },
  {
    "id": "mpr121",
    "name": "Adafruit MPR121 — 12-Key Touch",
    "short": "Sterowanie dotykowe bez przycisków",
    "category": "Sensory",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£11.20",
    "accent": "cyan",
    "spec": "12 wejść pojemnościowych • I²C • STEMMA QT",
    "url": "https://thepihut.com/search?q=Adafruit%20MPR121%2012-Key%20Capacitive%20Touch",
    "youtubePl": "Dotyk wykrywa 12-kanałowy czujnik pojemnościowy Adafruit MPR121.",
    "youtubeEn": "Touch input is handled by a 12-channel Adafruit MPR121 capacitive sensor."
  },
  {
    "id": "portsplus",
    "name": "RasPiO Portsplus GPIO",
    "short": "Czytelna mapa pinów Raspberry Pi",
    "category": "Połączenia",
    "supplier": "Pi Hut",
    "project": "Warsztat",
    "status": "W drodze",
    "accent": "slate",
    "spec": "Nakładka opisowa GPIO • szybsze prototypowanie • mniej pomyłek",
    "url": "https://thepihut.com/search?q=RasPiO%20Portsplus%20GPIO%20Board",
    "youtubePl": "Do szybkiego prototypowania używam płytki RasPiO Portsplus z czytelnym opisem GPIO.",
    "youtubeEn": "For quick prototyping I use a RasPiO Portsplus board with clearly labelled GPIO pins."
  },
  {
    "id": "speaker-usb",
    "name": "Mini External USB Stereo Speaker",
    "short": "Kompaktowy głośnik do prototypów",
    "category": "Audio",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "price": "£14.40",
    "accent": "pink",
    "spec": "USB • stereo • mała obudowa",
    "url": "https://thepihut.com/search?q=Mini%20External%20USB%20Stereo%20Speaker",
    "youtubePl": "Dźwięk podczas testów zapewnia kompaktowy zewnętrzny głośnik stereo USB.",
    "youtubeEn": "A compact external USB stereo speaker provides audio during testing."
  },
  {
    "id": "camera-cable-300",
    "name": "Taśma kamery do Raspberry Pi 5 — 300 mm",
    "short": "Dłuższe połączenie kamery z komputerem",
    "category": "Połączenia",
    "supplier": "Pi Hut",
    "project": "Kora / VEGA",
    "status": "W drodze",
    "accent": "slate",
    "spec": "300 mm • złącze MIPI do Raspberry Pi 5 • elastyczna taśma",
    "url": "https://thepihut.com/search?q=Camera%20Adapter%20Cable%20Raspberry%20Pi%205%20300mm",
    "youtubePl": "Kamera jest połączona z Raspberry Pi 5 elastyczną taśmą MIPI o długości 300 mm.",
    "youtubeEn": "The camera is connected to the Raspberry Pi 5 with a 300 mm flexible MIPI cable."
  },
  {
    "id": "usb-c-molex",
    "name": "Przewód USB-C → Molex 4-pin",
    "short": "Zasilanie urządzeń z gniazda Molex",
    "category": "Połączenia",
    "supplier": "Pi Hut",
    "project": "Warsztat",
    "status": "W drodze",
    "accent": "slate",
    "spec": "USB-C • gniazdo Molex 4-pin • przewód zasilający",
    "url": "https://thepihut.com/search?q=USB-C%20to%204-Pin%20Molex%20Socket%20Cable",
    "youtubePl": "Do nietypowych połączeń zasilania używam przewodu USB-C z czteropinowym gniazdem Molex.",
    "youtubeEn": "For custom power connections I use a USB-C cable with a four-pin Molex socket."
  },
  {
    "id": "poe-header",
    "name": "PoE+ HAT Extra Tall Header Kit",
    "short": "Wyższe złącze GPIO pod nakładki",
    "category": "Połączenia",
    "supplier": "Pi Hut",
    "project": "Warsztat",
    "status": "W drodze",
    "accent": "slate",
    "spec": "Podwyższone GPIO • montaż wciskany • do PoE+ HAT",
    "url": "https://thepihut.com/search?q=PoE%2B%20HAT%20Push-Fit%20Extra%20Tall%20Header%20Kit",
    "youtubePl": "Wyższe złącze GPIO pozwala zachować dostęp do pinów podczas używania nakładki PoE+ HAT.",
    "youtubeEn": "An extra-tall GPIO header keeps the pins accessible while using a PoE+ HAT."
  },
  {
    "id": "pca9685",
    "name": "PCA9685 — 16 kanałów PWM",
    "short": "Sterownik serwomechanizmów",
    "category": "Sterowanie",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "price": "£4.99 / 2 szt.",
    "accent": "green",
    "spec": "16 kanałów • 12-bit PWM • magistrala I²C",
    "url": "https://www.aliexpress.com/item/1005010288617803.html",
    "youtubePl": "Ruchem serwomechanizmów sterują 16-kanałowe moduły PCA9685 przez magistralę I²C.",
    "youtubeEn": "The servos are controlled by 16-channel PCA9685 modules over I²C."
  },
  {
    "id": "esp32c6",
    "name": "ESP32-C6 SuperMini",
    "short": "Mały kontroler Wi-Fi i Bluetooth",
    "category": "Sterowanie",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "price": "£2.95 / szt.",
    "accent": "green",
    "spec": "Wi-Fi 6 • Bluetooth 5 • RISC-V • kompaktowy format",
    "url": "https://www.aliexpress.com/item/1005010177876942.html",
    "youtubePl": "Sterowanie niskopoziomowe realizuje kompaktowa płytka ESP32-C6 SuperMini.",
    "youtubeEn": "Low-level control is handled by a compact ESP32-C6 SuperMini board."
  },
  {
    "id": "esp32s3",
    "name": "ESP32-S3 N16R8",
    "short": "Kontroler z 16 MB Flash i 8 MB PSRAM",
    "category": "Sterowanie",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Mam",
    "price": "£3.99 / szt.",
    "accent": "green",
    "spec": "16 MB Flash • 8 MB PSRAM • Wi-Fi • Bluetooth",
    "url": "https://www.aliexpress.com/item/1005006418608267.html",
    "youtubePl": "W prototypach używam ESP32-S3 N16R8 z 16 MB Flash i 8 MB PSRAM.",
    "youtubeEn": "My prototypes use ESP32-S3 N16R8 boards with 16 MB Flash and 8 MB PSRAM."
  },
  {
    "id": "rplidar-c1",
    "name": "SLAMTEC RPLIDAR C1",
    "short": "Skanowanie otoczenia w 360°",
    "category": "Sensory",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "accent": "cyan",
    "spec": "360° • pomiar ToF • zasięg do 12 m • mapowanie i nawigacja",
    "url": "https://www.aliexpress.com/item/1005006190309082.html",
    "youtubePl": "Otoczenie w 360 stopniach skanuje lidar SLAMTEC RPLIDAR C1 o zasięgu do 12 metrów.",
    "youtubeEn": "A SLAMTEC RPLIDAR C1 scans the surroundings in 360 degrees at distances up to 12 metres."
  },
  {
    "id": "vl53l1x",
    "name": "VL53L1X Time-of-Flight",
    "short": "Laserowy pomiar odległości",
    "category": "Sensory",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "accent": "cyan",
    "spec": "ToF • I²C • zasięg do 4 m",
    "url": "https://www.aliexpress.com/item/1005006459241792.html",
    "youtubePl": "Odległość z przodu mierzy laserowy czujnik Time-of-Flight VL53L1X o zasięgu do 4 metrów.",
    "youtubeEn": "Front distance is measured by a VL53L1X Time-of-Flight sensor with a range of up to 4 metres."
  },
  {
    "id": "bmi160",
    "name": "BMI160 — IMU 6DoF",
    "short": "Akcelerometr i żyroskop",
    "category": "Sensory",
    "supplier": "AliExpress",
    "project": "Roboty mobilne",
    "status": "Mam",
    "price": "£1.00 / szt.",
    "accent": "cyan",
    "spec": "6 osi • akcelerometr • żyroskop • I²C / SPI",
    "url": "https://www.aliexpress.com/item/1005006351402967.html",
    "youtubePl": "Ruch i przechylenie mierzy sześcioosiowy moduł BMI160 z akcelerometrem i żyroskopem.",
    "youtubeEn": "Motion and tilt are measured by a six-axis BMI160 module with an accelerometer and gyroscope."
  },
  {
    "id": "apds9960",
    "name": "APDS-9960",
    "short": "Gesty, zbliżenie, kolor i światło",
    "category": "Sensory",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Mam",
    "price": "£2.00 / szt.",
    "accent": "cyan",
    "spec": "Gesty • zbliżenie • RGB • natężenie światła • I²C",
    "url": "https://www.aliexpress.com/item/1005008637735476.html",
    "youtubePl": "Gesty, zbliżenie i kolor światła wykrywa wielofunkcyjny czujnik APDS-9960.",
    "youtubeEn": "Gestures, proximity and light colour are detected by an APDS-9960 multifunction sensor."
  },
  {
    "id": "ld2410c",
    "name": "LD2410C — radar 24 GHz",
    "short": "Wykrywanie obecności człowieka",
    "category": "Sensory",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Mam",
    "accent": "cyan",
    "spec": "FMCW 24 GHz • wykrywa ruch i mikroruchy • UART / Bluetooth",
    "url": "https://www.aliexpress.com/item/1005007035360905.html",
    "youtubePl": "Obecność człowieka wykrywa radar FMCW LD2410C pracujący w paśmie 24 GHz.",
    "youtubeEn": "Human presence is detected by an LD2410C FMCW radar operating at 24 GHz."
  },
  {
    "id": "bts7960",
    "name": "BTS7960 — mostek H 43 A",
    "short": "Sterownik silników dużej mocy",
    "category": "Napędy",
    "supplier": "AliExpress",
    "project": "Roboty mobilne",
    "status": "Mam",
    "price": "£8.19 / 2 szt.",
    "accent": "red",
    "spec": "Mostek H • do 43 A deklarowane przez sprzedawcę • PWM",
    "url": "https://www.aliexpress.com/item/1005006264172513.html",
    "youtubePl": "Silnikami dużej mocy sterują moduły mostka H BTS7960.",
    "youtubeEn": "High-power motors are controlled by BTS7960 H-bridge modules."
  },
  {
    "id": "rds5180",
    "name": "RDS5180 — serwo 80 kg",
    "short": "Mocne serwo dwuosiowe",
    "category": "Napędy",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Mam",
    "accent": "red",
    "spec": "7.4 V • metalowe przekładnie • podwójny wał",
    "url": "https://www.aliexpress.com/item/1005010064568049.html",
    "youtubePl": "Do ruchów wymagających dużego momentu wykorzystuję dwuosiowe serwo RDS5180 z metalową przekładnią.",
    "youtubeEn": "For high-torque movement I use an RDS5180 dual-shaft servo with metal gears."
  },
  {
    "id": "rds3115",
    "name": "RDS3115MG — serwo 17 kg",
    "short": "Dwuosiowe serwo do ramion",
    "category": "Napędy",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "accent": "red",
    "spec": "180° / 270° • metalowe przekładnie • podwójny wał",
    "url": "https://www.aliexpress.com/item/1005005471524950.html",
    "youtubePl": "Ramiona robota napędzają dwuosiowe serwa RDS3115MG z metalowymi przekładniami.",
    "youtubeEn": "The robot arms are driven by RDS3115MG dual-shaft servos with metal gears."
  },
  {
    "id": "sg90",
    "name": "SG90 — mikroserwo 9 g",
    "short": "Małe i lekkie serwo 180°",
    "category": "Napędy",
    "supplier": "AliExpress",
    "project": "Uniwersalne",
    "status": "Mam",
    "price": "£7.99 / 10 szt.",
    "accent": "red",
    "spec": "9 g • 180° • PWM • lekkie mechanizmy",
    "url": "https://www.aliexpress.com/item/1005007173214082.html",
    "youtubePl": "Lekkie mechanizmy poruszają popularne mikroserwa SG90 o masie 9 gramów.",
    "youtubeEn": "Lightweight mechanisms are moved by popular 9-gram SG90 micro servos."
  },
  {
    "id": "dc5v15a",
    "name": "Przetwornica DC-DC 5 V / 15 A",
    "short": "Zasilanie elektroniki z 12–24 V",
    "category": "Zasilanie",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Mam",
    "price": "£5.69 / szt.",
    "accent": "amber",
    "spec": "Wejście 12–24 V • wyjście 5 V • deklarowane 15 A",
    "url": "https://www.aliexpress.com/item/1005007062967909.html",
    "youtubePl": "Napięcie instalacji 12–24 V obniża przetwornica DC-DC do 5 V dla elektroniki sterującej.",
    "youtubeEn": "A DC-DC converter steps the 12–24 V supply down to 5 V for the control electronics."
  },
  {
    "id": "dcwide",
    "name": "Przetwornica DC-DC 12–90 V → 5 V",
    "short": "Szeroki zakres napięcia wejściowego",
    "category": "Zasilanie",
    "supplier": "AliExpress",
    "project": "Roboty mobilne",
    "status": "Mam",
    "price": "£5.79",
    "accent": "amber",
    "spec": "Wejście 12–90 V • wyjście 5 V • deklarowane 10 A",
    "url": "https://www.aliexpress.com/item/1005008118059523.html",
    "youtubePl": "Elektronikę zasilam przez szerokozakresową przetwornicę DC-DC 12–90 V na 5 V.",
    "youtubeEn": "The electronics are powered through a wide-input 12–90 V to 5 V DC-DC converter."
  },
  {
    "id": "ws2812matrix",
    "name": "Matryca WS2812B 16×16",
    "short": "256 adresowalnych diod RGB",
    "category": "Światło",
    "supplier": "AliExpress",
    "project": "Kora / VEGA",
    "status": "Używam",
    "price": "£5.40 / szt.",
    "accent": "blue",
    "spec": "256 pikseli • RGB • 5 V • indywidualne sterowanie",
    "url": "https://www.aliexpress.com/item/1005008565110209.html",
    "youtubePl": "Animacje twarzy wyświetla matryca 16×16 z 256 indywidualnie sterowanymi diodami WS2812B.",
    "youtubeEn": "Face animations are displayed on a 16×16 matrix with 256 individually addressable WS2812B LEDs."
  },
  {
    "id": "pcm5102",
    "name": "PCM5102A — przetwornik I²S DAC",
    "short": "Cyfrowe audio dla Raspberry Pi i ESP32",
    "category": "Audio",
    "supplier": "AliExpress",
    "project": "Audio",
    "status": "Używam",
    "price": "£1.60",
    "accent": "pink",
    "spec": "I²S • wyjście stereo analog • 32-bit DAC",
    "url": "https://www.aliexpress.com/item/1005006953485069.html",
    "youtubePl": "Cyfrowy dźwięk I²S zamienia na sygnał analogowy moduł DAC PCM5102A.",
    "youtubeEn": "A PCM5102A DAC converts the digital I²S audio stream into an analogue stereo signal."
  },
  {
    "id": "inmp441",
    "name": "INMP441 — mikrofon I²S",
    "short": "Cyfrowy mikrofon MEMS",
    "category": "Audio",
    "supplier": "AliExpress",
    "project": "Audio",
    "status": "Mam",
    "price": "£2.09 / szt.",
    "accent": "pink",
    "spec": "MEMS • I²S • do ESP32 • niski pobór mocy",
    "url": "https://www.aliexpress.com/item/1005008869908588.html",
    "youtubePl": "Dźwięk rejestruje cyfrowy mikrofon MEMS INMP441 po magistrali I²S.",
    "youtubeEn": "Audio is captured by an INMP441 digital MEMS microphone over I²S."
  },
  {
    "id": "ili9341",
    "name": "Wyświetlacz TFT 2.8″ ILI9341",
    "short": "Kolorowy ekran dotykowy SPI",
    "category": "Światło",
    "supplier": "AliExpress",
    "project": "Uniwersalne",
    "status": "Mam",
    "price": "£4.69 / szt.",
    "accent": "blue",
    "spec": "240×320 px • SPI • panel dotykowy",
    "url": "https://www.aliexpress.com/item/1005006425985706.html",
    "youtubePl": "Interfejs wyświetlam na kolorowym ekranie dotykowym TFT 2,8 cala z kontrolerem ILI9341.",
    "youtubeEn": "The interface is shown on a 2.8-inch colour TFT touchscreen driven by the ILI9341 controller."
  },
  {
    "id": "dupont",
    "name": "Przewody Dupont 20 cm",
    "short": "Połączenia prototypowe żeński–żeński",
    "category": "Połączenia",
    "supplier": "AliExpress",
    "project": "Warsztat",
    "status": "Mam",
    "price": "£1.01 / komplet",
    "accent": "slate",
    "spec": "40 żył • 20 cm • żeński–żeński",
    "url": "https://www.aliexpress.com/item/1005005501503609.html",
    "youtubePl": "Połączenia prototypowe wykonuję przewodami Dupont 20 cm żeński–żeński.",
    "youtubeEn": "Prototype connections are made with 20 cm female-to-female Dupont jumper wires."
  },
  {
    "id": "xh254",
    "name": "Złącza JST-XH 2.54 mm",
    "short": "Przewody i gniazda czteropinowe",
    "category": "Połączenia",
    "supplier": "AliExpress",
    "project": "Warsztat",
    "status": "Mam",
    "price": "£1.14 / 5 kpl.",
    "accent": "slate",
    "spec": "Raster 2.54 mm • 4 piny • przewody 10 cm",
    "url": "https://www.aliexpress.com/item/1005007460897865.html",
    "youtubePl": "Moduły łączę rozłącznymi złączami JST-XH 2,54 mm.",
    "youtubeEn": "Modules are connected with detachable 2.54 mm JST-XH connectors."
  },
  {
    "id": "cnc-hrc68",
    "name": "Frez węglikowy HRC68 — 4 ostrza",
    "short": "Frez do precyzyjnej obróbki metalu",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Mam",
    "price": "£3.02",
    "accent": "green",
    "spec": "1,5 mm • chwyt 4 mm • długość 50 mm • 4 ostrza",
    "url": "https://www.aliexpress.com/item/1005004697788231.html",
    "youtubePl": "Do precyzyjnej obróbki używam czteroostrzowego frezu węglikowego HRC68 o średnicy 1,5 mm.",
    "youtubeEn": "For precision machining I use a 1.5 mm HRC68 four-flute carbide end mill."
  },
  {
    "id": "cnc-grbl2",
    "name": "Sterownik GRBL 32-bit — 2 osie",
    "short": "Płyta sterująca grawerką laserową",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Mam",
    "price": "£14.80",
    "accent": "green",
    "spec": "32-bit MCU • USB-C • 2 osie • sterowniki zintegrowane",
    "url": "https://www.aliexpress.com/item/1005004183745208.html",
    "youtubePl": "Dwoma osiami grawerki steruje 32-bitowa płyta GRBL z portem USB-C.",
    "youtubeEn": "A 32-bit GRBL board with USB-C controls the engraver's two axes."
  },
  {
    "id": "cnc-rotary",
    "name": "Moduł obrotowy TwoTrees",
    "short": "Grawerowanie przedmiotów cylindrycznych",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Mam",
    "price": "£30.95",
    "accent": "red",
    "spec": "Napęd rolkowy • oś obrotowa • puszki i elementy cylindryczne",
    "url": "https://www.aliexpress.com/item/1005002810671779.html",
    "youtubePl": "Moduł obrotowy TwoTrees pozwala grawerować puszki i inne cylindryczne przedmioty.",
    "youtubeEn": "The TwoTrees rotary module enables engraving on cans and other cylindrical objects."
  },
  {
    "id": "cnc-z-axis",
    "name": "Moduł osi Z 2040V",
    "short": "Regulacja wysokości głowicy laserowej",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Mam",
    "price": "£15.47",
    "accent": "red",
    "spec": "Profil 2040V • regulacja osi Z • uchwyt głowicy",
    "url": "https://www.aliexpress.com/item/4000863582142.html",
    "youtubePl": "Moduł osi Z umożliwia precyzyjne ustawienie wysokości głowicy względem materiału.",
    "youtubeEn": "The Z-axis module provides precise adjustment of the head height above the workpiece."
  },
  {
    "id": "cnc-psu24",
    "name": "Zasilacz na szynę DIN — 24 V / 2,5 A",
    "short": "Zasilanie sterowania i napędów",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Mam",
    "price": "£6.14",
    "accent": "amber",
    "spec": "60 W • 24 V DC • 2,5 A • montaż na szynie DIN",
    "url": "https://www.aliexpress.com/item/1005004654602581.html",
    "youtubePl": "Sterowanie maszyny zasila kompaktowy zasilacz 24 V / 2,5 A montowany na szynie DIN.",
    "youtubeEn": "The machine control system is powered by a compact 24 V / 2.5 A DIN-rail power supply."
  },
  {
    "id": "cnc-endmill-2f",
    "name": "Frez DREANIQUE 2 mm — 2 ostrza",
    "short": "Frez prosty do drewna, MDF i tworzyw",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Używam",
    "price": "£1.65 / szt.",
    "accent": "green",
    "spec": "2 mm • długość robocza 17 / 22 mm • węglik spiekany",
    "url": "https://www.aliexpress.com/item/1005001580397962.html",
    "youtubePl": "Do wycinania i grawerowania używam dwuostrzowych frezów DREANIQUE o średnicy 2 mm.",
    "youtubeEn": "For cutting and engraving I use 2 mm DREANIQUE two-flute carbide end mills."
  },
  {
    "id": "cnc-dust-shoe",
    "name": "Szczotka odciągu wrzeciona — 65 mm",
    "short": "Zbieranie pyłu bezpośrednio przy frezie",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Używam",
    "price": "£12.78",
    "accent": "slate",
    "spec": "Mocowanie 65 mm • szczotka ochronna • króciec odciągu",
    "url": "https://www.aliexpress.com/item/1005004235237465.html",
    "youtubePl": "Szczotka odciągu o średnicy 65 mm zbiera pył bezpośrednio wokół wrzeciona.",
    "youtubeEn": "A 65 mm spindle dust shoe collects debris directly around the cutter."
  },
  {
    "id": "cnc-controller4",
    "name": "Sterownik CNC GRBL — 4 osie",
    "short": "Główna płyta sterowania XYZA",
    "category": "CNC",
    "supplier": "AliExpress",
    "project": "Frezarka CNC",
    "status": "Używam",
    "price": "£51.76",
    "accent": "violet",
    "spec": "32-bit MCU • osie X/Y/Z/A • DRV8825 • wrzeciono do 500 W",
    "url": "https://www.aliexpress.com/item/1005003918795013.html",
    "youtubePl": "Ruchem osi X, Y, Z i A steruje 32-bitowa płyta GRBL ze sterownikami DRV8825.",
    "youtubeEn": "A 32-bit GRBL board with DRV8825 drivers controls the X, Y, Z and A axes."
  }
];
const state = {
  query: "",
  category: new URLSearchParams(location.search).get("category") || "Wszystkie",
  supplier: new URLSearchParams(location.search).get("supplier") || "Wszystkie sklepy",
  project: new URLSearchParams(location.search).get("project") || "Wszystkie projekty"
};

const $ = (selector) => document.querySelector(selector);
const categories = ["Wszystkie", ...new Set(parts.map((part) => part.category))];
const projects = ["Wszystkie projekty", ...new Set(parts.map((part) => part.project))];
const suppliers = ["Wszystkie sklepy", ...new Set(parts.map((part) => part.supplier))];

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"})[character]);
}

function copyText(part, mode) {
  if (mode === "short") return part.name + " — " + part.spec;
  return part.youtubePl + "\n" + part.youtubeEn + "\n\nElement / Component: " + part.name + "\nSklep / Store: " + part.supplier + "\n" + part.url;
}

async function copyToClipboard(text) {
  if (navigator.clipboard?.writeText) return navigator.clipboard.writeText(text);
  const field = document.createElement("textarea");
  field.value = text;
  field.setAttribute("readonly", "");
  field.style.position = "fixed";
  field.style.opacity = "0";
  document.body.appendChild(field);
  field.select();
  document.execCommand("copy");
  field.remove();
}

function filteredParts() {
  const query = state.query.trim().toLocaleLowerCase("pl");
  return parts.filter((part) => {
    const text = [part.name, part.short, part.spec, part.project, part.supplier].join(" ").toLocaleLowerCase("pl");
    return (!query || text.includes(query)) &&
      (state.category === "Wszystkie" || part.category === state.category) &&
      (state.supplier === "Wszystkie sklepy" || part.supplier === state.supplier) &&
      (state.project === "Wszystkie projekty" || part.project === state.project);
  });
}

function renderCategories() {
  $("#categoryTabs").innerHTML = categories.map((category) =>
    '<button type="button" class="' + (state.category === category ? "active" : "") + '" data-category="' + escapeHtml(category) + '">' + escapeHtml(category) + "</button>"
  ).join("");
}

function renderCards() {
  const filtered = filteredParts();
  $("#resultCount").textContent = String(filtered.length).padStart(2, "0") + " WYNIKÓW";
  $("#emptyState").hidden = filtered.length !== 0;
  $("#partsGrid").innerHTML = filtered.map((part, index) =>     '<article class="part-card">' +
      '<div class="part-visual ' + escapeHtml(part.accent) + '">' +
        '<img src="' + escapeHtml(window.PART_IMAGES[part.id]) + '" alt="' + escapeHtml(part.name) + '" loading="lazy">' +
        '<span class="card-number">' + String(index + 1).padStart(2, "0") + '</span>' +
        '<span class="status" data-status="' + escapeHtml(part.status) + '">' + escapeHtml(part.status) + '</span>' +
      '</div>' +
      '<div class="part-body">' +
        '<div class="meta-row"><span>' + escapeHtml(part.category) + '</span><span>' + escapeHtml(part.supplier) + '</span></div>' +
        '<h3>' + escapeHtml(part.name) + '</h3><p class="part-short">' + escapeHtml(part.short) + '</p><p class="spec">' + escapeHtml(part.spec) + '</p>' +
        '<div class="project-row"><span>Projekt</span><strong>' + escapeHtml(part.project) + '</strong></div>' +
        '<div class="price-row"><span>Cena zakupu</span><strong>' + escapeHtml(part.price || "—") + '</strong></div>' +
        '<div class="card-actions"><button type="button" class="copy-primary" data-copy="full" data-id="' + escapeHtml(part.id) + '">Opis na YouTube</button>' +
        '<button type="button" data-copy="short" data-id="' + escapeHtml(part.id) + '" aria-label="Kopiuj krótki opis ' + escapeHtml(part.name) + '" title="Kopiuj krótki opis">⧉</button>' +
        '<a href="' + escapeHtml(part.url) + '" target="_blank" rel="noreferrer" aria-label="Otwórz produkt ' + escapeHtml(part.name) + '" title="Otwórz produkt">↗</a></div>' +
      '</div>' +
    '</article>'
  ).join("");
}

function render() {
  renderCategories();
  renderCards();
}

function fillSelect(selector, values, selected) {
  const select = $(selector);
  select.innerHTML = values.map((value) => '<option value="' + escapeHtml(value) + '">' + escapeHtml(value) + '</option>').join("");
  select.value = values.includes(selected) ? selected : values[0];
}

const activeCount = parts.filter((part) => part.status === "Używam").length;
const incomingCount = parts.filter((part) => part.status === "W drodze").length;
$("#stats").innerHTML = '<div><strong>' + parts.length + '</strong><span>elementów</span></div><div><strong>' + activeCount + '</strong><span>w użyciu</span></div><div><strong>' + new Set(parts.map((part) => part.supplier)).size + '</strong><span>sklepy</span></div><div><strong>' + incomingCount + '</strong><span>w drodze</span></div>';

fillSelect("#projectFilter", projects, state.project);
fillSelect("#supplierFilter", suppliers, state.supplier);
state.project = $("#projectFilter").value;
state.supplier = $("#supplierFilter").value;
render();

$("#search").addEventListener("input", (event) => { state.query = event.target.value; $("#clearSearch").hidden = !state.query; renderCards(); });
$("#clearSearch").addEventListener("click", () => { state.query = ""; $("#search").value = ""; $("#clearSearch").hidden = true; renderCards(); });
$("#projectFilter").addEventListener("change", (event) => { state.project = event.target.value; renderCards(); });
$("#supplierFilter").addEventListener("change", (event) => { state.supplier = event.target.value; renderCards(); });
$("#categoryTabs").addEventListener("click", (event) => { const button = event.target.closest("[data-category]"); if (!button) return; state.category = button.dataset.category; render(); });
$("#partsGrid").addEventListener("click", async (event) => {
  const button = event.target.closest("[data-copy]");
  if (!button) return;
  const part = parts.find((item) => item.id === button.dataset.id);
  if (!part) return;
  await copyToClipboard(copyText(part, button.dataset.copy));
  const previous = button.textContent;
  button.textContent = button.dataset.copy === "full" ? "Skopiowano ✓" : "✓";
  setTimeout(() => { button.textContent = previous; }, 1600);
});
