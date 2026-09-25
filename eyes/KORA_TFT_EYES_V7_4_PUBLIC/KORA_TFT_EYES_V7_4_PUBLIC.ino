#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <math.h>

// ============================================================
// KORA TFT EYES V7.4 NEON REAL MAX "CRT EDITION"
// ESP32-WROOM + GMT020-02-7P ver 1.3 (ST7789V 240x320)
//
// Everything from V7.3 (sleep-first, breathing, ear dances,
// blink patterns, OTA, watchdog, AP fallback, heartbeat,
// EYESCFG, color drift, anti-burn-in)
// PLUS:
//  - CRT wake: dot -> vertical line -> eyes unfold / budzenie CRT
//  - CRT sleep: eyes -> line -> dot -> black / zasypianie CRT
//  - sleep = anime arc lines, not slits / sen = lukowe kreski
//  - Arduino-ESP32 core 3.x watchdog API
// ============================================================

// ---------- TFT: VSPI ----------
static constexpr int PIN_TFT_SCLK = 18;
static constexpr int PIN_TFT_MOSI = 23;
static constexpr int PIN_TFT_CS   = 27;
static constexpr int PIN_TFT_DC   = 16;
static constexpr int PIN_TFT_RST  = 17;

// ---------- ear servos ----------
static constexpr int PIN_EAR_L = 25;
static constexpr int PIN_EAR_R = 26;

static constexpr int EAR_CENTER_L = 90;
static constexpr int EAR_CENTER_R = 90;
static constexpr int EAR_MIN_L = 55;
static constexpr int EAR_MAX_L = 125;
static constexpr int EAR_MIN_R = 55;
static constexpr int EAR_MAX_R = 125;
static constexpr bool EAR_L_REVERSED = false;
static constexpr bool EAR_R_REVERSED = true;

// ---------- Public release configuration ----------
// Shared free by Marcin Stapowicz / Kreatywny Morrcin.
// Guide: https://stapowiczmarcin-sys.github.io/eyes/
// Public copy: visual renderer and pin map preserved from V7.4.
// USB serial works without Wi-Fi. To use network features, set this to true
// and supply your OWN AP/OTA passwords below (12+ characters).
static constexpr bool ENABLE_WIFI = false;

// ---------- OTA / AP ----------
static const char* OTA_PASSWORD = "";
static const char* AP_SSID = "Kora-Eyes-Setup";
static const char* AP_PASS = "";

// ---------- display ----------
Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
static constexpr int SW = 320;
static constexpr int SH = 240;

// ---------- eye canvas geometry ----------
static int EYEW = 160;
static int EYEH = 240;
GFXcanvas16 *eyeCanvas = nullptr;

// ---------- network ----------
WebServer server(80);
Preferences prefs;
String wifiSsid;
String wifiPass;
bool wifiReady = false;
bool apMode = false;
uint32_t lastReconnectMs = 0;

// ---------- servos ----------
Servo earL;
Servo earR;
float earLNow = 0.0f;
float earRNow = 0.0f;
float earLTarget = 0.0f;
float earRTarget = 0.0f;

uint32_t earPerkMs = 0;
static constexpr uint16_t EAR_PERK_MS = 900;

uint32_t earDanceUntil = 0;
uint32_t earDroopUntil = 0;
uint32_t earAlertUntil = 0;

// ---------- sleep / wake ----------
static uint32_t wakeTimeoutMs = 0;
static bool autoSlept = false;
static uint32_t wakeAnimStart = 0;   // ear stretch after wake / rozciaganie uszu

// ---------- CRT power transition / przejscie CRT ----------
static uint8_t crtState = 0;         // 0 none, 1 = ON (wake), 2 = OFF (sleep)
static uint32_t crtStartMs = 0;
static constexpr uint16_t CRT_ON_MS = 900;
static constexpr uint16_t CRT_OFF_MS = 700;
static float crtFreezeOpenL = 1.0f;
static float crtFreezeOpenR = 1.0f;

// ---------- EYESCFG tuning / strojenie ----------
static uint32_t blinkMinMs = 1800;
static uint32_t blinkMaxMs = 5200;

// ---------- blink patterns / wzorce mrugania ----------
static float blinkMinOpen = 0.0f;
static bool blinkDoubleQueued = false;
static uint32_t queuedSecondBlinkMs = 0;
static uint8_t blinkStyle = 0;

// ---------- heartbeat ----------
static uint32_t lastHbMs = 0;

static void enterPiSleep(bool forcePose);

// ---------- iris fibers ----------
static constexpr int FIB_N = 30;
static float fibA[FIB_N];
static float fibLen[FIB_N];

// ---------- eye state ----------
struct FaceState {
  float lookX = 0.0f;
  float lookY = 0.0f;
  float openL = 1.0f;
  float openR = 1.0f;
  float tiltL = 0.0f;
  float tiltR = 0.0f;
  float curveL = 0.0f;
  float curveR = 0.0f;
  float pupil = 0.5f;
  uint16_t color = 0;
};

FaceState faceNow;
FaceState faceTarget;
String currentMood = "full_sleep";

bool quietMode = true;
bool talkMode = false;
bool thinkMode = false;
bool soulMode = true;
bool personDetected = false;
String talkMoodName = "talking_soft";

bool blinkActive = false;
uint32_t blinkStartMs = 0;
uint16_t blinkDurationMs = 180;
uint32_t nextBlinkMs = 0;

uint32_t nextSoulMs = 0;
uint32_t nextTalkMs = 0;
uint32_t nextThinkMs = 0;
uint32_t lastFrameMs = 0;
uint32_t lastEarMs = 0;
uint32_t lastInteractionMs = 0;

String serialLine;

static int upperLine[160];
static int lowerLine[160];

