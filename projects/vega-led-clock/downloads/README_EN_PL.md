# VEGA LED Clock 2.0 — ESP8266

## English

The clock keeps the original 137-LED layout, twelve calibrated hour positions, UK GMT/BST time, OLED, SinricPro and both OTA update methods.

### What changed

- **Time only:** time markers on a black background. No gradient, background effect or minute comet. Selecting this mode switches the clock back on.
- **Time + ambience:** short, distinct time trails and dark separation around the hands. Animation history is stored separately, so moving time markers cannot leave unwanted trails in the effects.
- **Effects only:** no time overlay, even if an older saved setting has the clock enabled.
- Independent time and effect brightness. The comet is composited below the time markers.
- 29 working animation selections, including the new Aurora, Embers and Orbit scenes. Previously most effect names selected the same fallback animation.
- Responsive, self-contained EN/PL web panel, live LED frame preview, colour presets, four schedule editors and non-blocking position preview.
- Large HH:MM on the 128×64 OLED, with seconds, mode and IP address fitting within the display. The language selector also changes the OLED mode labels.
- Asynchronous SNTP time synchronisation avoids blocking LED frames while waiting for a time-server reply. Until the first successful synchronisation, time is shown as `--:--` and time markers remain off. After synchronisation the clock continues running through a Wi-Fi interruption.
- Existing EEPROM settings are retained. New settings use a separate block. Temporary scheduled dimming is not saved as the normal brightness; repeated setting changes are combined into a save after 0.9 seconds of inactivity.

### Upload

1. Extract the entire `VEGA_Clock_2_0` folder. Open **VEGA_Clock_2_0.ino** in Arduino IDE, keeping all `.h` files next to it.
2. This sketch targets **ESP8266**, as in the supplied VEGA code. Select your actual ESP8266 board. Verification used **NodeMCU 1.0 (ESP-12E Module)** with ESP8266 core **3.1.2**.
3. Install the libraries listed below using Library Manager. Use **SinricPro**, rather than the separate SinricPro_Generic library.
4. **Config.h contains placeholders.** Enter your own Wi-Fi name/password, SinricPro App Key, App Secret and Switch Device ID, and choose your own OTA password before uploading. Keep the completed personal file out of a public repository.
5. Select the USB serial port and upload. To preserve calibration, avoid the ESP8266 **Erase Flash → All Flash Contents** option; use **Only Sketch**.
6. Open **http://zegar.local/** on the same Wi-Fi network, or use the IP address shown on the OLED. If your browser does not resolve `.local`, use the IP address.

Library versions used for the successful build:

| Library | Version |
| --- | --- |
| SinricPro | 5.1.0 |
| WebSockets | 2.7.2 |
| ArduinoJson | 7.4.3 |
| Adafruit NeoPixel | 1.15.5 |
| Adafruit GFX Library | 1.12.6 |
| Adafruit SSD1306 | 2.5.17 |
| Adafruit BusIO | 1.17.4 |

NTPClient is no longer required; time synchronisation uses the ESP8266 core.

The original connections are unchanged: NeoPixel data **D2 / GPIO4**, OLED SDA **D5 / GPIO14**, OLED SCL **D6 / GPIO12**, OLED address **0x3C**. These pin aliases apply to NodeMCU and D1 mini board definitions.

### First use

Choose **Time only** to check the time markers without any animation. For a clearer colour separation, select **Amber / cyan / red**. Existing saved colours are retained until you change them. Then choose **Time + ambience**, enable **Protect time contrast** and try **Aurora**.

Schedules use UK local time and act at their configured minute. The panel shows when a schedule is dimming time or effects. Selecting a mode or turning the clock back on restores the configured brightness immediately; subsequent scheduled events still apply. Disable a schedule if you want continuous display. To calibrate a position, select the hour, preview the LED and save its position. The preview lasts 0.7 seconds, after which the normal clock display resumes.

The panel works without external fonts, CDNs or an internet connection once the device is reachable on your LAN. Internet is needed for initial time synchronisation and SinricPro service access. Web OTA remains at **/update**, with the existing login settings.

### Files and validation

