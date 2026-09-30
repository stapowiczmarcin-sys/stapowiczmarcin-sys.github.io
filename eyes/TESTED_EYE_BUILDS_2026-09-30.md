# Kora Eyes — hardware-tested builds / wersje sprawdzone sprzętowo

**Tested on Kora: 30 Sep 2026 / Sprawdzone na Korze: 30 września 2026.**

The four public download packages are the exact firmware builds confirmed working on the real robot. The source code itself is kept unchanged from the tested files.

Cztery publiczne pakiety do pobrania zawierają dokładnie te wersje firmware, które zostały potwierdzone jako działające na prawdziwej Korze. Samego kodu nie poprawiamy po teście sprzętowym.

| Build | Auto sleep | Light GPIO mapping | SHA-256 of tested source |
|---|---:|---|---|
| Human V18 | 5 s | LEFT GPIO3 / RIGHT GPIO4 | `cf620a04d0c655c9a1d82bb14203252fcefd16a2fec45b9f592d710ece6af030` |
| Metal V18 | 5 s | LEFT GPIO4 / RIGHT GPIO3 | `fadeefb0f9acaedcb9a58d17b743165260b37021eaf3d9487a094c01ae051853` |
| Monster V18 | 5 s | LEFT GPIO4 / RIGHT GPIO3 | `c469400b230b5cb2bab4897ae1bd711c498ec3ec750dcf95a2a1bb68721f4c88` |
| Animal V14 | 5 s | LEFT GPIO3 / RIGHT GPIO4 | `109b7e376f34329aa5831ba43a451932e6f49efee88af0572480ee39e2c58133` |

## Important / Ważne

- Ambient-light inputs are GPIO3 and GPIO4, but **left/right assignment is not identical in all four sketches**. Follow `PIN_LIGHT_L` and `PIN_LIGHT_R` in the selected sketch.
- Wejścia czujników światła to GPIO3 i GPIO4, ale **przypisanie lewy/prawy nie jest identyczne we wszystkich czterech szkicach**. Kieruj się `PIN_LIGHT_L` i `PIN_LIGHT_R` w wybranym kodzie.
- AP/OTA credentials in these public builds are placeholders. Set your own values before enabling Wi-Fi/OTA.
- Hasła AP/OTA w publicznych wersjach są placeholderami. Ustaw własne przed włączeniem Wi-Fi/OTA.
