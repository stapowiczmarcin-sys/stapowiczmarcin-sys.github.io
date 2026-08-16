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