// ============================================================
// Utility
// ============================================================
static float clampF(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static int clampI(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static String trimCopy(String s) {
  s.trim();
  return s;
}

static String lowerCopy(String s) {
  s.trim();
  s.toLowerCase();
  return s;
}

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static uint16_t dimColor(uint16_t c, float f) {
  uint8_t r = (uint8_t)(((c >> 11) & 0x1F) * f);
  uint8_t g = (uint8_t)(((c >> 5) & 0x3F) * f);
  uint8_t b = (uint8_t)((c & 0x1F) * f);
  return ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
}

static uint16_t mixWhite(uint16_t c, float f) {
  uint8_t r = (uint8_t)((c >> 11) & 0x1F);
  uint8_t g = (uint8_t)((c >> 5) & 0x3F);
  uint8_t b = (uint8_t)(c & 0x1F);
  r = (uint8_t)(r + (31 - r) * f);
  g = (uint8_t)(g + (63 - g) * f);
  b = (uint8_t)(b + (31 - b) * f);
  return (r << 11) | (g << 5) | b;
}

// slow iris color drift / powolny drift barwy teczowki
static uint16_t driftColor(uint16_t c, uint32_t now) {
  float t = (float)now * 0.00008f;
  float fr = 1.0f + 0.10f * sinf(t);
  float fg = 1.0f + 0.10f * sinf(t + 2.1f);
  float fb = 1.0f + 0.10f * sinf(t + 4.2f);
  uint8_t r = (uint8_t)clampI((int)(((c >> 11) & 0x1F) * fr), 0, 31);
  uint8_t g = (uint8_t)clampI((int)(((c >> 5) & 0x3F) * fg), 0, 63);
  uint8_t b = (uint8_t)clampI((int)((c & 0x1F) * fb), 0, 31);
  return (r << 11) | (g << 5) | b;
}

static uint8_t hash2i(int x, int y) {
  uint32_t h = (uint32_t)x * 7349u ^ (uint32_t)y * 911u;
  h ^= h >> 8;
  return (uint8_t)(h & 255);
}

static void setEarLogicalTargets(float leftOffset, float rightOffset) {
  earLTarget = clampF(leftOffset, -35.0f, 35.0f);
  earRTarget = clampF(rightOffset, -35.0f, 35.0f);
}

static int logicalEarToAngle(float logical, bool reversed, int center, int lo, int hi) {
  float d = reversed ? -logical : logical;
  return clampI((int)lroundf((float)center + d), lo, hi);
}

static void triggerEarPerk() {
  if (quietMode) return;
  earPerkMs = millis();
}

static void updateEars() {
  uint32_t now = millis();
  if (now - lastEarMs < 12) return;
  lastEarMs = now;

  float baseL = earLTarget;
  float baseR = earRTarget;

  if (!quietMode) {
    float swayAmp = talkMode ? 2.5f : 5.0f;
    baseL += swayAmp * sinf(now * 0.00055f);
    baseR += swayAmp * sinf(now * 0.00055f + 1.1f);
  }

  // mood ear dances / tance uszu
  if (now < earDanceUntil) {
    float w = 11.0f * sinf(now * 0.018f);
    baseL += w;
    baseR -= w;
  }
  if (now < earDroopUntil) {
    baseL -= 12.0f;
    baseR -= 12.0f;
  }
  if (now < earAlertUntil) {
    baseL += 16.0f;
    baseR += 16.0f;
  }

  // wake ear stretch / rozciaganie uszu po przebudzeniu
  if (wakeAnimStart && now - wakeAnimStart < 1800) {
    float p = 1.0f - (float)(now - wakeAnimStart) / 1800.0f;
    baseL += 20.0f * p;
    baseR += 20.0f * p;
  }

  if (earPerkMs) {
    uint32_t age = now - earPerkMs;
    if (age >= EAR_PERK_MS) {
      earPerkMs = 0;
    } else {
      float p = (float)age / (float)EAR_PERK_MS;
      float env = sinf(p * PI);
      baseL += 14.0f * env;
      baseR += 14.0f * env;
    }
  }

  const float step = 0.8f;
  float dl = baseL - earLNow;
  float dr = baseR - earRNow;
  earLNow += clampF(dl, -step, step);
  earRNow += clampF(dr, -step, step);

  earL.write(logicalEarToAngle(earLNow, EAR_L_REVERSED, EAR_CENTER_L, EAR_MIN_L, EAR_MAX_L));
  earR.write(logicalEarToAngle(earRNow, EAR_R_REVERSED, EAR_CENTER_R, EAR_MIN_R, EAR_MAX_R));
}

// ============================================================
// Face / mood mapping
// ============================================================
static void setBaseFace(float x, float y, float oL, float oR,
                        float tL, float tR, float cL, float cR,
                        uint16_t col, float pupil, float eL, float eR) {
  faceTarget.lookX = clampF(x, -1.0f, 1.0f);
  faceTarget.lookY = clampF(y, -1.0f, 1.0f);
  faceTarget.openL = clampF(oL, 0.02f, 1.0f);
  faceTarget.openR = clampF(oR, 0.02f, 1.0f);
  faceTarget.tiltL = clampF(tL, -1.0f, 1.0f);
  faceTarget.tiltR = clampF(tR, -1.0f, 1.0f);
  faceTarget.curveL = clampF(cL, -1.0f, 1.0f);
  faceTarget.curveR = clampF(cR, -1.0f, 1.0f);
  faceTarget.pupil = clampF(pupil, 0.0f, 1.0f);
  faceTarget.color = col;
  setEarLogicalTargets(eL, eR);
}

static bool isSleepMood(const String &nameIn) {
  String n = lowerCopy(nameIn);
  return n == "sleepy_closed" || n == "full_sleep" || n == "sleep";
}

static void setMoodFx(const String &n) {
  blinkStyle = 0;
  blinkMinOpen = 0.0f;
  if (n == "angry" || n == "angry_left" || n == "angry_right" || n == "fear") {
    blinkStyle = 1;
  } else if (n == "happy" || n == "happy_big" || n == "excited" ||
             n == "dog_like" || n == "yes_soft" || n == "wink_left" || n == "wink_right") {
    blinkStyle = 2;
    earDanceUntil = millis() + 2500;
  } else if (n == "bored" || n == "sad" || n == "shy" ||
             n.startsWith("sleepy") || n == "full_sleep") {
    blinkStyle = 3;
    blinkMinOpen = 0.5f;
    if (n == "sad" || n == "shy") earDroopUntil = millis() + 4000;
  }
  if (n == "alert" || n == "human_found") {
    earAlertUntil = millis() + 3000;
  }
}

static bool applyMood(const String &nameIn) {
  String n = lowerCopy(nameIn);
  if (!n.length()) return false;

  const uint16_t CYAN    = rgb565(35, 225, 255);
  const uint16_t ICE     = rgb565(155, 245, 255);
  const uint16_t GREEN   = rgb565(70, 255, 150);
  const uint16_t BLUE    = rgb565(70, 145, 255);
  const uint16_t PURPLE  = rgb565(195, 105, 255);
  const uint16_t MAGENTA = rgb565(255, 75, 210);
  const uint16_t RED     = rgb565(255, 60, 45);
  const uint16_t ORANGE  = rgb565(255, 155, 40);

  if (n == "neutral" || n == "idle_watch" || n == "calm_watch") {
    setBaseFace(0, 0, 0.88f, 0.88f, 0.12f, 0.12f, 0, 0, CYAN, 0.50f, 0, 0);
  }
  else if (n == "calm_soft") {
    setBaseFace(0, 0, 0.80f, 0.80f, 0.05f, 0.05f, 0.15f, 0.15f, PURPLE, 0.50f, 6, 6);
  }
  else if (n == "sleepy" || n == "sleepy_watch" || n == "sleepy_open") {
    setBaseFace(0, 0.26f, 0.42f, 0.42f, 0.10f, 0.10f, 0.10f, 0.10f, BLUE, 0.30f, -18, -18);
  }
  else if (n == "sleepy_listen") {
    setBaseFace(0, 0.16f, 0.50f, 0.50f, 0.05f, 0.05f, 0.05f, 0.05f, BLUE, 0.40f, 18, 18);
  }
  else if (n == "sleepy_closed" || n == "full_sleep") {
    setBaseFace(0, 0.30f, 0.03f, 0.03f, 0.10f, 0.10f, 0.20f, 0.20f, BLUE, 0.25f, -28, -28);
  }
  else if (n == "alert" || n == "human_found") {
    setBaseFace(0, -0.06f, 1.0f, 1.0f, 0, 0, -0.10f, -0.10f, ICE, 0.70f, 24, 24);
  }
  else if (n == "happy") {
    setBaseFace(0, 0.06f, 0.86f, 0.86f, 0, 0, 0.55f, 0.55f, GREEN, 0.55f, 16, 16);
  }
  else if (n == "happy_big" || n == "excited") {
    setBaseFace(0, -0.08f, 1.0f, 1.0f, 0, 0, 0.35f, 0.35f, GREEN, 0.75f, 30, 30);
  }
  else if (n == "proud") {
    setBaseFace(0, -0.06f, 0.74f, 0.74f, -0.20f, -0.20f, 0.15f, 0.15f, MAGENTA, 0.50f, 12, 12);
  }
  else if (n == "proud_left") {
    setBaseFace(-0.55f, -0.06f, 0.74f, 0.74f, -0.20f, -0.20f, 0.15f, 0.15f, MAGENTA, 0.50f, 16, 8);
  }
  else if (n == "proud_right") {
    setBaseFace(0.55f, -0.06f, 0.74f, 0.74f, -0.20f, -0.20f, 0.15f, 0.15f, MAGENTA, 0.50f, 8, 16);
  }
  else if (n == "curious") {
    setBaseFace(-0.55f, -0.10f, 0.95f, 0.82f, 0, 0, -0.05f, 0.05f, CYAN, 0.65f, 26, 8);
  }
  else if (n == "curious_right") {
    setBaseFace(0.55f, -0.10f, 0.82f, 0.95f, 0, 0, 0.05f, -0.05f, CYAN, 0.65f, 8, 26);
  }
  else if (n == "suspicious_left" || n == "skeptical_soft") {
    setBaseFace(-0.62f, 0, 0.48f, 0.72f, 0.40f, 0.10f, 0.05f, 0.05f, ORANGE, 0.35f, -8, 14);
  }
  else if (n == "suspicious_right") {
    setBaseFace(0.62f, 0, 0.72f, 0.48f, 0.10f, 0.40f, 0.05f, 0.05f, ORANGE, 0.35f, 14, -8);
  }
  else if (n == "angry") {
    setBaseFace(0, 0, 0.80f, 0.80f, 0.85f, 0.85f, 0, 0, RED, 0.40f, -18, -18);
  }
  else if (n == "angry_left") {
    setBaseFace(-0.50f, 0, 0.78f, 0.82f, 0.85f, 0.70f, 0, 0, RED, 0.40f, -20, -14);
  }
  else if (n == "angry_right") {
    setBaseFace(0.50f, 0, 0.82f, 0.78f, 0.70f, 0.85f, 0, 0, RED, 0.40f, -14, -20);
  }
  else if (n == "sad") {
    setBaseFace(0, 0.22f, 0.62f, 0.62f, -0.10f, -0.10f, -0.35f, -0.35f, BLUE, 0.35f, -18, -18);
  }
  else if (n == "shy") {
    setBaseFace(-0.28f, 0.28f, 0.58f, 0.58f, 0, 0, 0.30f, 0.30f, PURPLE, 0.45f, -12, -12);
  }
  else if (n == "surprised" || n == "fear") {
    setBaseFace(0, -0.04f, 1.0f, 1.0f, 0, 0, -0.20f, -0.20f,
                n == "fear" ? ORANGE : MAGENTA,
                n == "fear" ? 0.95f : 0.85f,
                n == "fear" ? -30 : 30,
                n == "fear" ? -30 : 30);
  }
  else if (n == "bored") {
    setBaseFace(0.30f, 0.16f, 0.32f, 0.32f, 0.22f, 0.22f, 0, 0, BLUE, 0.28f, -22, -22);
  }
  else if (n == "listening" || n == "gentle_focus") {
    setBaseFace(0, -0.04f, 0.90f, 0.90f, 0, 0, 0, 0, ICE, 0.65f, 28, 28);
  }
  else if (n == "talking_soft") {
    setBaseFace(0, 0, 0.86f, 0.86f, 0.08f, 0.08f, 0.05f, 0.05f, CYAN, 0.50f, 10, 10);
  }
  else if (n == "talking_wide") {
    setBaseFace(0, -0.06f, 0.98f, 0.98f, 0, 0, 0, 0, CYAN, 0.60f, 16, 16);
  }
  else if (n == "wink_left") {
    setBaseFace(0, 0, 0.03f, 0.88f, 0, 0.10f, 0.70f, 0.05f, GREEN, 0.55f, 14, 14);
  }
  else if (n == "wink_right") {
    setBaseFace(0, 0, 0.88f, 0.03f, 0.10f, 0, 0.05f, 0.70f, GREEN, 0.55f, 14, 14);
  }
  else if (n == "search_left") {
    setBaseFace(-0.92f, 0, 0.88f, 0.88f, 0.05f, 0.05f, 0, 0, CYAN, 0.55f, 16, 8);
  }
  else if (n == "search_right") {
    setBaseFace(0.92f, 0, 0.88f, 0.88f, 0.05f, 0.05f, 0, 0, CYAN, 0.55f, 8, 16);
  }
  else if (n == "track_center") {
    setBaseFace(0, 0, 0.90f, 0.90f, 0.06f, 0.06f, 0, 0, ICE, 0.55f, 16, 16);
  }
  else if (n == "yes_soft") {
    setBaseFace(0, 0.22f, 0.84f, 0.84f, 0.05f, 0.05f, 0.20f, 0.20f, GREEN, 0.50f, 14, 14);
  }
  else if (n == "no_soft") {
    setBaseFace(-0.22f, 0, 0.78f, 0.78f, 0.15f, 0.15f, 0, 0, ORANGE, 0.45f, 2, 2);
  }
  else if (n == "dog_like") {
    setBaseFace(0, -0.04f, 0.96f, 0.96f, -0.10f, -0.10f, 0.45f, 0.45f, GREEN, 0.80f, 30, 30);
  }
  else if (n == "comfort" || n == "tender") {
    setBaseFace(0, 0.06f, 0.72f, 0.72f, 0, 0, 0.40f, 0.40f, PURPLE, 0.55f, 18, 18);
  }
  else if (n == "thinking") {
    setBaseFace(-0.42f, -0.46f, 0.80f, 0.80f, 0.10f, 0.20f, 0, 0, PURPLE, 0.45f, 10, 18);
  }
  else {
    return false;
  }

  currentMood = n;
  lastInteractionMs = millis();
  autoSlept = false;
  setMoodFx(n);
  return true;
}

// ============================================================
// Animation
// ============================================================
static void startBlink(uint16_t ms = 180) {
  if (quietMode) return;
  if (crtState) return;   // no blinking during CRT transition / bez mrugania w CRT
  blinkActive = true;
  blinkStartMs = millis();
  switch (blinkStyle) {
    case 1: blinkDurationMs = clampI(ms, 90, 140); break;
    case 3: blinkDurationMs = clampI(ms, 350, 450); break;
    default: blinkDurationMs = clampI(ms, 100, 500); break;
  }
}

static void scheduleNextBlink() {
  uint32_t lo = blinkMinMs, hi = blinkMaxMs;
  if (hi < lo) hi = lo;
  nextBlinkMs = millis() + (uint32_t)random(lo, hi);
}

static void scheduleNextSoul() {
  nextSoulMs = millis() + (uint32_t)random(1800, 4600);
}

static float easeToward(float now, float target, float k) {
  return now + (target - now) * k;
}

static void updateAutonomy() {
  uint32_t now = millis();

  if (!quietMode && wakeTimeoutMs > 0 && now - lastInteractionMs > wakeTimeoutMs) {
    enterPiSleep(true);
    if (!autoSlept) {
      autoSlept = true;
      Serial.println("OK AUTO_SLEEP timeout=" + String(wakeTimeoutMs));
    }
    return;
  }

  if (queuedSecondBlinkMs && now >= queuedSecondBlinkMs) {
    queuedSecondBlinkMs = 0;
    startBlink(150);
  }

  if (!quietMode && !crtState && now >= nextBlinkMs && !blinkActive) {
    startBlink((uint16_t)random(150, 230));
    scheduleNextBlink();
  }

  if (talkMode && !quietMode && now >= nextTalkMs) {
    nextTalkMs = now + (uint32_t)random(120, 240);
    faceTarget.lookX = clampF(faceTarget.lookX + (float)random(-10, 11) / 100.0f, -0.35f, 0.35f);
    faceTarget.lookY = clampF(faceTarget.lookY + (float)random(-7, 8) / 100.0f, -0.20f, 0.20f);
    setEarLogicalTargets(10 + random(-6, 7), 10 + random(-6, 7));
    lastInteractionMs = now;
  }

  if (thinkMode && !quietMode && now >= nextThinkMs) {
    nextThinkMs = now + (uint32_t)random(500, 900);
    faceTarget.lookX = -0.25f + (float)random(-15, 16) / 100.0f;
    faceTarget.lookY = -0.50f + (float)random(-8, 9) / 100.0f;
  }

  if (soulMode && !talkMode && !thinkMode && !quietMode && now >= nextSoulMs) {
    scheduleNextSoul();
    int r = random(0, 100);
    if (r < 18) {
      startBlink((uint16_t)random(150, 220));
    } else if (r < 54) {
      faceTarget.lookX = (float)random(-55, 56) / 100.0f;
      faceTarget.lookY = (float)random(-18, 19) / 100.0f;
    } else if (r < 76) {
      setEarLogicalTargets((float)random(5, 19), (float)random(5, 19));
    } else {
      applyMood("idle_watch");
    }
  }
}

static float blinkEnvelope() {
  if (!blinkActive) return 1.0f;
  uint32_t now = millis();
  uint32_t dt = now - blinkStartMs;
  if (dt >= blinkDurationMs) {
    blinkActive = false;
    if (blinkStyle == 2 && !queuedSecondBlinkMs) {
      queuedSecondBlinkMs = now + 180;
    }
    return 1.0f;
  }
  float p = (float)dt / (float)blinkDurationMs;
  float span = 1.0f - blinkMinOpen;
  if (p < 0.45f) return 1.0f - (p / 0.45f) * span;
  if (p < 0.60f) return blinkMinOpen;
  if (p < 0.85f) return blinkMinOpen + (p - 0.60f) / 0.25f * span * 1.08f;
  return 1.08f - (p - 0.85f) / 0.15f * span * 0.08f;
}

static void updateFaceMotion() {
  faceNow.lookX = easeToward(faceNow.lookX, faceTarget.lookX, 0.22f);
  faceNow.lookY = easeToward(faceNow.lookY, faceTarget.lookY, 0.22f);
  faceNow.openL = easeToward(faceNow.openL, faceTarget.openL, 0.25f);
  faceNow.openR = easeToward(faceNow.openR, faceTarget.openR, 0.25f);
  faceNow.tiltL = easeToward(faceNow.tiltL, faceTarget.tiltL, 0.18f);
  faceNow.tiltR = easeToward(faceNow.tiltR, faceTarget.tiltR, 0.18f);
  faceNow.curveL = easeToward(faceNow.curveL, faceTarget.curveL, 0.18f);
  faceNow.curveR = easeToward(faceNow.curveR, faceTarget.curveR, 0.18f);
  faceNow.pupil = easeToward(faceNow.pupil, faceTarget.pupil, 0.12f);
  faceNow.color = faceTarget.color;
}

// ============================================================
// NEON EYE RENDERER
// ============================================================
static void renderEye(GFXcanvas16 &cv, float open, float tilt, float curve,
                      float lookX, float lookY, float pupil,
                      uint16_t colIn, bool leftEye, float breathPx) {
  const int ecx = EYEW / 2;
  const int ecy = EYEH / 2;
  const float halfW = (float)EYEW * 0.4125f;
  const float vScale = (float)EYEH / 240.0f;

  uint16_t col = driftColor(colIn, millis());

  cv.fillScreen(ST77XX_BLACK);

  // sleep: dim but visible silhouette / sen: przygaszona widoczna sylwetka
  float pulse = quietMode
      ? 0.45f + 0.12f * sinf((float)millis() * 0.003f)
      : 1.0f + 0.08f * sinf((float)millis() * 0.003f);

  int gw = (int)(halfW + 13.0f);
  int gh = (int)(82.0f * vScale);
  cv.fillRoundRect(ecx - gw - 6, ecy - gh - 4, (gw + 6) * 2, (gh + 4) * 2, gh,
                   dimColor(col, 0.05f * pulse));
  cv.fillRoundRect(ecx - gw, ecy - gh, gw * 2, gh * 2, gh - 6,
                   dimColor(col, 0.11f * pulse));
  cv.fillRoundRect(ecx - (int)halfW - 4, ecy - (int)(62.0f * vScale),
                   (int)halfW * 2 + 8, (int)(124.0f * vScale), (int)(60.0f * vScale),
                   dimColor(col, 0.20f * pulse));
  cv.fillRoundRect(ecx - (int)halfW - 4, ecy - (int)(58.0f * vScale),
                   (int)halfW * 2 + 8, (int)(116.0f * vScale), (int)(54.0f * vScale),
                   mixWhite(col, 0.62f));

  int ix = ecx + (int)(lookX * 17.0f);
  int iy = ecy + (int)(lookY * 13.0f * vScale);
  const int irisR = (int)(36.0f * vScale) + ((EYEW > 152) ? 2 : 0);

  cv.fillCircle(ix, iy, irisR + 2, rgb565(8, 18, 26));
  cv.fillCircle(ix, iy, irisR - 1, dimColor(col, 0.45f));
  cv.fillCircle(ix, iy, irisR - 7, col);
  cv.fillCircle(ix, iy, irisR - 14, mixWhite(col, 0.28f));
  cv.fillCircle(ix, iy, irisR - 20, mixWhite(col, 0.45f));

  for (int f = 0; f < FIB_N; f++) {
    float a = fibA[f] + (leftEye ? 0.0f : 0.05f);
    float rOut = (float)(irisR - 3);
    float rIn = rOut - 14.0f * fibLen[f] * vScale;
    int x1 = ix + (int)(cosf(a) * rOut);
    int y1 = iy + (int)(sinf(a) * rOut);
    int x2 = ix + (int)(cosf(a) * rIn);
    int y2 = iy + (int)(sinf(a) * rIn);
    cv.drawLine(x1, y1, x2, y2, dimColor(col, 0.40f));
  }

  int pr = (int)((10.0f + 11.0f * pupil) * vScale);
  cv.fillCircle(ix, iy, pr, rgb565(8, 12, 18));
  cv.fillCircle(ix - pr / 3, iy - pr / 3, max(1, pr / 5), rgb565(50, 70, 90));

  cv.fillCircle(ix - 11, iy - 12, (int)(7 * vScale), mixWhite(col, 0.92f));
  cv.fillCircle(ix + 10, iy + 9, (int)(3 * vScale), ST77XX_WHITE);
  cv.fillCircle(ix + 4, iy + 15, (int)(2 * vScale), dimColor(ST77XX_WHITE, 0.35f));

  float innerDir = leftEye ? 1.0f : -1.0f;

  uint16_t rimCol  = mixWhite(col, 0.40f);
  uint16_t rimCol2 = dimColor(col, 0.50f);
  uint16_t rimCol3 = dimColor(col, 0.28f);
  uint16_t tearCol  = mixWhite(col, 0.55f);
  uint16_t tearCol2 = mixWhite(col, 0.30f);

  for (int x = 0; x < EYEW; x++) {
    float nx = ((float)x - ecx) / halfW;
    float ax = fabsf(nx);
    if (ax > 1.0f || x >= 160) {
      cv.drawFastVLine(x, 0, EYEH, ST77XX_BLACK);
      continue;
    }
    float wr = sqrtf(1.0f - nx * nx);

    float halfOpen = 8.0f + 64.0f * open * vScale;
    float uy = (float)ecy - halfOpen * (0.50f + 0.50f * wr)
             + tilt * innerDir * nx * 22.0f * vScale
             + curve * wr * 16.0f * vScale
             + breathPx;
    float ly = (float)ecy + (12.0f + 32.0f * open) * vScale * (0.55f + 0.45f * wr)
             - curve * wr * 12.0f * vScale
             + breathPx;

    // CLOSED EYE: single arc line instead of a slit / sen: jedna kreska
    if (open < 0.09f) {
      ly = uy + 2.0f;
    }

    int uyi = clampI((int)lroundf(uy), 0, EYEH - 1);
    int lyi = clampI((int)lroundf(ly), 0, EYEH - 1);
    if (lyi <= uyi) lyi = uyi + 1;
    upperLine[x] = uyi;
    lowerLine[x] = lyi;

    if (uyi > 0) cv.drawFastVLine(x, 0, uyi, ST77XX_BLACK);
    if (lyi < EYEH - 1) cv.drawFastVLine(x, lyi, EYEH - lyi, ST77XX_BLACK);

    cv.drawPixel(x, uyi, rimCol);
    if (uyi + 1 < EYEH) cv.drawPixel(x, uyi + 1, rimCol2);
    if (uyi + 2 < EYEH) cv.drawPixel(x, uyi + 2, rimCol3);

    if (lyi - 2 >= 0) cv.drawPixel(x, lyi - 2, tearCol);
    if (lyi - 3 >= 0) cv.drawPixel(x, lyi - 3, tearCol2);
  }

  uint16_t *buf = cv.getBuffer();
  uint16_t underCol = rgb565(30, 45, 70);
  for (int x = 0; x < EYEW && x < 160; x++) {
    int uyi = upperLine[x];
    for (int d = 3; d <= 8; d++) {
      int y = uyi + d;
      if (y >= EYEH) break;
      uint16_t *p = &buf[(size_t)y * EYEW + x];
      *p = dimColor(*p, 0.70f - (float)(d - 3) * 0.075f);
    }
    int lyi = lowerLine[x];
    for (int d = 2; d <= 6; d++) {
      int y = lyi + d;
      if (y >= EYEH) break;
      uint16_t *p = &buf[(size_t)y * EYEW + x];
      float f = 0.35f - (float)(d - 2) * 0.08f;
      *p = dimColor(underCol, f);
    }
  }

  int dTop = ecy - (int)(56.0f * vScale);
  int dBot = ecy + (int)(56.0f * vScale);
  for (int y = dTop; y < dBot; y += 2) {
    if (y < 0 || y >= EYEH) continue;
    for (int x = ((y & 1) ? 1 : 0); x < EYEW; x += 2) {
      uint16_t *p = &buf[(size_t)y * EYEW + x];
      uint8_t h = hash2i(x, y);
      if (h < 40) *p = dimColor(*p, 0.94f);
      else if (h > 215) *p = mixWhite(*p, 0.05f);
    }
  }
}

// ============================================================
// CRT mask: dot -> line -> full / kropka -> linia -> pelne
// ============================================================
static void applyCrtMask(GFXcanvas16 &cv, uint16_t col) {
  if (!crtState) return;
  uint32_t now = millis();
  float p;
  float hw, hh;
  const int cx = EYEW / 2;
  const int cy = EYEH / 2;

  if (crtState == 1) {
    // CRT ON: dot -> vertical line -> horizontal expand
    p = clampF((float)(now - crtStartMs) / (float)CRT_ON_MS, 0.0f, 1.0f);
    float s1 = p / 0.30f;
    float s2 = (p - 0.30f) / 0.25f;
    float s3 = (p - 0.55f) / 0.45f;
    if (p < 0.30f)      { hh = 2.0f + s1 * 6.0f;              hw = hh * 0.7f; }
    else if (p < 0.55f) { hh = 8.0f + s2 * (float)(cy - 8);   hw = 8.0f; }
    else                { hh = (float)cy;                     hw = 8.0f + s3 * (float)(cx - 8); }
  } else {
    // CRT OFF: collapse to line -> dot -> black
    p = clampF((float)(now - crtStartMs) / (float)CRT_OFF_MS, 0.0f, 1.0f);
    float s1 = p / 0.40f;
    float s2 = (p - 0.40f) / 0.30f;
    float s3 = (p - 0.70f) / 0.30f;
    if (p < 0.40f)      { hw = (float)cx - s1 * ((float)cx - 10.0f); hh = (float)cy; }
    else if (p < 0.70f) { hw = 10.0f; hh = (float)cy - s2 * ((float)cy - 3.0f); }
    else                { hw = 10.0f * (1.0f - s3); hh = 3.0f; }
  }

  int hwi = (int)lroundf(hw);
  int hhi = (int)lroundf(hh);

  // black everything outside the window / czern poza oknem
  cv.fillRect(0, 0, cx - hwi, EYEH, ST77XX_BLACK);
  cv.fillRect(cx + hwi, 0, EYEW - (cx + hwi), EYEH, ST77XX_BLACK);
  cv.fillRect(cx - hwi, 0, hwi * 2, cy - hhi, ST77XX_BLACK);
  cv.fillRect(cx - hwi, cy + hhi, hwi * 2, EYEH - (cy + hhi), ST77XX_BLACK);

  // bright phosphor edge / jasna krawedz fosforu
  uint16_t edge = mixWhite(col, 0.90f);
  if (hwi > 1 && hwi < cx) {
    cv.fillRect(cx - hwi, cy - hhi, 1, hhi * 2, edge);
    cv.fillRect(cx + hwi - 1, cy - hhi, 1, hhi * 2, edge);
  }
  if (hhi > 1 && hwi <= 10) {
    cv.fillRect(cx - hwi, cy - hhi, hwi * 2, 1, edge);
    cv.fillRect(cx - hwi, cy + hhi - 1, hwi * 2, 1, edge);
  }
}

static void drawFace() {
  uint32_t now = millis();
  if (now - lastFrameMs < 33) return;
  lastFrameMs = now;

  if (!eyeCanvas || !eyeCanvas->getBuffer()) return;

  // CRT state transitions
  if (crtState == 2 && now - crtStartMs > CRT_OFF_MS) {
    crtState = 0;
    faceNow.openL = faceNow.openR = 0.03f;
  }
  if (crtState == 1 && now - crtStartMs > CRT_ON_MS) {
    crtState = 0;
  }

  float env = (crtState == 2) ? 1.0f : blinkEnvelope();
  float oL = (crtState == 2) ? crtFreezeOpenL : clampF(faceNow.openL * env, 0.0f, 1.05f);
  float oR = (crtState == 2) ? crtFreezeOpenR : clampF(faceNow.openR * env, 0.0f, 1.05f);

  // anti burn-in: slow 1px wander / anty wypalanie
  int driftX = (int)roundf(sinf(now * 0.00004f) * 1.4f);
  int driftY = (int)roundf(sinf(now * 0.00003f + 1.7f) * 1.4f);

  // sleep breathing / oddech snu
  float breath = 0.0f;
  if (quietMode) {
    breath = 2.4f * (0.5f + 0.5f * sinf(now * 0.0015f));
  }

  const int xL = (160 - EYEW) / 2 + driftX;
  const int xR = 160 + (160 - EYEW) / 2 + driftX;
  const int yT = (240 - EYEH) / 2 + driftY;

  renderEye(*eyeCanvas, oL, faceNow.tiltL, faceNow.curveL,
            faceNow.lookX, faceNow.lookY, faceNow.pupil, faceNow.color, true, breath);
  applyCrtMask(*eyeCanvas, faceNow.color);
  tft.drawRGBBitmap(xL, yT, eyeCanvas->getBuffer(), EYEW, EYEH);

  renderEye(*eyeCanvas, oR, faceNow.tiltR, faceNow.curveR,
            faceNow.lookX, faceNow.lookY, faceNow.pupil, faceNow.color, false, breath);
  applyCrtMask(*eyeCanvas, faceNow.color);
  tft.drawRGBBitmap(xR, yT, eyeCanvas->getBuffer(), EYEW, EYEH);
}

// ============================================================
// Control helpers
// ============================================================
static void wakeFromPi(uint32_t ms = 8000, const String &moodName = "idle_watch") {
  wakeTimeoutMs = ms;
  quietMode = false;
  talkMode = false;
  thinkMode = false;
  autoSlept = false;
  lastInteractionMs = millis();
  applyMood(moodName.length() ? moodName : "idle_watch");

  // CRT ON: eyes fully formed, mask reveals them / oczy gotowe, maska odkrywa
  faceNow = faceTarget;
  crtState = 1;
  crtStartMs = millis();
  wakeAnimStart = millis();

  scheduleNextBlink();
  scheduleNextSoul();
  triggerEarPerk();
}

static void enterPiSleep(bool forcePose = true) {
  (void)forcePose;
  // CRT OFF only if eyes were visible / CRT OFF tylko gdy oczy widoczne
  if (!quietMode && !crtState) {
    crtState = 2;
    crtStartMs = millis();
    crtFreezeOpenL = faceNow.openL;
    crtFreezeOpenR = faceNow.openR;
  }
  quietMode = true;
  talkMode = false;
  thinkMode = false;
  blinkActive = false;
  earPerkMs = 0;
  wakeTimeoutMs = 0;
  wakeAnimStart = 0;
  currentMood = "full_sleep";
  applyMood("full_sleep");
}

static void setQuietMode(bool on, bool forcePose = true) {
  if (on) enterPiSleep(forcePose);
  else wakeFromPi(8000, "idle_watch");
}

static void setTalkMode(bool on, const String &moodName) {
  if (quietMode && on) return;
  talkMode = on;
  lastInteractionMs = millis();
  if (on) {
    thinkMode = false;
    talkMoodName = moodName.length() ? moodName : "talking_soft";
    if (!applyMood(talkMoodName)) applyMood("talking_soft");
    nextTalkMs = 0;
  } else {
    applyMood("idle_watch");
  }
}

static void setThinkMode(bool on) {
  if (quietMode && on) return;
  thinkMode = on;
  lastInteractionMs = millis();
  if (on) {
    talkMode = false;
    applyMood("thinking");
    nextThinkMs = 0;
  } else {
    applyMood("idle_watch");
  }
}

static void handlePersonFound() {
  personDetected = true;
  if (quietMode) {
    wakeFromPi(8000, "human_found");
  } else {
    lastInteractionMs = millis();
    applyMood("human_found");
  }
}

static void handlePersonLost() {
  personDetected = false;
  lastInteractionMs = millis();
  if (!quietMode) applyMood("idle_watch");
}

// ============================================================
// EYESCFG runtime tuning / strojenie w locie
// ============================================================
static void saveCfg() {
  prefs.begin("koraeyes", false);
  prefs.putULong("bmin", blinkMinMs);
  prefs.putULong("bmax", blinkMaxMs);
  prefs.end();
}

static void loadCfg() {
  prefs.begin("koraeyes", true);
  blinkMinMs = prefs.getULong("bmin", 1800);
  blinkMaxMs = prefs.getULong("bmax", 5200);
  prefs.end();
  if (blinkMaxMs < blinkMinMs) blinkMaxMs = blinkMinMs;
}

static void handleEyesCfg(const String &args) {
  int start = 0;
  bool changed = false;
  while (start < args.length()) {
    int sp = args.indexOf(' ', start);
    if (sp < 0) sp = args.length();
    String pair = args.substring(start, sp);
    start = sp + 1;
    int eq = pair.indexOf('=');
    if (eq <= 0) continue;
    String key = lowerCopy(pair.substring(0, eq));
    long val = pair.substring(eq + 1).toInt();
    if (key == "blinkmin" && val >= 300 && val <= 30000) { blinkMinMs = (uint32_t)val; changed = true; }
    else if (key == "blinkmax" && val >= 300 && val <= 60000) { blinkMaxMs = (uint32_t)val; changed = true; }
  }
  if (blinkMaxMs < blinkMinMs) blinkMaxMs = blinkMinMs;
  if (changed) saveCfg();
  Serial.println("OK EYESCFG blinkmin=" + String(blinkMinMs) +
                 " blinkmax=" + String(blinkMaxMs) + (changed ? " saved" : ""));
}

// ============================================================
// Serial protocol
// ============================================================
static void printStatus() {
  Serial.print("STATUS mood="); Serial.print(currentMood);
  Serial.print(" talk="); Serial.print(talkMode ? 1 : 0);
  Serial.print(" think="); Serial.print(thinkMode ? 1 : 0);
  Serial.print(" quiet="); Serial.print(quietMode ? 1 : 0);
  Serial.print(" soul="); Serial.print(soulMode ? 1 : 0);
  Serial.print(" person="); Serial.print(personDetected ? 1 : 0);
  Serial.print(" wakeTimeout="); Serial.print(wakeTimeoutMs);
  Serial.print(" idle="); Serial.print(quietMode ? 0 : (int)(millis() - lastInteractionMs));
  Serial.print(" earsL="); Serial.print((int)earLNow);
  Serial.print(" earsR="); Serial.print((int)earRNow);
  Serial.print(" heap="); Serial.print((int)ESP.getFreeHeap());
  Serial.print(" uptime="); Serial.print((unsigned long)(millis() / 1000));
  Serial.print(" reset="); Serial.print((int)esp_reset_reason());
  Serial.print(" ap="); Serial.print(apMode ? 1 : 0);
  Serial.print(" ip="); Serial.println(WiFi.isConnected() ? WiFi.localIP().toString() : String(apMode ? WiFi.softAPIP().toString() : String("none")));
}

static void printBehaviorList() {
  static const char* names[] = {
    "neutral","idle_watch","sleepy","sleepy_closed","full_sleep","alert",
    "happy","happy_big","proud","curious","curious_right","suspicious_left",
    "suspicious_right","angry","angry_left","angry_right","sad","shy",
    "surprised","bored","listening","talking_soft","talking_wide","wink_left",
    "wink_right","human_found","search_left","search_right","track_center","fear",
    "sleepy_watch","excited","proud_left","proud_right","sleepy_listen","yes_soft",
    "no_soft","dog_like","calm_soft","gentle_focus","comfort","thinking",
    "sleepy_open","skeptical_soft","tender","calm_watch"
  };
  Serial.println("BEHAVIORS:");
  for (const char* n : names) { Serial.print("- "); Serial.println(n); }
  Serial.println("END");
}

static void saveWiFiCredentials(const String &ssid, const String &pass) {
  prefs.begin("koraeyes", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();
  Serial.println("OK WIFI SAVED - restarting");
  delay(250);
  ESP.restart();
}

static void handleSerialLine(String line) {
  line.trim();
  if (!line.length()) return;

  String upper = line;
  upper.toUpperCase();

  if (upper.startsWith("WIFI ")) {
    String arg = line.substring(5);
    int sep = arg.indexOf('|');
    if (sep <= 0) { Serial.println("ERR WIFI format: WIFI ssid|password"); return; }
    String ssid = arg.substring(0, sep);
    String pass = arg.substring(sep + 1);
    ssid.trim();
    saveWiFiCredentials(ssid, pass);
    return;
  }

  if (upper.startsWith("EYESCFG ")) {
    handleEyesCfg(trimCopy(line.substring(8)));
    return;
  }

  if (upper == "STATUS") { printStatus(); return; }
  if (upper == "LIST") { printBehaviorList(); return; }

  lastInteractionMs = millis();
  autoSlept = false;
  triggerEarPerk();

  if (upper == "WAKE" || upper == "OPEN") {
    wakeFromPi(8000, "idle_watch"); Serial.println("OK WAKE"); return;
  }
  if (upper.startsWith("WAKE ")) {
    uint32_t ms = (uint32_t)clampI(line.substring(5).toInt(), 1000, 600000);
    wakeFromPi(ms, "idle_watch"); Serial.println("OK WAKE " + String(ms)); return;
  }
  if (upper == "SLEEP" || upper == "CLOSE") {
    enterPiSleep(true); Serial.println("OK SLEEP"); return;
  }
  if (upper.startsWith("BLINKPROFILE ")) {
    Serial.println("OK BLINKPROFILE " + trimCopy(line.substring(13))); return;
  }
  if (upper == "M" || upper == ".") {
    if (quietMode) wakeFromPi(8000, "idle_watch");
    startBlink(180); Serial.println("OK BLINK"); return;
  }
  if (upper == "U") { handlePersonFound(); Serial.println("OK PERSON_FOUND"); return; }
  if (upper == "N") { handlePersonLost(); Serial.println("OK PERSON_LOST"); return; }

  if (upper == "QUIET ON") { setQuietMode(true, true); Serial.println("OK QUIET ON"); return; }
  if (upper == "QUIET OFF") { setQuietMode(false, true); Serial.println("OK QUIET OFF"); return; }
  if (upper == "SOUL ON") { soulMode = true; scheduleNextSoul(); Serial.println("OK SOUL ON"); return; }
  if (upper == "SOUL OFF") { soulMode = false; Serial.println("OK SOUL OFF"); return; }
  if (upper == "SOUL NOW") { if (!quietMode) { nextSoulMs = 0; Serial.println("OK SOUL NOW"); } else Serial.println("ERR quiet_mode"); return; }
  if (upper == "TALK ON") { setTalkMode(true, "talking_soft"); Serial.println("OK TALK ON"); return; }
  if (upper == "TALK OFF") { setTalkMode(false, ""); Serial.println("OK TALK OFF"); return; }
  if (upper == "THINK ON") { setThinkMode(true); Serial.println("OK THINK ON"); return; }
  if (upper == "THINK OFF") { setThinkMode(false); Serial.println("OK THINK OFF"); return; }

  if (upper.startsWith("MOOD ")) {
    String mood = trimCopy(line.substring(5));
    if (isSleepMood(mood)) { enterPiSleep(true); Serial.println("OK MOOD " + mood); return; }
    quietMode = false; talkMode = false; thinkMode = false;
    if (applyMood(mood)) Serial.println("OK MOOD " + mood);
    else Serial.println("ERR mood_not_found");
    return;
  }

  if (upper.startsWith("TALK ")) {
    String arg = trimCopy(line.substring(5));
    if (arg.equalsIgnoreCase("ON")) { setTalkMode(true, "talking_soft"); Serial.println("OK TALK ON"); }
    else if (arg.equalsIgnoreCase("OFF")) { setTalkMode(false, ""); Serial.println("OK TALK OFF"); }
    else { setTalkMode(true, arg); Serial.println("OK TALK " + arg); }
    return;
  }

  if (upper.startsWith("EARL ")) {
    earLTarget = clampF((float)line.substring(5).toInt(), -35, 35);
    Serial.println("OK EARL " + String((int)earLTarget)); return;
  }
  if (upper.startsWith("EARR ")) {
    earRTarget = clampF((float)line.substring(5).toInt(), -35, 35);
    Serial.println("OK EARR " + String((int)earRTarget)); return;
  }

  if (line.length() == 1) {
    char c = line.charAt(0);
    switch (c) {
      case 'h': applyMood("neutral"); Serial.println("OK neutral"); return;
      case 'H': applyMood("happy"); Serial.println("OK happy"); return;
      case 'G': applyMood("curious"); Serial.println("OK curious"); return;
      case 'T': applyMood("sleepy_watch"); Serial.println("OK sleepy_watch"); return;
      case '!': applyMood("surprised"); Serial.println("OK surprised"); return;
      case 'S': applyMood("happy_big"); Serial.println("OK happy_big"); return;
      case 'A': applyMood("angry"); Serial.println("OK angry"); return;
      case 'Q': applyMood("curious_right"); Serial.println("OK curious_right"); return;
      case 'B': applyMood("bored"); Serial.println("OK bored"); return;
      case 'O': applyMood("alert"); Serial.println("OK alert"); return;
      case 'Z': applyMood("sleepy_closed"); Serial.println("OK sleepy_closed"); return;
      case 'C': applyMood("calm_soft"); Serial.println("OK calm_soft"); return;
      case 'F': applyMood("gentle_focus"); Serial.println("OK gentle_focus"); return;
      case 'K': applyMood("thinking"); Serial.println("OK thinking"); return;
      case 'Y': applyMood("tender"); Serial.println("OK tender"); return;
      case '[': applyMood("search_left"); Serial.println("OK search_left"); return;
      case ']': applyMood("search_right"); Serial.println("OK search_right"); return;
      case '=': applyMood("track_center"); Serial.println("OK track_center"); return;
      default: break;
    }
  }

  Serial.println("ERR unknown_cmd");
}

static void updateSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      handleSerialLine(serialLine);
      serialLine = "";
      continue;
    }
    serialLine += c;
    if (serialLine.length() > 180) serialLine = "";
  }
}

