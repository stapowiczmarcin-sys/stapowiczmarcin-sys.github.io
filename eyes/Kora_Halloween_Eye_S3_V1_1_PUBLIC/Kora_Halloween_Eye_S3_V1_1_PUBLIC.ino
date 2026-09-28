// ============================================================
// KORA HALLOWEEN EYE S3 V1.1
// ESP32-S3 Super Mini + TFT 2.8" ILI9341 240x320 + PIR motion sensor
//
// ONE EYE ONLY.
// - PIR GPIO12 detects movement
// - movement -> display wakes, backlight fades in, eye opens
// - PIR rising edge -> one wake event (held HIGH does NOT extend awake time)
// - nervous fast gaze, squints, blinks while awake
// - 3-4 s after the last NEW PIR edge -> eye closes, backlight fades out,
//   ILI9341 gets DISPOFF + SLPIN
//
// TFT wiring (confirmed layout):
// SCK  = GPIO6
// MISO = GPIO2   (not required by TFT; reserved for touch)
// MOSI = GPIO7
// CS   = GPIO10
// DC   = GPIO3
// RST  = GPIO4
// BL   = GPIO5
// TOUCH CS = GPIO8 (kept HIGH, touch not used)
// PIR OUT  = GPIO12
//
// Board: ESP32S3 Dev Module
// Arduino-ESP32 core 3.x
// Libraries: Adafruit GFX, Adafruit ILI9341
// ============================================================

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <esp_system.h>
#include <math.h>

// ------------------------------------------------------------
// PINS
// ------------------------------------------------------------
static constexpr int PIN_SCK     = 6;
static constexpr int PIN_MISO    = 2;
static constexpr int PIN_MOSI    = 7;
static constexpr int PIN_TFT_CS  = 10;
static constexpr int PIN_TFT_DC  = 3;
static constexpr int PIN_TFT_RST = 4;
static constexpr int PIN_TFT_BL  = 5;
static constexpr int PIN_TS_CS   = 8;
static constexpr int PIN_PIR     = 12;

// ------------------------------------------------------------
// DISPLAY
// ------------------------------------------------------------
Adafruit_ILI9341 tft(&SPI, PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_RST);

static constexpr int SCREEN_W = 240;
static constexpr int SCREEN_H = 320;

// Canvas only for the eye area, not full 240x320.
// 240 x 220 x 2 = 105600 bytes.
static constexpr int CANVAS_W = 240;
static constexpr int CANVAS_H = 220;
static constexpr int CANVAS_Y = 50;

GFXcanvas16* eyeCanvas = nullptr;

// ------------------------------------------------------------
// TIMING
// ------------------------------------------------------------
static constexpr uint16_t FRAME_MS = 45;       // ~22 FPS target
static constexpr uint16_t WAKE_MS  = 650;
static constexpr uint16_t CLOSE_MS = 650;

static constexpr uint32_t SLEEP_MIN_MS = 3000;
static constexpr uint32_t SLEEP_MAX_MS = 4000;

// ------------------------------------------------------------
// EYE POWER STATE
// ------------------------------------------------------------
enum EyeState : uint8_t {
  EYE_SLEEPING = 0,
  EYE_WAKING,
  EYE_AWAKE,
  EYE_CLOSING
};

EyeState eyeState = EYE_SLEEPING;

uint32_t stateStartedMs = 0;
uint32_t sleepDeadlineMs = 0;
uint32_t lastFrameMs = 0;

float powerOpen = 0.0f;
uint8_t backlightNow = 0;

// PIR edge detector. Many 3-pin PIR modules keep OUT=HIGH for seconds.
// We therefore react to a NEW LOW->HIGH edge only; a held HIGH cannot
// postpone sleep forever.
bool pirPrevHigh = false;
bool pirArmed = false;

// ------------------------------------------------------------
// NERVOUS GAZE
// Inspired by the lookX/lookY + micro-saccade model from KoraEyesCyber.
// ------------------------------------------------------------
float lookX = 0.0f;
float lookY = 0.0f;
float targetX = 0.0f;
float targetY = 0.0f;

float microX = 0.0f;
float microY = 0.0f;

uint32_t nextLookMs = 0;
uint32_t nextMicroMs = 0;
uint32_t microEndMs = 0;

// ------------------------------------------------------------
// BLINK / SQUINT
// ------------------------------------------------------------
bool blinkActive = false;
uint32_t blinkStartedMs = 0;
uint16_t blinkDurationMs = 170;
uint32_t nextBlinkMs = 0;

