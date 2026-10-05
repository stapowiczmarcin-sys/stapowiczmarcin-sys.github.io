# KORA Battery Meter v0.1 — EN
ESP32-C3 Super Mini + ILI9341 240×320 + INA226. Hardware validation pending; this is a monitor, not an automatic battery tester.

## Wiring
TFT: CS GPIO10, RST GPIO5, DC GPIO4, MOSI/SDA GPIO7, SCK/SCL GPIO6, MISO GPIO2. Preserve the display's already working power/backlight wiring; do not guess the voltage of a different module.
NEW I2C wiring: INA226 SDA GPIO0, SCL GPIO1, VCC 3.3 V, GND common. GPIO0/1 are no longer available for the weather station encoder. Optional BACK button: GPIO8 to GND. Never hold BACK during boot (strapping pin).
INA226 address 0x40, manufacturer ID 0x5449. Confirm module jumpers and terminal labels.
Battery + → suitable fuse → INA226 IN+ → INA226 IN− → external load +. Battery − → load − and common GND. VBUS, when separately exposed, connects to IN−. Check module schematic: some boards already connect these.
Power ESP32 via USB independently; do not connect the cell directly to an ESP32 GPIO/3.3 V rail. Module shunt, terminals and wiring set the allowable current. This version is for a small bench setup, not Kora's servo power rail.

## Arduino IDE
Select ESP32C3 Dev Module, enable USB CDC On Boot if needed for your board. Install Adafruit GFX Library and Adafruit ILI9341 (and dependencies). Open KORA_Battery_Meter/KORA_Battery_Meter.ino.
Set SHUNT_OHMS to the actual shunt resistance: R100 = 0.1 Ω; R010 = 0.01 Ω. Default 0 intentionally disables current/energy readings. Do not assume your module value. INA226 shunt full scale is about ±81.92 mV; keep well below this and the module's rated power/current.
Upload and compare voltage/current with a trusted multimeter at two or more loads. No accuracy claim until checked. Graph shows the latest 100 samples (~20 s), autoscaled with minimum 50 mV span.
Connect phone to KORA-Meter, password KoraMeter1; open http://192.168.4.1. Phone may report no Internet. Read live V/A/W, separate incoming/outgoing mAh/Wh; export a snapshot CSV. Hold BACK 1.5 s, send RESET at 115200 baud, or use web reset. Totals are RAM-only and reset on power loss.

## Meaning of measurements
Positive = discharge, negative = charge. OUT and IN are integrated session totals, not full battery capacity or percentage. To determine usable capacity, start with a correctly fully charged known chemistry, use a rated external electronic load with independent low-voltage cutoff, and measure a complete discharge at a specified current to a specified endpoint. Set the endpoint from the cell manufacturer's data; this sketch does NOT disconnect the load. Do not use a bare resistor as an unattended lithium-cell tester. No runtime estimate without an independently established remaining capacity.
Disconnected sensor displays an error and stops integration; saturation is rejected. Reconnect/setup recovery may require reboot. Sensor/transport errors and changing sign reduce integration precision. Initial or missing intervals are not counted.

# KORA Battery Meter v0.1 — PL
Prototyp miernika na ESP32-C3, TFT ILI9341 i INA226. Nie był jeszcze skompilowany w Arduino ani sprawdzony na sprzęcie. To miernik, nie automatyczny tester z odcięciem.

TFT: CS 10, RST 5, DC 4, MOSI 7, SCK 6, MISO 2 — zachowane piny pogodynki. Zasilanie/podświetlenie pozostaw jak w działającym układzie. NOWE przewody INA226: SDA GPIO0, SCL GPIO1, VCC 3.3 V, GND wspólna. Enkoder pogodynki odłącz od GPIO0/1. BACK: GPIO8 do GND; nie przytrzymuj podczas uruchamiania.
Ogniwo plus → odpowiedni bezpiecznik → IN+ → IN− → plus obciążenia; minus ogniwa → minus obciążenia i wspólna masa. Osobny VBUS → IN−, jeśli moduł nie ma już tego połączenia. ESP zasilaj osobno przez USB. Nie podłączaj ogniwa do GPIO ani szyny 3.3 V. Ten projekt nie jest do toru prądowego serw Kory.
W Arduino zainstaluj Adafruit GFX i Adafruit ILI9341, wybierz ESP32C3 Dev Module. Wpisz rzeczywistą wartość SHUNT_OHMS: R100 = 0.1 Ω, R010 = 0.01 Ω. Domyślne 0 blokuje pomiar prądu. Sprawdź pomiary multimetrem i dopuszczalny prąd modułu.
Wi-Fi KORA-Meter / KoraMeter1, panel http://192.168.4.1. V, A, W oraz osobno oddane/pobrane mAh i Wh. CSV to bieżące podsumowanie, nie historia. Reset: BACK 1.5 s, RESET przez Serial Monitor 115200 lub przycisk w panelu. Restart kasuje sumy.
OUT to energia oddana od resetu, nie automatycznie pojemność ogniwa. Test pojemności wymaga pełnego ładowania, właściwego obciążenia i niezależnego odcięcia przy napięciu zgodnym z dokumentacją ogniwa. Kod nie steruje obciążeniem ani odcięciem. Wykres ~20 s, skala automatyczna. Nie zgadujemy procentu naładowania.

## Short / pomysł zapisany 2026-10-05
EN: Phone: “Confirm you're not a robot.” Kora taps. Pause. “Apparently, all you have to do is click.”
PL: Telefon: „Potwierdź, że nie jesteś robotem”. Kora wciska. Pauza. „Najwyraźniej wystarczy kliknąć”.
Use the local parody screen at ../kora-not-a-robot/; no real CAPTCHA. Tap by human or compatible capacitive stylus: a plastic robot leg may not register. Robot motion/control code remains unchanged.