// ============================================================
// HTTP API + AP setup page
// ============================================================
static void sendJsonOk() {
  server.send(200, "application/json", "{\"ok\":true}");
}

static int argInt(const char* name, int fallback = 0) {
  if (!server.hasArg(name)) return fallback;
  return server.arg(name).toInt();
}

static void handleState() {
  String json = "{\"ok\":true,\"mode\":\"tft\",\"mood\":\"" + currentMood +
                "\",\"talk\":" + String(talkMode ? "true" : "false") +
                ",\"think\":" + String(thinkMode ? "true" : "false") +
                ",\"quiet\":" + String(quietMode ? "true" : "false") +
                ",\"soul\":" + String(soulMode ? "true" : "false") +
                ",\"person\":" + String(personDetected ? "true" : "false") +
                ",\"wakeTimeout\":" + String(wakeTimeoutMs) +
                ",\"earsL\":" + String((int)earLNow) +
                ",\"earsR\":" + String((int)earRNow) +
                ",\"uptime\":" + String((unsigned long)(millis() / 1000)) +
                ",\"ap\":" + String(apMode ? "true" : "false") +
                ",\"ip\":\"" + (WiFi.isConnected() ? WiFi.localIP().toString() : String(apMode ? WiFi.softAPIP().toString() : String("none"))) + "\"}";
  server.send(200, "application/json", json);
}