bool squintActive = false;
uint32_t squintStartedMs = 0;
uint16_t squintDurationMs = 260;
float squintDepth = 0.55f;
uint32_t nextSquintMs = 0;

// ------------------------------------------------------------
// HELPERS
// ------------------------------------------------------------
static float clampF(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

static int clampI(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

static float smooth01(float x) {
  x = clampF(x, 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static uint16_t blend565(uint16_t a, uint16_t b, float t) {
  t = clampF(t, 0.0f, 1.0f);

  int ar = (a >> 11) & 0x1F;
  int ag = (a >> 5) & 0x3F;
  int ab = a & 0x1F;

  int br = (b >> 11) & 0x1F;
  int bg = (b >> 5) & 0x3F;
  int bb = b & 0x1F;

  int r = ar + (int)((br - ar) * t);
  int g = ag + (int)((bg - ag) * t);
  int bl = ab + (int)((bb - ab) * t);

  return (uint16_t)((r << 11) | (g << 5) | bl);
}

static uint32_t hash32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

// ------------------------------------------------------------
// BACKLIGHT
// Arduino-ESP32 core 3.x LEDC API
// ------------------------------------------------------------
static void setBacklight(uint8_t value) {
  backlightNow = value;
  ledcWrite(PIN_TFT_BL, value);
}

// ------------------------------------------------------------
// HARD TFT RESET
// ------------------------------------------------------------
static void hardResetTFT() {
  digitalWrite(PIN_TFT_RST, HIGH);
  delay(10);
  digitalWrite(PIN_TFT_RST, LOW);
  delay(40);
  digitalWrite(PIN_TFT_RST, HIGH);
  delay(120);
}

// ------------------------------------------------------------
// LOW-LEVEL DISPLAY SLEEP / WAKE
// ------------------------------------------------------------
static void displayCommand(uint8_t cmd) {
  tft.startWrite();
  tft.writeCommand(cmd);
  tft.endWrite();
}

static void displayWakeHardware() {
  displayCommand(0x11); // SLPOUT
  delay(120);
  displayCommand(0x29); // DISPON
  delay(20);
}

static void displaySleepHardware() {
  displayCommand(0x28); // DISPOFF
  delay(20);
  displayCommand(0x10); // SLPIN
  delay(120);
}

// ------------------------------------------------------------
// CANVAS SHAPES
// ------------------------------------------------------------
static void fillEllipse(
  GFXcanvas16 &cv,
  int cx,
  int cy,
  int rx,
  int ry,
  uint16_t color
) {
  if (rx <= 0 || ry <= 0) return;

  for (int y = -ry; y <= ry; ++y) {
    float ny = (float)y / (float)ry;
    float q = 1.0f - ny * ny;
    if (q <= 0.0f) continue;

    int hx = (int)lroundf((float)rx * sqrtf(q));
    cv.drawFastHLine(cx - hx, cy + y, hx * 2 + 1, color);
  }
}

static void drawEllipseOutline(
  GFXcanvas16 &cv,
  int cx,
  int cy,
  int rx,
  int ry,
  uint16_t color
) {
  if (rx <= 1 || ry <= 1) return;

  for (int a = 0; a < 360; a += 2) {
    float r = (float)a * DEG_TO_RAD;
    int x = cx + (int)lroundf(cosf(r) * rx);
    int y = cy + (int)lroundf(sinf(r) * ry);
    cv.drawPixel(x, y, color);
  }
}

// ------------------------------------------------------------
// STATIC HALLOWEEN BACKGROUND
// ------------------------------------------------------------
static void drawStaticBackground() {
  tft.fillScreen(rgb565(2, 0, 4));

  // faint blood-red halo around the eye zone
  for (int r = 115; r >= 70; r -= 8) {
    float k = 1.0f - (float)(r - 70) / 45.0f;
    uint16_t c = rgb565((uint8_t)(16 + 24 * k), 0, (uint8_t)(4 + 5 * k));
    tft.drawCircle(120, 160, r, c);
  }
}

// ------------------------------------------------------------
// DETERMINISTIC VEINS
// ------------------------------------------------------------
static void drawVeins(
  GFXcanvas16 &cv,
  int cx,
  int cy,
  int rx,
  int ry,
  int irisX,
  int irisY
) {
  const uint16_t VEIN1 = rgb565(115, 14, 9);
  const uint16_t VEIN2 = rgb565(75, 7, 8);

  for (int i = 0; i < 13; ++i) {
    uint32_t h = hash32(0xA13F27u + (uint32_t)i * 977u);

    float ang = ((h & 0xFFFFu) / 65535.0f) * 2.0f * PI;
    float edgeScale = 0.82f + ((h >> 16) & 0xFFu) / 255.0f * 0.14f;

    int sx = cx + (int)(cosf(ang) * rx * edgeScale);
    int sy = cy + (int)(sinf(ang) * ry * edgeScale);

    float toward = 0.34f + ((h >> 24) & 0xFFu) / 255.0f * 0.24f;

    int ex = sx + (int)((irisX - sx) * toward);
    int ey = sy + (int)((irisY - sy) * toward);

    int bend = (int)((h >> 8) & 0x0F) - 7;
    int mx = (sx + ex) / 2 + (int)(-sinf(ang) * bend);
    int my = (sy + ey) / 2 + (int)( cosf(ang) * bend);

    cv.drawLine(sx, sy, mx, my, VEIN1);
    cv.drawLine(mx, my, ex, ey, VEIN2);

    if ((i % 3) == 0) {
      cv.drawLine(mx, my, mx + bend / 2, my - 5, VEIN2);
    }
  }
}

// ------------------------------------------------------------
// FIERY IRIS
// ------------------------------------------------------------
static void drawIris(
  GFXcanvas16 &cv,
  int cx,
  int cy,
  int radius,
  float pupilPulse,
  uint32_t now
) {
  uint16_t OUTER = rgb565(58, 2, 0);
  uint16_t RED   = rgb565(175, 20, 2);
  uint16_t ORG   = rgb565(255, 88, 4);
  uint16_t GOLD  = rgb565(255, 182, 18);
  uint16_t PALE  = rgb565(255, 226, 80);

  fillEllipse(cv, cx, cy, radius + 4, radius + 4, OUTER);
  fillEllipse(cv, cx, cy, radius, radius, RED);
  fillEllipse(cv, cx, cy, radius - 7, radius - 7, ORG);
  fillEllipse(cv, cx, cy, radius - 17, radius - 17, GOLD);

  float phase = now * 0.0033f;

  for (int a = 0; a < 360; a += 7) {
    float rad = a * DEG_TO_RAD;

    uint32_t h = hash32((uint32_t)a * 19937u + 0x77AA11u);
    float jitter = ((h & 255u) / 255.0f - 0.5f) * 7.0f;

    int r1 = radius - 4;
    int r2 = 10 + (int)jitter;

    int x1 = cx + (int)(cosf(rad) * r1);
    int y1 = cy + (int)(sinf(rad) * r1);

    float wobble = sinf(phase + a * 0.08f) * 2.0f;
    int x2 = cx + (int)(cosf(rad) * (r2 + wobble));
    int y2 = cy + (int)(sinf(rad) * (r2 + wobble));

    uint16_t c = (a % 21 == 0) ? PALE : OUTER;
    cv.drawLine(x1, y1, x2, y2, c);
  }

  int pupilRx = 7 + (int)(5.0f * pupilPulse);
  int pupilRy = 31 + (int)(4.0f * pupilPulse);

  fillEllipse(cv, cx, cy, pupilRx + 3, pupilRy + 3, rgb565(70, 0, 0));
  fillEllipse(cv, cx, cy, pupilRx, pupilRy, 0x0000);

  // sinister wet highlights
  cv.fillCircle(cx - 14, cy - 18, 5, rgb565(255, 244, 190));
  cv.fillCircle(cx - 11, cy - 16, 2, 0xFFFF);
  cv.fillCircle(cx + 13, cy + 15, 2, rgb565(255, 155, 45));
}

// ------------------------------------------------------------
// BLINK ENVELOPE
// ------------------------------------------------------------
static float blinkEnvelope(uint32_t now) {
  if (!blinkActive) return 1.0f;

  uint32_t age = now - blinkStartedMs;

  if (age >= blinkDurationMs) {
    blinkActive = false;
    return 1.0f;
  }

  float p = (float)age / (float)blinkDurationMs;

  if (p < 0.42f) {
    return 1.0f - smooth01(p / 0.42f) * 0.98f;
  }

  if (p < 0.58f) {
    return 0.02f;
  }

  return 0.02f + smooth01((p - 0.58f) / 0.42f) * 0.98f;
}

// ------------------------------------------------------------
// SQUINT ENVELOPE
// ------------------------------------------------------------
static float squintEnvelope(uint32_t now) {
  if (!squintActive) return 1.0f;

  uint32_t age = now - squintStartedMs;

  if (age >= squintDurationMs) {
    squintActive = false;
    return 1.0f;
  }

  float p = (float)age / (float)squintDurationMs;
  float e = sinf(p * PI);

  return 1.0f - e * squintDepth;
}

// ------------------------------------------------------------
// RENDER ONE HALLOWEEN EYE
// ------------------------------------------------------------
static void renderEye(uint32_t now) {
  if (!eyeCanvas || !eyeCanvas->getBuffer()) return;

  GFXcanvas16 &cv = *eyeCanvas;

  const uint16_t BG      = rgb565(2, 0, 3);
  const uint16_t LID_DK  = rgb565(24, 1, 4);
  const uint16_t LID_RED = rgb565(92, 7, 5);
  const uint16_t SCLERA  = rgb565(224, 158, 92);
  const uint16_t SCLERA2 = rgb565(255, 192, 110);

  cv.fillScreen(BG);

  const int cx = 120;
  const int cy = 110;

  float blink = blinkEnvelope(now);
  float squint = squintEnvelope(now);

  float liveOpen = powerOpen * blink * squint;
  liveOpen = clampF(liveOpen, 0.01f, 1.0f);

  // subtle breathing / evil pulse
  float breathe = 1.0f + 0.018f * sinf(now * 0.004f);

  int outerRx = (int)(112.0f * breathe);
  int outerRy = max(2, (int)(76.0f * liveOpen));

  int whiteRx = 104;
  int whiteRy = max(1, (int)(59.0f * liveOpen));

  // red halo/lid tissue
  fillEllipse(cv, cx, cy, outerRx, outerRy, LID_DK);
  fillEllipse(cv, cx, cy, outerRx - 4, max(1, outerRy - 4), LID_RED);

  // eyeball
  fillEllipse(cv, cx, cy, whiteRx, whiteRy, SCLERA);

  // warm center makes sclera less flat
  fillEllipse(cv, cx, cy + 2, whiteRx - 5, max(1, whiteRy - 5), SCLERA2);

  // Gaze. Clamp iris more aggressively when squinting.
  float gazeScaleX = 37.0f * liveOpen;
  float gazeScaleY = 20.0f * liveOpen;

  int irisX = cx + (int)((lookX + microX) * gazeScaleX);
  int irisY = cy + (int)((lookY + microY) * gazeScaleY);

  irisX = clampI(irisX, cx - 42, cx + 42);
  irisY = clampI(irisY, cy - 23, cy + 23);

  if (liveOpen > 0.10f) {
    drawVeins(cv, cx, cy, whiteRx, whiteRy, irisX, irisY);

    int irisRadius = max(11, (int)(44.0f * (0.72f + 0.28f * liveOpen)));
    float pupilPulse = 0.5f + 0.5f * sinf(now * 0.008f);

    drawIris(cv, irisX, irisY, irisRadius, pupilPulse, now);
  }

  // top/bottom darkness emphasizes eyelids during squint
  if (liveOpen < 0.72f) {
    int shade = (int)((0.72f - liveOpen) * 32.0f);
    if (shade > 0) {
      cv.fillTriangle(0, 0, CANVAS_W, 0, CANVAS_W, shade, BG);
      cv.fillTriangle(0, 0, 0, shade, CANVAS_W, shade, BG);

      cv.fillTriangle(
        0, CANVAS_H,
        CANVAS_W, CANVAS_H,
        CANVAS_W, CANVAS_H - shade,
        BG
      );
      cv.fillTriangle(
        0, CANVAS_H,
        0, CANVAS_H - shade,
        CANVAS_W, CANVAS_H - shade,
        BG
      );
    }
  }

  // upper lashes - only when reasonably open
  if (liveOpen > 0.28f) {
    for (int i = 0; i < 9; ++i) {
      int x = 37 + i * 21;
      int y = cy - outerRy + 7 + abs(i - 4) * 2;
      cv.drawLine(x, y, x - 5 + (i % 3) * 5, y - 13, rgb565(18, 0, 2));
    }
  }

  // lid outline
  drawEllipseOutline(cv, cx, cy, outerRx - 2, outerRy - 2, rgb565(138, 12, 5));

  // push canvas
  tft.drawRGBBitmap(
    0,
    CANVAS_Y,
    cv.getBuffer(),
    CANVAS_W,
    CANVAS_H
  );
}

// ------------------------------------------------------------
// NERVOUS MOTION
// ------------------------------------------------------------
static void scheduleNervousEvents(uint32_t now) {
  if (now >= nextLookMs) {
    // Mostly strong side-to-side jumps.
    int pick = random(0, 100);

    if (pick < 35) {
      targetX = random(-100, -55) / 100.0f;
      targetY = random(-35, 36) / 100.0f;
    } else if (pick < 70) {
      targetX = random(55, 101) / 100.0f;
      targetY = random(-35, 36) / 100.0f;
    } else {
      targetX = random(-90, 91) / 100.0f;
      targetY = random(-65, 66) / 100.0f;
    }

    nextLookMs = now + (uint32_t)random(90, 260);
  }

  if (now >= nextMicroMs) {
    microX = random(-90, 91) / 1000.0f;
    microY = random(-70, 71) / 1000.0f;

    microEndMs = now + (uint32_t)random(35, 90);
    nextMicroMs = now + (uint32_t)random(160, 460);
  }

  if (microEndMs && now >= microEndMs) {
    microX *= 0.55f;
    microY *= 0.55f;

    if (fabsf(microX) < 0.002f) microX = 0;
    if (fabsf(microY) < 0.002f) microY = 0;

    microEndMs = 0;
  }

  if (!blinkActive && now >= nextBlinkMs) {
    blinkActive = true;
    blinkStartedMs = now;
    blinkDurationMs = (uint16_t)random(115, 220);
    nextBlinkMs = now + (uint32_t)random(650, 1900);
  }

  if (!squintActive && now >= nextSquintMs) {
    squintActive = true;
    squintStartedMs = now;
    squintDurationMs = (uint16_t)random(190, 480);
    squintDepth = random(35, 72) / 100.0f;
    nextSquintMs = now + (uint32_t)random(450, 1400);
  }

  // Fast ease like an anxious eye, not a slow robot gaze.
  lookX += (targetX - lookX) * 0.42f;
  lookY += (targetY - lookY) * 0.42f;
}

// ------------------------------------------------------------
// POWER STATE TRANSITIONS
// ------------------------------------------------------------
static void setNewSleepDeadline(uint32_t now) {
  sleepDeadlineMs =
    now +
    (uint32_t)random((long)SLEEP_MIN_MS, (long)SLEEP_MAX_MS + 1L);
}

static void beginWake(uint32_t now) {
  if (eyeState == EYE_SLEEPING) {
    displayWakeHardware();
    drawStaticBackground();
    setBacklight(0);
    powerOpen = 0.02f;
  }

  eyeState = EYE_WAKING;

  // Continue smoothly if movement interrupts closing.
  uint32_t backdate = (uint32_t)(clampF(powerOpen, 0.0f, 1.0f) * WAKE_MS);
  stateStartedMs = now - backdate;

  setNewSleepDeadline(now);

  targetX = random(-60, 61) / 100.0f;
  targetY = random(-25, 26) / 100.0f;

  nextLookMs = now;
  nextBlinkMs = now + (uint32_t)random(350, 850);
  nextSquintMs = now + (uint32_t)random(250, 700);

  Serial.println("PIR -> WAKE");
}

static void beginClosing(uint32_t now) {
  eyeState = EYE_CLOSING;

  // If we were not fully open, preserve continuity.
  uint32_t backdate = (uint32_t)((1.0f - clampF(powerOpen, 0.0f, 1.0f)) * CLOSE_MS);
  stateStartedMs = now - backdate;

  targetX = 0.0f;
  targetY = 0.10f;

  Serial.println("NO MOTION -> CLOSE");
}

static void finishSleep() {
  powerOpen = 0.0f;

  // draw final closed eye before power-down
  renderEye(millis());

  setBacklight(0);
  delay(40);

  displaySleepHardware();

  eyeState = EYE_SLEEPING;

  Serial.println("DISPLAY SLEEP");
}

// ------------------------------------------------------------
// UPDATE STATE MACHINE
// ------------------------------------------------------------
static void updatePowerState(uint32_t now, bool motionEvent) {
  // IMPORTANT: motionEvent is a LOW->HIGH edge, NOT the raw PIR level.
  // A PIR that holds OUT high for 5, 10 or 30 seconds must not keep
  // pushing sleepDeadlineMs forward on every loop iteration.
  if (motionEvent) {
    setNewSleepDeadline(now);

    if (eyeState == EYE_SLEEPING || eyeState == EYE_CLOSING) {
      beginWake(now);
    }
  }

  switch (eyeState) {
    case EYE_SLEEPING:
      powerOpen = 0.0f;
      break;

    case EYE_WAKING: {
      float p = (float)(now - stateStartedMs) / (float)WAKE_MS;
      p = smooth01(p);

      powerOpen = p;
      setBacklight((uint8_t)lroundf(255.0f * p));

      if (p >= 0.999f) {
        powerOpen = 1.0f;
        setBacklight(255);
        eyeState = EYE_AWAKE;
        Serial.println("EYE AWAKE");
      }
      break;
    }

    case EYE_AWAKE:
      powerOpen = 1.0f;

      if ((int32_t)(now - sleepDeadlineMs) >= 0) {
        beginClosing(now);
      }
      break;

    case EYE_CLOSING: {
      if (motionEvent) {
        beginWake(now);
        break;
      }

      float p = (float)(now - stateStartedMs) / (float)CLOSE_MS;
      p = smooth01(p);

      powerOpen = 1.0f - p;

      // Keep bright during first part, then fade hard into darkness.
      float light = 1.0f;
      if (p > 0.42f) {
        light = 1.0f - (p - 0.42f) / 0.58f;
      }
      light = clampF(light, 0.0f, 1.0f);
      setBacklight((uint8_t)lroundf(255.0f * light));

      if (p >= 0.999f) {
        finishSleep();
      }
      break;
    }
  }
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(350);

  Serial.println();
  Serial.println("KORA HALLOWEEN EYE S3 V1.1");

  randomSeed(esp_random());

  pinMode(PIN_PIR, INPUT);

  // Do not treat a PIR power-up HIGH as real motion.
  // We arm the detector only after OUT has first been observed LOW.
  pirPrevHigh = (digitalRead(PIN_PIR) == HIGH);
  pirArmed = !pirPrevHigh;

  pinMode(PIN_TS_CS, OUTPUT);
  digitalWrite(PIN_TS_CS, HIGH); // touch disabled, never steals SPI

  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);

  pinMode(PIN_TFT_DC, OUTPUT);
  pinMode(PIN_TFT_RST, OUTPUT);

  // Backlight PWM
  if (!ledcAttach(PIN_TFT_BL, 5000, 8)) {
    Serial.println("WARN: LEDC attach failed");
  }
  setBacklight(0);

  hardResetTFT();

  // Same working SPI mapping as your S3 distance-screen build.
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, -1);

  // Conservative clock - known-safe on wired modules.
  tft.begin(12000000);
  tft.setRotation(0); // 240x320 portrait
  tft.setTextWrap(false);

  drawStaticBackground();

  eyeCanvas = new GFXcanvas16(CANVAS_W, CANVAS_H);

  if (!eyeCanvas || !eyeCanvas->getBuffer()) {
    Serial.println("FATAL: canvas allocation failed");

    // visible diagnostic if canvas allocation fails
    setBacklight(180);
    tft.fillScreen(ILI9341_RED);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 130);
    tft.print("CANVAS ERROR");

    while (true) {
      delay(1000);
    }
  }

  Serial.print("Canvas OK bytes=");
  Serial.println(CANVAS_W * CANVAS_H * 2);

  // Start truly asleep. PIR is the only wake source.
  renderEye(millis());
  setBacklight(0);
  displaySleepHardware();

  eyeState = EYE_SLEEPING;
  powerOpen = 0.0f;

  Serial.println("READY - PIR GPIO12 edge-triggered; held HIGH cannot block sleep");
}

// ------------------------------------------------------------
// LOOP
// ------------------------------------------------------------
void loop() {
  uint32_t now = millis();

  const bool pirHigh = (digitalRead(PIN_PIR) == HIGH);
  bool motionEvent = false;

  // PIR modules often hold HIGH for much longer than the desired
  // 3-4 second eye activity. Only a fresh LOW->HIGH edge is motion.
  if (!pirArmed) {
    if (!pirHigh) {
      pirArmed = true;
      pirPrevHigh = false;
      Serial.println("PIR ARMED (LOW seen)");
    }
  } else {
    if (pirHigh && !pirPrevHigh) {
      motionEvent = true;
      Serial.println("PIR EDGE HIGH");
    }
    pirPrevHigh = pirHigh;
  }

  updatePowerState(now, motionEvent);

  if (eyeState != EYE_SLEEPING) {
    scheduleNervousEvents(now);

    if (now - lastFrameMs >= FRAME_MS) {
      lastFrameMs = now;
      renderEye(now);
    }
  }

  delay(1);
}
