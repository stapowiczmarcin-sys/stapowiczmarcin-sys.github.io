# Kora Eye Film Show — Raspberry Pi director

A small desktop control panel for filming and demonstrating Kora's digital eyes from a Raspberry Pi over USB serial.

**Supported eye families:**
- Human V18
- Metal V18
- Monster V18
- Animal V14

The panel is intentionally separate from Kora's main robot control. It does **not** control the mechanical head, legs or Servo2040.

## What it can do

- ready-made **SHORT** and **LONG** filming sequences for each eye style,
- `WAKE` / `SLEEP`,
- manual blink with `M`,
- `LOOK X Y` gaze control, where X and Y are in the `-1.0 … 1.0` range,
- manual iris controls for the V18 Human / Metal / Monster builds,
- a 3×3 manual gaze pad,
- serial log from the ESP32-S3,
- a dedicated **LIGHT DEMO** for the real light sensors,
- countdown cues before a take.

## Real light demo

The current round-eye builds use the physical light inputs:

- **GPIO3 — left light sensor**
- **GPIO4 — right light sensor**

The film panel does **not** fake the effect by sending a brightness value. During LIGHT DEMO you physically shine a light on the sensors or cover them. The eye firmware itself changes pupil size and, in strong light, progressively squints the eyelids.

## Safety by design

The panel does not transmit anything when it starts.

1. You select a serial device.
2. You explicitly press **CONNECT**.
3. Only then can an eye command be sent.

Likely Servo2040 / MicroPython / Pimoroni device names are filtered from the eye-port list as an additional guard. Always verify the actual USB device after reconnecting hardware because `/dev/ttyUSB*` numbers can change.

## Raspberry Pi requirements

- Python 3
- Tkinter
- `pyserial`
- USB connection from the Pi to the ESP32-S3 running one of the supported eye firmwares
- serial speed: **115200 baud**

On Raspberry Pi OS, if `pyserial` is missing:

```bash
sudo apt install -y python3-serial
```

Copy the panel to your project folder, for example:

```bash
cd /home/marcin/vega_robot
python3 kora_eye_film_show.py
```

The optional Kora configuration path `/home/marcin/vega_robot/config/ports.json` is used only when present. On another robot or Pi it can be absent; the panel will still scan normal Linux serial paths.

## Suggested filming workflow

1. Flash one eye firmware to the ESP32-S3.
2. Connect the ESP32-S3 USB cable to the Raspberry Pi.
3. Open **Kora Eye Film Show**.
4. Choose the correct eye serial device and press **CONNECT**.
5. Record `HUMAN SHORT`, `METAL SHORT`, `MONSTER SHORT` or `ANIMAL SHORT` for vertical video.
6. Record the matching **LONG** sequence for a full YouTube video.
7. Record **LIGHT DEMO** close-up to show that the pupils and eyelids react to real ambient light.
8. Stop the panel before reconnecting or changing USB devices.

## Serial commands used by the panel

```text
WAKE
SLEEP
M
LOOK -1.0..1.0 -1.0..1.0
IRIS <name>   # V18 Human / Metal / Monster only
```

The exact iris names supported by a firmware build should be checked in that build's own guide/source. Animal V14 is not given iris-change commands by the automatic filming sequences.

---

# Kora Eye Film Show — reżyser na Raspberry Pi

Mały panel okienkowy do nagrywania i prezentowania cyfrowych oczu Kory. Raspberry Pi steruje ESP32-S3 przez USB/serial.

**Obsługiwane wersje:**
- Human V18
- Metal V18
- Monster V18
- Animal V14

Panel jest celowo oddzielony od głównego sterowania robotem. **Nie steruje mechaniczną głową, nogami ani Servo2040.**

## Co potrafi

- gotowe sekwencje **SHORT** i **LONG** dla każdego rodzaju oczu,
- `WAKE` / `SLEEP`,
- ręczne mrugnięcie `M`,
- kierunek spojrzenia `LOOK X Y`, gdzie X i Y mają zakres `-1.0 … 1.0`,
- ręczne sterowanie tęczówką w Human / Metal / Monster V18,
- ręczny pad spojrzenia 3×3,
- podgląd logu odpowiedzi ESP32-S3,
- osobny **LIGHT DEMO** dla prawdziwych czujników światła,
- odliczanie przed ujęciem.

## Prawdziwy test światła

Aktualne wersje okrągłych oczu używają:

- **GPIO3 — lewy czujnik światła**
- **GPIO4 — prawy czujnik światła**

Panel nie udaje zmiany światła przez wysyłanie wartości programowej. W LIGHT DEMO naprawdę świecisz latarką na czujniki albo je zasłaniasz. Firmware oczu sam zmienia rozmiar źrenic, a przy mocnym świetle dodatkowo stopniowo mruży powieki.

## Bezpieczeństwo

Po uruchomieniu panel **niczego nie wysyła**.

1. Wybierasz urządzenie szeregowe.
2. Sam naciskasz **CONNECT**.
3. Dopiero wtedy można wysłać komendę do oczu.

Nazwy wyglądające na Servo2040 / MicroPython / Pimoroni są dodatkowo filtrowane z listy kandydatów. Po każdym przepięciu USB i tak trzeba sprawdzić prawdziwy port, bo numery `/dev/ttyUSB*` mogą się zmienić.

## Wymagania na Raspberry Pi

- Python 3
- Tkinter
- `pyserial`
- połączenie USB Pi ↔ ESP32-S3 z firmware oczu
- transmisja **115200 baud**

Jeżeli brakuje `pyserial`:

```bash
sudo apt install -y python3-serial
```

Przykładowe uruchomienie:

```bash
cd /home/marcin/vega_robot
python3 kora_eye_film_show.py
```

Opcjonalna ścieżka Kory `/home/marcin/vega_robot/config/ports.json` jest używana tylko wtedy, gdy istnieje. Na innym Raspberry Pi panel nadal może wyszukać standardowe porty szeregowe Linuksa.

## Jak nagrywać

1. Wgraj jeden firmware oczu na ESP32-S3.
2. Podepnij USB ESP32-S3 do Raspberry Pi.
3. Uruchom **Kora Eye Film Show**.
4. Wybierz właściwy port oczu i naciśnij **CONNECT**.
5. Do pionowego filmu nagraj odpowiedni `HUMAN / METAL / MONSTER / ANIMAL SHORT`.
6. Do pełnego filmu nagraj odpowiadającą mu sekwencję **LONG**.
7. Osobno nagraj **LIGHT DEMO**, najlepiej blisko oczu, żeby było widać prawdziwą reakcję źrenic i powiek.
8. Przed przepinaniem USB zatrzymaj sekwencję i rozłącz panel.

## Komendy używane przez panel

```text
WAKE
SLEEP
M
LOOK -1.0..1.0 -1.0..1.0
IRIS <nazwa>   # tylko Human / Metal / Monster V18
```

Dokładne nazwy tęczówek dostępne w danej wersji należy sprawdzić w instrukcji lub źródle konkretnego firmware. Automatyczne sekwencje Animal V14 nie wysyłają komend zmiany tęczówki.