static void handleApRoot() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssid = trimCopy(server.arg("ssid"));
    String pass = server.arg("pass");
    if (ssid.length()) {
      server.send(200, "text/html", "<meta charset='utf-8'><body style='font-family:sans-serif;background:#0a0e14;color:#dfe;padding:2em'>"
        "<h2>KORA EYES - ZAPISANO / SAVED</h2><p>Restarting... / Restartuje sie...</p></body>");
      saveWiFiCredentials(ssid, pass);
      return;
    }
  }
  server.send(200, "text/html",
    "<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<body style='font-family:sans-serif;background:#0a0e14;color:#dfe;padding:2em'>"
    "<h2>KORA EYES SETUP</h2>"
    "<form method='GET' action='/'>"
    "<p>SSID:<br><input name='ssid' style='width:100%;padding:8px;font-size:16px'></p>"
    "<p>HASLO / PASSWORD:<br><input name='pass' type='password' style='width:100%;padding:8px;font-size:16px'></p>"
    "<p><button type='submit' style='padding:12px 24px;font-size:16px;background:#23e1ff;border:0;border-radius:8px'>SAVE + RESTART</button></p>"
    "</form></body>");
}

static void handleRoot() {
  if (apMode) { handleApRoot(); return; }
  server.send(200, "text/plain", "KORA TFT EYES V7.4 CRT\n/api/state\nOTA: enabled (pass [configured privately])\n");
}

