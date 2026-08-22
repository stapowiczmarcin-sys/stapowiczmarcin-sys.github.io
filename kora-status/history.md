# Kora — durable project history

> **Żelazna zasada / Iron rule**  
> Każda istotna zmiana, test, awaria, rollback, nowy moduł, nowa część i decyzja projektowa trafia na WWW. Nie polegamy na pamięci czatu.  
> Every significant change, test, failure, rollback, new module, new part and project decision goes to the web. Chat memory is not the source of truth.

## Mandatory WWW mirrors

Po każdej istotnej pracy aktualizujemy opis i prawdziwy status — bez publikowania kodu — w następujących miejscach:

- Public Source of Truth: https://stapowiczmarcin-sys.github.io/kora-status/
- Kora Master Panel: https://vega-pi.tailfd0c05.ts.net/
- Kora Live: https://vega-pi.tailfd0c05.ts.net/live
- Kora Live JSON: https://vega-pi.tailfd0c05.ts.net/api/live.json

Master Panel ma już trwałe dane projektu w `state.json`, `modules.json` i `changelog.jsonl`. Live/Live JSON są generowane cyklicznie z `live_state.json`; dlatego trwałe dodanie historii prac do Live wymaga bezpiecznej, read-only integracji dokumentacji z generatorem Live, a nie ręcznego dopisywania do pliku, które zostałoby nadpisane.

## Status policy

- **WORKING** — only after a successful real functional or voice test.
- **CODE_OK** — compiles/static checks pass; real test still pending.
- **TESTING** — installed/prepared and under real-world testing.
- **FAILED** — real test failed or caused a regression.
- **ROLLED_BACK** — failed change reverted to the previous known state.

---

## 2026-08-22 — Kora/Alexa social cover corrected

**Status: ASSET_READY**

PL: Usunięto z okładki zdania zdradzające odpowiedź Kory. Zastąpił je teaser „Kora ma swoje zdanie / Poczekaj na jej odpowiedź”, dzięki czemu puenta pozostaje w filmie. Gotowa wersja Instagram Reels/Stories ma dokładnie 1080 × 1920 px, proporcje 9:16 i przestrzeń barwną sRGB. Lekki WebP służy do podglądu w Kora Brain, a pełny PNG jest dostępny do pobrania.

EN: The lines revealing Kora's reply were removed from the cover. They were replaced with the teaser “Kora has an opinion / Wait for her answer”, keeping the punchline inside the video. The Instagram Reels/Stories version is exactly 1080 × 1920 px, 9:16 and sRGB. A lightweight WebP is used for the Kora Brain preview and the full PNG remains available for download.

## 2026-08-22 — Portfolio hardware status corrected

**Status: CORRECTED**

The public portfolio and parts catalogue were synchronized with the verified hardware state. Raspberry Pi AI HAT+ 2 with Hailo-10H and ReSpeaker XVF3800 are no longer described as in transit. Hailo and ReSpeaker are in use, X1203 is present and under testing, and Adafruit MPR121 is owned but remains awaiting confirmed integration. RPLIDAR C1 remains separate test hardware and does not replace the A2 used by the current Kora.

## 2026-08-21 — First metal-leg movement

**Status: VERIFIED / NOT WALKING**

The battery pack was assembled, the BMS was soldered and installed, and the 32-channel servo controller was connected to the metal-leg system. The first visible movement/twitch was confirmed. This validates the power-and-control chain, but it is not a walking result. Controlled gait tests are the next stage.

## 2026-08-20 — Voice path partially restored

**Status: TESTING**

ReSpeaker XVF3800 recording was confirmed and the wake word **Kora** triggered an acknowledgement. The complete spoken-question-to-answer cycle still needs a confirmed PL/EN end-to-end test.

## 2026-08-16 — Documentation mirror rule expanded

**Status: RULE ACTIVE / LIVE INTEGRATION PENDING**

The permanent project-memory rule now explicitly includes the Pi-side Master Panel, Kora Live and Live JSON endpoints in addition to the public Source of Truth. Every significant change must be summarized on those WWW surfaces without publishing source code, secrets or private credentials. The Master Panel is already data-driven. Live/Live JSON still need a safe read-only documentation field after the current audio/wake rollback is confirmed.

## 2026-08-16 — Local dialogue regression

**Status: FAILED / ROLLBACK REQUIRED**

A new local-dialogue layer was being introduced so Raspberry Pi 5 could answer simple PL/EN dialogue locally instead of sending everything to the model/router. After the latest deployment, the LED ring turned red and Kora stopped hearing/listening correctly.

Decision:

1. Stop adding new features.
2. Roll back the latest local-dialogue change.
3. Confirm `vega.service` is active.
4. Confirm wake/listening works with a single spoken **Kora**.
5. Confirm the ring returns to its normal listening state.
6. Only after the base audio/wake path is restored, resume work on local dialogue and English personality routing.

Relevant paths:

- `/home/marcin/vega_robot/vega_genius.py`
- `/home/marcin/vega_robot/kora_spontaneous_chat.py`
- `/home/marcin/vega_robot/kora_brain_router.py`
- target local-dialogue module: `/home/marcin/vega_robot/kora_system/kora_local_dialogue.py`

## 2026-08-16 — English secretary-like responses

**Status: OPEN**

Observed English conversation routing can produce source `laptop_ollama_en_lang_guard` and generic replies instead of Kora's intended personality. One repeated bad response was the generic request for a more concrete angle. This must not be marked WORKING until a real voice test shows varied, relevant answers.

## 2026-08-16 — English time routing

**Status: OPEN**

Questions such as `What time is it?` should be handled locally and must not fall through into generic spontaneous/model conversation.

## 2026-08-16 — Wake false positive

**Status: OPEN / RETEST AFTER ROLLBACK**

The short wake word `Kora` can be recognized well, but a false wake was observed when saying `Marzena`. Wake false-positive handling must be retested after the rollback and restoration of stable listening.

---

## Documentation safety

Publish technical project history, code status, paths, versions, test results, hardware and architectural decisions. Do **not** publish API keys, passwords, authentication tokens, private credentials or private personal data.