- `VegaFirmware.h`: firmware logic, original hardware configuration and preserved EEPROM layout.
- `ClockRender.h`: independent visual-layer composition and time contrast protection.
- `VegaTime.h`: asynchronous core time synchronisation.
- `WebPanel.html`: editable web panel source.
- `WebUI.h`: the same panel stored in flash. Run `build_web_panel.py` after editing the HTML.
- `Config.h`: connection placeholders to complete before uploading.

Build passed for ESP8266 NodeMCU. Host tests exercised all 29 animations, calibrated mapping, time-only/effects-only modes, independent brightness, effect-history isolation, comet priority, EEPROM migration, schedules and UK daylight-saving boundaries. Browser tests checked both languages, mobile layout and controls against a simulated device API. Screenshots are software previews. The physical clock has not been flashed or measured in this session.

## Polski

Zegar zachowuje oryginalny układ 137 LED, dwanaście skalibrowanych pozycji godzin, czas UK GMT/BST, OLED, SinricPro i obie metody aktualizacji OTA.

### Co poprawiono

- **Tylko godzina:** znaczniki czasu na czarnym tle. Bez gradientu, animacji tła i komety co minutę. Wybranie tego trybu ponownie włącza zegar.
- **Godzina + efekty:** krótkie, wyraźne ślady wskazówek oraz ciemna przerwa wokół nich. Animacje mają osobną pamięć obrazu, więc przesuwające się wskazówki nie zostawiają w nich przypadkowych smug.
- **Tylko efekty:** godzina nie jest nakładana, nawet jeśli starsze zapisane ustawienia miały ją włączoną.
- Niezależna jasność godziny i efektów. Kometa jest rysowana pod znacznikami czasu.
- 29 działających animacji, w tym nowe: Zorza, Żar i Orbita. Wcześniej większość nazw efektów uruchamiała tę samą animację zastępczą.
- Nowy panel WWW EN/PL, dopasowanie do telefonu, podgląd ostatniej klatki LED, gotowe zestawy kolorów, cztery edytory harmonogramów i podgląd kalibracji bez zatrzymywania obsługi urządzenia.
- Duża godzina HH:MM na OLED 128×64. Sekundy, tryb i adres IP mieszczą się na ekranie. Wybór języka zmienia również opisy trybu na OLED.
- Synchronizacja czasu SNTP działa w tle i nie blokuje animacji podczas oczekiwania na odpowiedź serwera. Przed pierwszą synchronizacją wyświetla się `--:--`, a znaczniki czasu są wygaszone. Po synchronizacji zegar działa dalej również podczas przerwy w Wi-Fi.
- Zachowana zgodność z zapisanymi ustawieniami EEPROM. Nowe opcje mają osobny blok. Nocne przyciemnienie nie nadpisuje jasności bazowej; kolejne zmiany ustawień łączą się w zapis po 0,9 sekundy bez następnej zmiany.

### Wgranie

1. Rozpakuj cały folder `VEGA_Clock_2_0`. W Arduino IDE otwórz **VEGA_Clock_2_0.ino**. Wszystkie pliki `.h` muszą zostać obok szkicu.
2. Ten kod jest dla **ESP8266**, zgodnie z przesłanym szkicem VEGA. Wybierz model swojej płytki ESP8266. Kompilację sprawdziłem dla **NodeMCU 1.0 (ESP-12E Module)** z pakietem ESP8266 **3.1.2**.
3. Zainstaluj biblioteki z tabeli w części angielskiej przez menedżer bibliotek. Wybierz **SinricPro**, zamiast osobnej biblioteki SinricPro_Generic.
4. **Config.h zawiera pola do uzupełnienia.** Przed wgraniem wpisz swoją nazwę i hasło Wi-Fi, App Key, App Secret oraz Device ID przełącznika SinricPro, a także ustaw własne hasło OTA. Uzupełnionego osobistego pliku nie dodawaj do publicznego repozytorium.
5. Wybierz port USB i wgraj szkic. Aby zachować kalibrację, w ustawieniu ESP8266 **Erase Flash** wybierz **Only Sketch**, zamiast **All Flash Contents**.
6. Otwórz **http://zegar.local/** w tej samej sieci Wi-Fi albo wpisz adres IP widoczny na OLED. Jeżeli `.local` nie działa w przeglądarce, użyj adresu IP.