static void handlePreview() {
  lastInteractionMs = millis();
  autoSlept = false;
  triggerEarPerk();
  if (server.hasArg("s0")) faceTarget.lookX = clampF((float)argInt("s0") / 40.0f, -1.0f, 1.0f);
  if (server.hasArg("s1")) faceTarget.lookY = clampF((float)argInt("s1") / 40.0f, -1.0f, 1.0f);

  if (server.hasArg("s2") || server.hasArg("s3")) {
    int a = abs(argInt("s2")); int b = abs(argInt("s3"));
    faceTarget.openL = clampF(1.0f - ((float)(a + b) / 2.0f) / 125.0f, 0.05f, 1.0f);
  }
  if (server.hasArg("s4") || server.hasArg("s5")) {
    int a = abs(argInt("s4")); int b = abs(argInt("s5"));
    faceTarget.openR = clampF(1.0f - ((float)(a + b) / 2.0f) / 125.0f, 0.05f, 1.0f);
  }

  if (server.hasArg("s6") || server.hasArg("s7")) {
    float l = ((float)argInt("s6") + (float)argInt("s7")) * 0.5f;
    earLTarget = clampF(l, -35, 35);
  }
  if (server.hasArg("s8") || server.hasArg("s9")) {
    float r = ((float)argInt("s8") + (float)argInt("s9")) * 0.5f;
    earRTarget = clampF(r, -35, 35);
  }

  quietMode = false;
  lastInteractionMs = millis();
  sendJsonOk();
}

