# KORA MASTER CONTROL — snapshot 2026-09-16

## Scope

This snapshot documents the Servo 2040 migration and the new Kora desktop control/calibration layer.

## VERIFIED

- Raspberry Pi detects the Servo 2040 RP2040 as a MicroPython board.
- MicroPython reported version 1.29.0 on Raspberry Pi Pico / RP2040.
- The custom Servo 2040 bridge started and printed `KORA_SERVO2040_BRIDGE_READY`.
- Real USB communication test succeeded: `PING` -> `PONG`.

## CODE READY / awaiting hardware verification

- Desktop GUI: `kora_remote_2040.py`.
- Direction controls: forward, backward, left, right, rotate left, rotate right, base position, servo off.
- Gait controls: gait 1 / gait 2, speed, step length, turn strength.
- Six-leg calibration window using X/Y/Z values.
- Each X/Y/Z correction changes by one unit and recalculates calibration/IK.
- Save operation writes `point.txt` and creates a timestamped backup first.
- Motion path: `control.py` -> `servo.py` -> USB bridge -> Servo 2040.

## NOT YET MARKED WORKING

Forward/backward/sideways/rotation through the new Servo 2040 output path must not be marked WORKING until Kora completes a real movement test on hardware.

## Leg map — Servo 2040

| Kora leg | Coxa | Femur | Tibia | Foot switch |
|---|---:|---:|---:|---|
| Right rear | 1 | 2 | 3 | SENSOR 1 |
| Right middle | 4 | 5 | 6 | SENSOR 2 |
| Right front | 7 | 8 | 9 | SENSOR 3 |
| Left front | 10 | 11 | 12 | SENSOR 4 |
| Left middle | 13 | 14 | 15 | SENSOR 5 |
| Left rear | 16 | 17 | 18 | SENSOR 6 |

## Core local files

```text
kora_remote_2040.py
control.py
servo.py
point.txt
kora_servo2040_bridge.py
```

## Backup policy

Create a timestamped local snapshot containing the five core files above plus a README with the verified state at the time of backup.

## Public documentation

Kora Master Control page:

`https://stapowiczmarcin-sys.github.io/kora-master/`