Biblioteka NTPClient nie jest już potrzebna; synchronizację obsługuje pakiet ESP8266.

Połączenia pozostają takie jak w oryginale: dane NeoPixel **D2 / GPIO4**, OLED SDA **D5 / GPIO14**, OLED SCL **D6 / GPIO12**, adres OLED **0x3C**. Takie nazwy pinów dotyczą definicji płytek NodeMCU i D1 mini.

### Pierwsze uruchomienie

Wybierz **Tylko godzina**, aby sprawdzić znaczniki bez animacji. Dla czytelnego rozdzielenia kolorów wybierz **Bursztyn / turkus / czerwień**. Wcześniej zapisane kolory pozostają do czasu ich zmiany. Następnie wybierz **Godzina + efekty**, włącz **Chroń kontrast godziny** i wypróbuj **Zorzę**.

Harmonogramy używają czasu UK i reagują o wskazanej minucie. Panel pokazuje, gdy harmonogram przyciemnia godzinę albo efekty. Wybranie trybu lub ponowne włączenie zegara natychmiast przywraca ustawioną jasność; kolejne zdarzenia harmonogramu nadal obowiązują. Wyłącz dany harmonogram, jeżeli chcesz stałego wyświetlania. Przy kalibracji wybierz godzinę, sprawdź numer LED i zapisz pozycję. Podgląd trwa 0,7 sekundy, po czym wraca zwykły ekran zegara.

Panel nie wymaga zewnętrznych czcionek, CDN ani internetu, gdy urządzenie jest dostępne w sieci lokalnej. Internet jest potrzebny do pierwszej synchronizacji czasu oraz usługi SinricPro. Aktualizacja przez WWW pozostaje pod adresem **/update**, z dotychczasowymi danymi logowania.

### Pliki i sprawdzenie

- `VegaFirmware.h`: działanie zegara, oryginalne połączenia i zachowany układ EEPROM.
- `ClockRender.h`: osobne warstwy obrazu i ochrona czytelności godziny.
- `VegaTime.h`: synchronizacja czasu w tle.
- `WebPanel.html`: źródło panelu WWW do edycji.
- `WebUI.h`: ten sam panel zapisany w pamięci flash. Po edycji HTML uruchom `build_web_panel.py`.
- `Config.h`: pola na Twoje dane połączenia do uzupełnienia przed wgraniem.

Kompilacja dla ESP8266 NodeMCU przeszła poprawnie. Testy na komputerze objęły wszystkie 29 animacji, kalibrację, tryby wyświetlania, niezależną jasność, brak smug wskazówek w efektach, pierwszeństwo godziny przed kometą, zapis ustawień, harmonogramy i granice zmiany czasu w UK. Panel sprawdziłem w obu językach oraz w widoku telefonu, korzystając z symulowanego urządzenia. Zrzuty przedstawiają podgląd programowy. W tej sesji nie wgrywałem programu do fizycznego zegara ani nie mierzyłem jego działania.

## Project page / Strona projektu

https://stapowiczmarcin-sys.github.io/projects/vega-led-clock/

Video / Film: **YOUTUBE_LINK_TO_ADD / LINK_YOUTUBE_DO_DODANIA**

Reference wiring / Połączenia referencyjne: `docs/wiring.svg`. Data pins are taken from the sketch. Power conditioning and a level shifter are recommended reference additions, not a record of an inspected physical build.
Piny danych pochodzą z kodu. Zasilanie, zabezpieczenia i konwerter poziomów są zaleceniami dla schematu referencyjnego; nie potwierdzają części zamontowanych w konkretnym egzemplarzu.

Sources / Źródła:
- ESP8266 board installation: https://arduino-esp8266.readthedocs.io/en/3.1.2/installing.html
- NeoPixel wiring recommendations: https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices
- SinricPro SDK: https://github.com/sinricpro/esp8266-esp32-sdk
- SinricPro account/device setup: https://help.sinric.pro/pages/tutorials/general/device-creation-wizard