static void setupHttpApi() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/preview", HTTP_GET, handlePreview);
  server.on("/api/blink", HTTP_GET, [](){ lastInteractionMs = millis(); triggerEarPerk(); startBlink(180); sendJsonOk(); });
  server.on("/api/blink", HTTP_POST, [](){ lastInteractionMs = millis(); triggerEarPerk(); startBlink(180); sendJsonOk(); });

  server.on("/api/playBehavior", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    String name = trimCopy(server.arg("name"));
    bool ok = applyMood(name);
    server.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
  });
  server.on("/api/mood", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    String name = trimCopy(server.arg("name"));
    if (isSleepMood(name)) { enterPiSleep(true); sendJsonOk(); return; }
    quietMode = false; talkMode = false; thinkMode = false;
    bool ok = applyMood(name);
    server.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
  });
  server.on("/api/person", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    bool on = server.arg("on") == "1";
    if (on) handlePersonFound(); else handlePersonLost();
    sendJsonOk();
  });
  server.on("/api/talk", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    bool on = server.arg("on") == "1";
    String name = trimCopy(server.arg("name"));
    setTalkMode(on, name.length() ? name : "talking_soft");
    sendJsonOk();
  });
  server.on("/api/think", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    setThinkMode(server.arg("on") == "1"); sendJsonOk();
  });
  server.on("/api/quiet", HTTP_GET, [](){
    setQuietMode(server.arg("on") == "1", true); sendJsonOk();
  });
  server.on("/api/soul", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    if (server.hasArg("on")) soulMode = server.arg("on") == "1";
    if (server.hasArg("now") || server.arg("cmd") == "now") nextSoulMs = 0;
    sendJsonOk();
  });
  server.on("/api/blinkProfile", HTTP_GET, [](){ sendJsonOk(); });

  auto wakeHandler = [](){
    uint32_t ms = server.hasArg("ms") ? (uint32_t)clampI(server.arg("ms").toInt(), 1000, 600000) : 8000;
    String mood = server.hasArg("mood") ? trimCopy(server.arg("mood")) : String("idle_watch");
    wakeFromPi(ms, mood); sendJsonOk();
  };
  server.on("/api/wake", HTTP_GET, wakeHandler);
  server.on("/api/wake", HTTP_POST, wakeHandler);
  server.on("/api/openEyes", HTTP_GET, wakeHandler);
  server.on("/api/openEyes", HTTP_POST, wakeHandler);

  auto sleepHandler = [](){ enterPiSleep(true); sendJsonOk(); };
  server.on("/api/sleep", HTTP_GET, sleepHandler);
  server.on("/api/sleep", HTTP_POST, sleepHandler);
  server.on("/api/closeEyes", HTTP_GET, sleepHandler);
  server.on("/api/closeEyes", HTTP_POST, sleepHandler);

  server.on("/api/setNeutral", HTTP_GET, [](){ lastInteractionMs = millis(); triggerEarPerk(); applyMood("neutral"); sendJsonOk(); });
  server.on("/api/saveBehavior", HTTP_GET, [](){ server.send(200, "application/json", "{\"ok\":false,\"reason\":\"fixed_tft_behaviors\"}"); });
  server.on("/api/deleteBehavior", HTTP_GET, [](){ server.send(200, "application/json", "{\"ok\":false,\"reason\":\"fixed_tft_behaviors\"}"); });
  server.on("/api/factoryReset", HTTP_GET, [](){
    lastInteractionMs = millis(); autoSlept = false;
    triggerEarPerk();
    applyMood("idle_watch"); soulMode = true; talkMode = false; thinkMode = false; quietMode = false; sendJsonOk();
  });

  server.onNotFound([](){ server.send(404, "application/json", "{\"ok\":false,\"error\":\"not_found\"}"); });
  server.begin();
}

