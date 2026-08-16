# Kora — durable project history

> **Żelazna zasada / Iron rule**  
> Każda istotna zmiana, test, awaria, rollback, nowy moduł, nowa część i decyzja projektowa trafia na WWW. Nie polegamy na pamięci czatu.  
> Every significant change, test, failure, rollback, new module, new part and project decision goes to the web. Chat memory is not the source of truth.

## Status policy

- **WORKING** — only after a successful real functional or voice test.
- **CODE_OK** — compiles/static checks pass; real test still pending.
- **TESTING** — installed/prepared and under real-world testing.
- **FAILED** — real test failed or caused a regression.
- **ROLLED_BACK** — failed change reverted to the previous known state.

---

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