static void setupOTA() {
  if (strlen(OTA_PASSWORD) < 12) {
    Serial.println("OTA DISABLED - set your own OTA_PASSWORD (12+ characters)");
    return;
  }
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    Serial.println("OTA START");
    esp_task_wdt_reset();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    esp_task_wdt_reset();
    static int lastPct = -1;
    int pct = (int)(progress * 100 / total);
    if (pct != lastPct && pct % 10 == 0) { lastPct = pct; Serial.printf("OTA %d%%\n", pct); }
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA END - restarting");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA ERROR %d\n", (int)error);
  });
  ArduinoOTA.begin();
  Serial.println("OTA READY (private password configured)");
}

static void startApMode() {
  if (strlen(AP_PASS) < 12 || strlen(AP_PASS) > 63) {
    Serial.println("AP DISABLED - set your own AP_PASS (12-63 characters); USB serial ready");
    return;
  }
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  setupHttpApi();
  Serial.println("AP MODE: " + String(AP_SSID));
  Serial.println("SETUP PAGE: http://" + WiFi.softAPIP().toString() + "/");
}

static void initWiFi() {
  if (!ENABLE_WIFI) {
    WiFi.mode(WIFI_OFF);
    Serial.println("WIFI DISABLED - USB serial control ready");
    return;
  }
  prefs.begin("koraeyes", true);
  wifiSsid = prefs.getString("ssid", "");
  wifiPass = prefs.getString("pass", "");
  prefs.end();

  if (!wifiSsid.length()) {
    Serial.println("WIFI NOT CONFIGURED - starting AP setup mode");
    startApMode();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());

  Serial.print("WIFI connecting");
  uint32_t start = millis();
  while (!WiFi.isConnected() && millis() - start < 12000) {
    delay(250);
    Serial.print('.');
    esp_task_wdt_reset();
  }
  Serial.println();

  if (!WiFi.isConnected()) {
    Serial.println("WIFI FAILED - AP setup mode + serial control still works");
    startApMode();
    return;
  }

  wifiReady = true;
  Serial.print("WIFI OK IP="); Serial.println(WiFi.localIP());
  if (MDNS.begin("kora-eyes")) Serial.println("MDNS: http://kora-eyes.local/");
  setupHttpApi();
  setupOTA();
}

// ============================================================
// Setup / loop
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(400);

  randomSeed((uint32_t)esp_random());

  for (int f = 0; f < FIB_N; f++) {
    fibA[f] = (float)f * (2.0f * PI / (float)FIB_N) + (float)random(-6, 7) * 0.02f;
    fibLen[f] = 0.55f + (float)random(0, 45) / 100.0f;
  }

  SPI.begin(PIN_TFT_SCLK, -1, PIN_TFT_MOSI, PIN_TFT_CS);
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  loadCfg();

  Serial.printf("HEAP start: %u\n", (unsigned)ESP.getFreeHeap());
  const int sizes[][2] = { {160, 240}, {152, 224}, {144, 208} };
  for (int i = 0; i < 3; i++) {
    eyeCanvas = new GFXcanvas16(sizes[i][0], sizes[i][1]);
    if (eyeCanvas && eyeCanvas->getBuffer()) {
      EYEW = sizes[i][0];
      EYEH = sizes[i][1];
      break;
    }
    delete eyeCanvas;
    eyeCanvas = nullptr;
  }
  if (!eyeCanvas || !eyeCanvas->getBuffer()) {
    EYEW = 128; EYEH = 180;
    eyeCanvas = new GFXcanvas16(EYEW, EYEH);
  }
  if (!eyeCanvas || !eyeCanvas->getBuffer()) {
    Serial.println("ERR EYE BUFFER ALLOCATION - eyes disabled, serial still works");
  } else {
    Serial.printf("EYE CANVAS %dx%d OK, heap: %u\n", EYEW, EYEH, (unsigned)ESP.getFreeHeap());
  }

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  earL.setPeriodHertz(50);
  earR.setPeriodHertz(50);
  earL.attach(PIN_EAR_L, 500, 2400);
  earR.attach(PIN_EAR_R, 500, 2400);
  earLNow = earRNow = 0;
  setEarLogicalTargets(0, 0);
  updateEars();

  // watchdog 10 s (ESP32 core 3.x / IDF 5.x API)
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = 10000,
    .idle_core_mask = 0,
    .trigger_panic = true,
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL);

  // SLEEP-FIRST BOOT / BOOT W STANIE SNU
  enterPiSleep(false);
  scheduleNextSoul();

  initWiFi();

  Serial.println("READY KORA_TFT_EYES_V7_4_PUBLIC CRT (sleep-first, OTA, watchdog)");
  Serial.println("WAKE <ms> / SLEEP / STATUS / LIST / M / U / N");
  Serial.println("MOOD ... | TALK ... | EYESCFG blinkmin=.. blinkmax=..");
  Serial.println("OTA: ArduinoOTA, host kora-eyes, password [configured privately]");
}

void loop() {
  esp_task_wdt_reset();

  // WiFi auto-reconnect
  if (wifiReady && !apMode && !WiFi.isConnected() &&
      millis() - lastReconnectMs > 10000) {
    lastReconnectMs = millis();
    Serial.println("WIFI LOST - reconnecting");
    WiFi.disconnect();
    WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());
  }

  if (wifiReady || apMode) {
    server.handleClient();
    if (wifiReady) ArduinoOTA.handle();
  }

  // heartbeat to Pi every 5 s
  if (millis() - lastHbMs > 5000) {
    lastHbMs = millis();
    Serial.println("HB state=" + String(quietMode ? "sleep" : "awake") +
                   " mood=" + currentMood +
                   " heap=" + String((int)ESP.getFreeHeap()) +
                   " ms=" + String((unsigned long)millis()));
  }

  updateSerial();
  updateAutonomy();
  updateFaceMotion();
  updateEars();
  drawFace();

  delay(1);
}