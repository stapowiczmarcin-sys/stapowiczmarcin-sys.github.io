/*
  KORA QUADRUPED — FREE COMMUNITY RELEASE / DARMOWE WYDANIE SPOŁECZNOŚCIOWE

  Project integration, web calibration, controls, added movements and testing:
  Marcin Stąpowicz — https://stapowiczmarcin-sys.github.io/

  The inverse-kinematics and basic quadruped-motion foundation originated in
  the SunFounder four-legged Crawling Robot example (2015, panerqiang@sunfounder.com).
  This release has been extensively modified for Kora's ESP32 + PCA9685 build.

  IMPORTANT / WAŻNE:
  - Set your own Wi-Fi name and password below before uploading.
  - Set ESP-NOW MAC addresses only if you use the optional remote/head modules.
  - Calibrate with the robot lifted and servos unloaded before floor testing.
  - This is experimental hardware code. Check wiring, voltage and servo limits.
*/

// ======================= VEGA SPIDER – ESP-NOW + BT + UART + WEB + PCA9685 + 2x VL53L1X + OLED + DISCOVER + NECK/EYES RECEIVER / ODBIORNIK =======================
// Board: ESP32 WROOM-32 / Płytka: ESP32 WROOM-32
//
// Version without ultrasonic sensors / Wersja bez ultradźwięków:
// - Obstacle avoidance uses 2x VL53L1X ToF sensors (front-left / front-right) sharing address 0x29 via XSHUT. / Omijanie przeszkód używa 2x VL53L1X.
//
// Added / maintained features / Dodane / utrzymane funkcje:
// 1) Smart avoid: selects the direction with more free space (L vs R). / Wybiera kierunek z większą ilością miejsca.
// 2) Brief braking before an avoidance manoeuvre. / Krótkie hamowanie przed omijaniem.
// 3) OLED 128x64 (I2C): IP, ToF mode, distances, status, speed, servo state and alerts. / IP, tryb ToF, dystanse, status, prędkość, serwa i alerty.
// 4) Servo calibration saved to NVS. / Kalibracja serw zapisywana w NVS.
// 5) Automatic moves (sequences), recording and slots. / Auto-ruchy, nagrywanie i sloty.
// 6) PERSISTENT BODY HEIGHT (Z)
//
// NEW / NOWOŚĆ:
// 7) DISCOVER MODE (ON/OFF):
//    - When enabled, the spider walks for about 4 minutes while avoiding obstacles with ToF. / Po włączeniu pająk chodzi około 4 min i omija przeszkody.
//    - After the timer expires it sleeps and snores (placeholder for a future MP3). / Po czasie zasypia i chrapie.
//    - Web endpoints / Endpointy WWW: /discover/get, /discover/set?on=1|0
//    - UI buttons added. / Dodano przyciski w UI.
//    - Command 'd' can be sent over BT, UART, Serial, ESP-NOW or the web panel. / Komendę 'd' można wysłać każdym z tych interfejsów.
//
// 8) ESP-NOW LINK TO THE NECK + EYES + EARS MODULE / LINK DO MODUŁU SZYJA + OCZY + USZY:
//    - The spider sends commands to keep the head and eyes awake while moving or exploring. / Pająk wybudza głowę i oczy podczas ruchu.
//    - Head-module command map / Mapowanie komend modułu głowy:
//
//       L / R / P / F   – neck left / right / forward; eyes and ears react / szyja lewo / prawo / przód; oczy i uszy reagują
//       C               – centre the neck / szyja wraca na środek
//       O               – eyes wide open / oczy szeroko otwarte
//       Z               – eyes closed / oczy zamknięte
//       M               – one non-blocking blink / jedno mrugnięcie bez blokowania
//       S               – HAPPY
//       A               – ANGRY
//       Q               – CURIOUS
//       B               – BORED
//
// I2C shared by PCA9685, ToF and OLED / wspólne dla PCA9685, ToF i OLED:
// - SDA=21, SCL=22 (WROOM-32 defaults / domyślne dla WROOM-32)
//
// XSHUT:
// - XSHUT_L_PIN = 26
// - XSHUT_R_PIN = 27
//
// WiFi:
// - SSID: XXX
// - PASS: XXX
//
// =====================================================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <Preferences.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <BluetoothSerial.h>
#include <Adafruit_VL53L1X.h>

// OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------- BT Classic --------------------
BluetoothSerial SerialBT;

// -------------------- Home WiFi / domowa sieć WiFi --------------------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// WiFi connection control and repeated web-panel address display. / Kontrola WiFi i ponowne wyświetlanie adresu panelu.
// The address is shown in Serial Monitor not only at startup, / Adres pojawia się w Monitorze portu nie tylko przy starcie,
// but also after every reconnection to the router. / ale również po każdym ponownym połączeniu z routerem.
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000UL;
static unsigned long lastWifiRetryMs = 0;
static bool wifiWasConnected = false;

WebServer server(80);

// Separate NVS namespaces / Osobne przestrzenie NVS
Preferences prefsCal;   // Servo calibration / Kalibracja serw
Preferences prefsSeq;   // Sequences and auto-move / Sekwencje i auto-ruch

// -------------------- PINS / PINY --------------------
// UART2
static const int UART2_RX = 15;
static const int UART2_TX = 5;

// XSHUT
static const int XSHUT_L_PIN = 26;  // Front-left ToF / ToF przód-lewo
static const int XSHUT_R_PIN = 27;  // Front-right ToF / ToF przód-prawo

// -------------------- PCA9685 --------------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

// Leg/joint to PCA9685 channel map / Mapowanie nóg i przegubów na kanały PCA9685
const uint8_t servo_channel[4][3] = {
  {0, 1, 2},    // 0 – front right / przednia prawa
  {3, 4, 5},    // 1 – rear right / tylna prawa
  {6, 7, 8},    // 2 – front left / przednia lewa
  {9, 10, 11}   // 3 – rear left / tylna lewa
};
/*
const uint8_t servo_channel[4][3] = {
  {0, 1, 2},     // 0 – front right / przednia prawa
  {3, 4, 5},     // 1 – rear right / tylna prawa
  {13, 14, 8},   // 2 – front left / przednia lewa
  {9, 10, 11}    // 3 – rear left / tylna lewa
};
*/


// Pulse range for 0–180° / Zakres impulsów dla 0–180°
const uint16_t SERVO_MIN_TICK = 150;
const uint16_t SERVO_MAX_TICK = 600;

bool pcaReady = false;

// -------------------- CALIBRATION (servo offsets) / KALIBRACJA --------------------
int calValues[4][3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

// -------------------- Motion and kinematics parameters / Parametry ruchu i kinematyki --------------------
const float length_a = 55;
const float length_b = 77.5;
const float length_c = 27.5;
const float length_side = 71;

const float z_absolute = -28;

// Base values / Wartości bazowe
const float z_default_base = -50;
const float z_up_base      = -30;
const float z_boot_base    = z_absolute;
const float z_startup_relaxed = -58.0;

const float x_default = 62, x_offset = 0;
const float y_start = 0, y_step = 40;

// Current IK targets / Aktualne cele IK
volatile float site_now[4][3];
volatile float site_expect[4][3];
float temp_speed[4][3];

// Speeds / Prędkości
float move_speed;
float speed_multiple = 1.0;
const float spot_turn_speed = 4;
const float leg_move_speed  = 8;
const float body_move_speed = 3;
const float stand_seat_speed = 1;
const float max_speed_multiple = 2.0;
const float min_speed_multiple = 0.1;
const float speed_increment    = 0.1;
int   rest_counter;

const float KEEP = 255;
const float pi   = 3.1415926;

// Turning geometry / Geometria skrętów
const float temp_a = sqrt(pow(2 * x_default + length_side, 2) + pow(y_step, 2));
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt(pow(2 * x_default + length_side, 2) + pow(2 * y_start + y_step + length_side, 2));
const float temp_alpha = acos((pow(temp_a, 2) + pow(temp_b, 2) - pow(temp_c, 2)) / 2 / temp_a / temp_b);
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin(temp_alpha) - turn_y1 - length_side;

const float max_speed = 5.0;

// Body-height Z limits / Ograniczenia wysokości ciała Z
const float body_z_min = -80.0;
const float body_z_max = -25.0;

// Step used by body-motion arrow buttons / Krok strzałek ruchu korpusu
const int BODY_SHIFT_STEP = 10;

bool  isBlueTooth = false;
bool  isSerialControl = false;
bool  calibrationMode = false;

unsigned char curStatus = 's';

float g_alpha, g_beta, g_gamma;
TaskHandle_t Task_HandleOne = NULL;

// -------------------- ESP-NOW transmitter MAC / MAC NADAJNIKA --------------------
uint8_t sender_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Replace if ESP-NOW is used / Zmień, jeśli używasz ESP-NOW

// -------------------- ESP-NOW neck/eyes module MAC / MAC MODUŁU SZYJA/OCZY --------------------
uint8_t neckEyesAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Optional receiver / Opcjonalny odbiornik

bool          espNowOk       = false;
volatile bool hasEspNowCmd   = false;
volatile char espNowCmd      = 0;

// -------------------- Automatic moves / Auto-ruchy --------------------
String        autoSeq        = "";
unsigned long autoIntervalMs = 8000;
bool          autoEnabled    = false;
int           autoIndex      = 0;
unsigned long lastAutoMillis = 0;

// -------------------- Web sequence-recording buffer / Bufor nagrywania sekwencji z WWW --------------------
String recordSeq = "";
bool   recordOn  = false;

// Sequence slots / Sloty sekwencji
const int MAX_REC_SLOTS = 5;
String    recSlotSeq[MAX_REC_SLOTS];
String    recSlotName[MAX_REC_SLOTS];

// -------------------- PERSISTENT BODY HEIGHT --------------------
float bodyZCurrent   = z_default_base;
float bodyZUpCurrent = z_up_base;
const float bodyLiftDelta = (z_up_base - z_default_base);

static float clampBodyZ(float z) {
  if (z > body_z_max) z = body_z_max;
  if (z < body_z_min) z = body_z_min;
  return z;
}
static void updateBodyHeight(float newBodyZ) {
  bodyZCurrent = clampBodyZ(newBodyZ);
  float up = bodyZCurrent + bodyLiftDelta;
  bodyZUpCurrent = clampBodyZ(up);
}

// -------------------- SERVO POWER SAVING (IDLE) / OSZCZĘDZANIE SERW --------------------
const unsigned long IDLE_TIMEOUT_MS = 3000000UL; // 30 s
unsigned long lastActivityMs = 0;
bool idleDetached = false;

static void markActivity() { lastActivityMs = millis(); }

// -------------------- DISCOVER + SLEEP + SNORE --------------------
static const unsigned long DISCOVER_DURATION_MS = 4UL * 60UL * 1000UL; // 4 min
static bool discoverEnabled  = false;
static bool discoverRunning  = false;
static unsigned long discoverStartMs = 0;
static unsigned long lastDiscoverActionMs = 0;
static const unsigned long DISCOVER_COOLDOWN_MS = 60;

static bool sleepActive = false;
static unsigned long lastSnoreMs = 0;
static bool snoreFlip = false;

static void enterSleepMode();
static void handleSleepSnore();
static void startDiscover();
static void stopDiscover(bool naturalSleep);
static void handleDiscoverMode();

// -------------------- NECK/EYES keep-alive during motion / podtrzymanie podczas ruchu --------------------
static unsigned long lastNeckTxMs = 0;
static const unsigned long NECK_TX_GAP_MS = 18; // Very short gap for 2-3 consecutive commands / Krótki odstęp dla 2-3 komend
static unsigned long lastNeckKeepAliveMs = 0;
static const unsigned long NECK_KEEPALIVE_MS = 1200;  // Keep-alive about every 1.2 s / Podtrzymanie co około 1,2 s

static inline void neckSend(char cmd){
  if(!espNowOk) return;
  unsigned long now = millis();
  if(now - lastNeckTxMs < NECK_TX_GAP_MS) return;
  lastNeckTxMs = now;
  esp_now_send(neckEyesAddress, (const uint8_t*)&cmd, 1);
}

// A short command packet sent only on mode changes (minimal controlled delay). / Krótki pakiet komend tylko przy zmianach trybu.
static void neckSendSeq(const char* s){
  if(!s) return;
  for(size_t i=0; s[i]; i++){
    char c = s[i];
    if(c==' ' || c==',' || c=='\n' || c=='\r' || c=='\t') continue;
    esp_now_send(neckEyesAddress, (const uint8_t*)&c, 1);
    delay(12);
  }
}

// Spider command to head-gesture map / Mapowanie komend pająka na gesty głowy
static void neckMirrorRobotCmd(char ch){
  switch(ch){
    case 'f': neckSend('P'); neckSend('O'); neckSend('Q'); break;
    case 'b': neckSend('P'); neckSend('O'); neckSend('B'); break;
    case 'l': neckSend('L'); neckSend('O'); neckSend('Q'); break;
    case 'r': neckSend('R'); neckSend('O'); neckSend('Q'); break;
    case 'L': neckSend('L'); neckSend('O'); break;
    case 'R': neckSend('R'); neckSend('O'); break;

    case 'm': neckSend('S'); neckSend('O'); break; // happy dance -> HAPPY
    case '1': neckSend('Q'); neckSend('M'); break;
    case '2': neckSend('S'); neckSend('M'); break;
    case '3': neckSend('S'); neckSend('O'); break;
    case '4': neckSend('S'); neckSend('O'); break;
    case '5': neckSend('B'); break;

    case 's': /* Do not sleep on a normal STOP / Nie usypiaj przy zwykłym STOP */ break;
    case 'E': neckSend('O'); neckSend('P'); break; // High stand: eyes open / Wysoka pozycja: oczy otwarte
    case 'F': /* Do not move the eyes / Nie ruszaj oczu */ break;
     case 'H': curStatus = 'H'; strafe_left(1); break;   // Strafe LEFT - w lewo
    case 'K': curStatus = 'K'; strafe_right(1); break;  // Strafe RIGHT - w prawo
    default: break;
  }
}

// Head keep-alive during Discover and general movement / Podtrzymanie głowy podczas eksploracji i ruchu
static void neckKeepAliveTick(){
  if(sleepActive) return;

  bool moving = (curStatus=='f' || curStatus=='b' || curStatus=='l' || curStatus=='r' || curStatus=='d');
  if(!discoverEnabled && !moving) return;

  unsigned long now = millis();
  if(now - lastNeckKeepAliveMs < NECK_KEEPALIVE_MS) return;
  lastNeckKeepAliveMs = now;

  // Slight randomness to reduce robotic repetition / Lekka losowość, żeby ruch nie był mechaniczny
  int r = random(100);
  if(r < 12)      neckSend('M');       // mrug
  else if(r < 28) neckSend('Q');       // ciekawski
  else            neckSend('O');       // Eyes open / Oczy otwarte

  // Occasional subtle neck turn / Czasem subtelny zwrot szyi
  int d = random(3);
  if(d==0) neckSend('L');
  else if(d==1) neckSend('R');
  else neckSend('P');
}

// -------------------- DELAY SCALING / SKALOWANIE OPÓŹNIEŃ --------------------
static void scaledDelay(uint32_t ms) {
  float m = speed_multiple;
  if (m < 0.05f) m = 0.05f;
  uint32_t adj = (uint32_t)lround((double)ms / (double)m);
  if (adj < 1) adj = 1;
  delay(adj);
}

// -------------------- ANGLE TO PCA9685 PULSE CONVERSION / KONWERSJA KĄTA NA IMPULS --------------------
static uint16_t angleToTick(float angle) {
  if (angle < 0)   angle = 0;
  if (angle > 180) angle = 180;
  float ratio = angle / 180.0f;
  return (uint16_t)(SERVO_MIN_TICK + (SERVO_MAX_TICK - SERVO_MIN_TICK) * ratio);
}
void setServoAngle(int leg, int joint, float angle) {
  if (!pcaReady) return;
  if (leg < 0 || leg > 3 || joint < 0 || joint > 2) return;
  uint8_t ch = servo_channel[leg][joint];
  uint16_t tick = angleToTick(angle);
  pwm.setPWM(ch, 0, tick);
}

// -------------------- SAVE / RESTORE POSITION / ZAPIS I PRZYWRACANIE POZYCJI --------------------
void savePose(float pose[4][3]) {
  for (int leg = 0; leg < 4; leg++) {
    for (int joint = 0; joint < 3; joint++) {
      pose[leg][joint] = site_now[leg][joint];
    }
  }
}


void cartesian_to_polar(volatile float &a, volatile float &b, volatile float &c, float x, float y, float z);
void polar_to_servo(int leg, float alpha, float beta, float gamma);

static void writeCurrentIKPoseImmediate() {
  if (!pcaReady) return;
  for (int leg = 0; leg < 4; leg++) {
    cartesian_to_polar(g_alpha, g_beta, g_gamma, site_now[leg][0], site_now[leg][1], site_now[leg][2]);
    polar_to_servo(leg, g_alpha, g_beta, g_gamma);
  }
}

// -------------------- CORE PROTOTYPES / PROTOTYPY --------------------
void servo_attach();
void servo_detach();
void servo_service(void *parm);

void uartOper();
void bluetoothOper();
void controlOperations(char ch);

void cartesian_to_polar(volatile float &a, volatile float &b, volatile float &c, float x, float y, float z);
void polar_to_servo(int leg, float alpha, float beta, float gamma);

void set_site(int leg, float x, float y, float z);
void wait_reach(int leg);
void wait_all_reach(void);

void custom_set_site(int leg, float x, float y, float z);
void custom_wait_all_reach();

void sit(void);
void stand(void);
void safeStopNow(const char* reason = "STOP");

void body_left(int i);
void body_right(int i);
void body_front(int i);
void body_back(int i);
void reset_body_pose();

void body_up();
void body_down();

void step_forward(unsigned int step);
void step_back(unsigned int step);
void turn_left(unsigned int step);
void turn_right(unsigned int step);

void hand_wave(int i);
void hand_shake(int i);
void sway_body();

void move_right(int steps);
void move_left(int steps);

void rear_legs_wave(int steps);
void rear_leg_wave2(int i);

void jump_forward();
void spin_on_leg(int leg_to_stay, int steps);
void dance_step(int steps);

void squat_back_legs();
void squat_back_legs_stop();
void squat_front_legs();

void sequence_legs_move();
void sequence_right_legs_move();
void sequence_left_legs_move();

void moonwalk(int steps);
void circle_walk(int steps);
void sidewalk(int steps);
void wave_walk(int steps);

// ESP-NOW
void initEspNow();
void onEspNowRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);
void reportDone();

// Calibration mode / Tryb kalibracji
void enterCalibrationMode();
void exitCalibrationMode();
void applyCalibrationPose();
void loadCalibration();
void saveCalibration();

// Web / WiFi / automatic moves / WWW / WiFi / auto-ruchy
void setupWiFi();
void handleWiFiStatus();
void printWiFiAddress();
void setupWebServer();
void handleRoot();
void handleCmd();
void handleCalGet();
void handleCalStep();
void handleCalMode();
void handleCalExport();
void handleCalReset();
void handleSeqGet();
void handleSeqSet();
void handleBodyStep();
void handleBodyShift();

// Web sequence recording and slots / Nagrywanie sekwencji z WWW i sloty
void recordEvent(const String &event);
void playRecordedSequence(const String &seq);
void handleRecStart();
void handleRecStop();
void handleRecClear();
void handleRecGet();
void handleRecPlay();
void handleRecSlots();
void handleRecSaveSlot();
void handleRecLoadSlot();
void handleRecPlaySlot();

// ToF
void handleToFGet();
void handleToFMode();

// Discover HTTP
void handleDiscoverGet();
void handleDiscoverSet();

// Tricks / Triki
void trick_bow_hello();
void trick_wave_combo();
void trick_double_jump();
void trick_happy_dance();
void trick_idle_breath();
void trick_hello_combo();
void trick_guard_scan();

// Sequence NVS / NVS sekwencji
void loadRecordedSlots();
void saveRecordedSlot(int idx);
void loadAutoFromNVS();
void saveAutoToNVS();

void initZero();
void handleIdlePowerSave();
void handleAutoSequence();

// -------------------- ToF: 2x VL53L1X via XSHUT / przez XSHUT --------------------
Adafruit_VL53L1X tofL;
Adafruit_VL53L1X tofR;

enum ToFSoftMode { TOF_STABLE = 0, TOF_FAST = 1 };
static ToFSoftMode tofMode = TOF_STABLE;

static float filtL = -1;
static float filtR = -1;
static int   lastRawL = -1;
static int   lastRawR = -1;

static const float EMA_ALPHA_STABLE = 0.25f;
static const float EMA_ALPHA_FAST   = 0.40f;

// Thresholds in mm / Progi w mm
static int TH_OBSTACLE_MM    = 280;  // Obstacle threshold for forward command / Próg przeszkody dla komendy naprzód
static int TH_APPROACH_MM    = 240;  // gdy stoi
static int TH_APPROACH_DELTA = 60;   // Rapid approach / Szybkie zbliżanie

static unsigned long lastToFReadMs = 0;
static unsigned long lastApproachActionMs = 0;
static const unsigned long APPROACH_COOLDOWN_MS = 2500;

static inline float emaAlpha(){
  return (tofMode == TOF_FAST) ? EMA_ALPHA_FAST : EMA_ALPHA_STABLE;
}

static bool i2cPing(uint8_t addr){
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

static inline void xshutAllOff(){
  digitalWrite(XSHUT_L_PIN, LOW);
  digitalWrite(XSHUT_R_PIN, LOW);
}
static inline void xshutLeftOnly(){
  digitalWrite(XSHUT_R_PIN, LOW);
  digitalWrite(XSHUT_L_PIN, HIGH);
}
static inline void xshutRightOnly(){
  digitalWrite(XSHUT_L_PIN, LOW);
  digitalWrite(XSHUT_R_PIN, HIGH);
}

// Read one sensor in mm; returns -1 on error. / Odczyt jednego czujnika w mm; -1 oznacza błąd.
static int readOneToFmm(Adafruit_VL53L1X &sensor, bool left){
  if(left) xshutLeftOnly(); else xshutRightOnly();

  delay(60);

  if(!i2cPing(0x29)){
    xshutAllOff();
    return -1;
  }

  if(!sensor.begin(0x29, &Wire, false)){
    xshutAllOff();
    return -1;
  }

  if(!sensor.startRanging()){
    xshutAllOff();
    return -1;
  }

  const unsigned long t0 = millis();
  while(!sensor.dataReady()){
    if(millis() - t0 > 160){
      sensor.stopRanging();
      xshutAllOff();
      return -1;
    }
    delay(1);
  }

  int16_t d = sensor.distance();
  sensor.clearInterrupt();
  sensor.stopRanging();
  xshutAllOff();

  if(d <= 0) return -1;
  return (int)d;
}

static void readToFPairFiltered(){
  int rawL = readOneToFmm(tofL, true);
  int rawR = readOneToFmm(tofR, false);

  lastRawL = rawL;
  lastRawR = rawR;

  float a = emaAlpha();

  if(rawL > 0){
    if(filtL < 0) filtL = rawL;
    else filtL = filtL + a * (rawL - filtL);
  }
  if(rawR > 0){
    if(filtR < 0) filtR = rawR;
    else filtR = filtR + a * (rawR - filtR);
  }
}

static int minToF(){
  bool okL = filtL > 0;
  bool okR = filtR > 0;
  if(okL && okR) return (int)min(filtL, filtR);
  if(okL) return (int)filtL;
  if(okR) return (int)filtR;
  return -1;
}

// Soft mode: change the read interval / Tryb soft: zmiana okresu odczytu
static inline uint16_t tofReadPeriodMs(){
  return (tofMode == TOF_FAST) ? 160 : 260;
}

// -------------------- OLED 128x64 --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledReady = false;

static String oledAlert = "";
static unsigned long lastOledMs = 0;
static const unsigned long OLED_PERIOD_MS = 250;

static void oledSetAlert(const String &msg){
  oledAlert = msg;
}

static void oledInit(){
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    oledReady = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("VEGA SPIDER");
    display.println("OLED OK");
    display.display();
  }else{
    oledReady = false;
  }
}

static void oledRender(){
  if(!oledReady) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Line 1 / Linia 1
  display.setCursor(0,0);
  display.print("Vega ");
  display.print(pcaReady ? "SERVO:ON" : "SERVO:OFF");
  if(discoverEnabled) display.print(" D");
  if(sleepActive)     display.print(" Zzz");

  // Line 2 / Linia 2
  display.setCursor(0,10);
  display.print("IP: ");
  if(WiFi.status() == WL_CONNECTED) display.print(WiFi.localIP());
  else display.print("no/brak WiFi");

  // Line 3 / Linia 3
  display.setCursor(0,20);
  display.print("ToF: ");
  display.print((tofMode == TOF_FAST) ? "FAST" : "STABLE");

  // Line 4 / Linia 4
  display.setCursor(0,30);
  display.print("L:");
  display.print((filtL > 0) ? String((int)filtL) : String("--"));
  display.print(" R:");
  display.print((filtR > 0) ? String((int)filtR) : String("--"));

  // Line 5 / Linia 5
  display.setCursor(0,40);
  display.print("St:");
  display.print((char)curStatus);
  display.print(" spd x");
  display.print(speed_multiple, 1);

  // Line 6 / Linia 6
  display.setCursor(0,52);
  if(oledAlert.length()){
    display.print(oledAlert);
  }else{
    display.print("OK");
  }

  display.display();
}

static void oledTick(){
  if(!oledReady) return;
  unsigned long now = millis();
  if(now - lastOledMs < OLED_PERIOD_MS) return;
  lastOledMs = now;
  oledRender();
}

// -------------------- Simple protection while stationary when something approaches / Zabezpieczenie podczas postoju --------------------
static void handleToFApproachWhileStanding(){
  if(curStatus != 's') return;
  int dmin = minToF();
  static int lastMin = -1;

  if(dmin > 0 && lastMin > 0){
    int delta = lastMin - dmin;
    bool approaching = (dmin < TH_APPROACH_MM) && (delta > TH_APPROACH_DELTA);

    if(approaching && millis() - lastApproachActionMs > APPROACH_COOLDOWN_MS){
      lastApproachActionMs = millis();
      oledSetAlert("APPROACH!");

      body_up();
      scaledDelay(120);

      step_back(5);
      scaledDelay(120);
      step_back(5);

      curStatus = 's';
      stand();
      markActivity();
    }
  }
  lastMin = dmin;
}

// -------------------- ToF: block forward command when obstructed / blokada komendy naprzód przy przeszkodzie --------------------
static bool isObstacleAheadNow(){
  readToFPairFiltered();
  int d = minToF();
  if(d > 0 && d < TH_OBSTACLE_MM) return true;
  return false;
}

// -------------------- Smart avoid --------------------
static int decideAvoidDir(){
  bool okL = filtL > 0;
  bool okR = filtR > 0;

  if(okL && okR){
    int L = (int)filtL;
    int R = (int)filtR;
    if(L > R + 25) return -1;
    if(R > L + 25) return 1;
    return 1;
  }
  if(okL) return -1;
  if(okR) return 1;
  return 0;
}

static void autoAvoidSmart(){
  stand();
  scaledDelay(60);

  readToFPairFiltered();

  int dir = decideAvoidDir();

  oledSetAlert("OBSTACLE!");
  Serial.print("[ToF] avoid dir = ");
  Serial.println(dir);

  // Head gesture: obstacle detected / Gest głowy: wykryto przeszkodę
  neckSend('Q');
  neckSend('O');

  if(dir < 0){
    neckSend('L');
    turn_left(1);
    scaledDelay(70);
    step_forward(1);
    scaledDelay(70);
    turn_right(1);
    neckSend('P');
  }else{
    neckSend('R');
    turn_right(1);
    scaledDelay(70);
    step_forward(1);
    scaledDelay(70);
    turn_left(1);
    neckSend('P');
  }

  stand();
}

// ====================== WEB PAGE (SPA) / STRONA WWW ======================
const char MAIN_page[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Vega Spider – Control panel / Panel sterowania</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
*{box-sizing:border-box;margin:0;padding:0;font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}
body{background:#050711;color:#f5f5f5;min-height:100vh;display:flex;align-items:flex-start;justify-content:center;padding:16px}
.wrap{max-width:1150px;width:100%}
h1{font-size:26px;margin-bottom:12px;text-align:center}
h2{font-size:18px;margin:12px 0}
.panels{display:flex;flex-wrap:wrap;gap:12px}
.card{background:rgba(16,20,40,.95);border-radius:14px;padding:12px;flex:1 1 320px;box-shadow:0 8px 18px rgba(0,0,0,.55);border:1px solid rgba(88,130,255,.35)}
.card small{color:#aaa}
.btn-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(90px,1fr));gap:8px;margin-top:8px}
button{border:none;border-radius:999px;padding:8px 10px;font-size:13px;background:linear-gradient(135deg,#3b82f6,#6366f1);color:#fff;cursor:pointer;transition:.15s;box-shadow:0 4px 10px rgba(37,99,235,.5)}
button:hover{transform:translateY(-1px);box-shadow:0 6px 16px rgba(37,99,235,.65)}
button.secondary{background:rgba(40,48,80,.9);box-shadow:none;border:1px solid rgba(148,163,255,.5)}
button.danger{background:linear-gradient(135deg,#ef4444,#b91c1c)}
.section-title{display:flex;justify-content:space-between;align-items:center;gap:8px}
#status{margin-top:8px;font-size:12px;color:#a5b4fc;min-height:16px}

/* ===== CALIBRATION: improved layout / KALIBRACJA: poprawiony układ ===== */
.cal-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px;margin-top:8px}
@media(max-width:820px){.cal-grid{grid-template-columns:1fr}}
.cal-leg{border-radius:12px;background:rgba(11,16,35,.95);padding:10px;border:1px solid rgba(99,102,241,.18)}
.cal-leg h3{font-size:14px;margin-bottom:6px;letter-spacing:.2px}
.cal-row{
  display:flex;align-items:center;justify-content:space-between;
  gap:8px;margin-top:6px;font-size:12px;flex-wrap:wrap;
}
.cal-row span.label{
  flex:1 1 150px;color:#e5e7eb;min-width:120px
}
.cal-row span.value{
  min-width:34px;text-align:center;font-variant-numeric:tabular-nums;
  padding:2px 8px;border-radius:999px;background:rgba(255,255,255,.06);
  border:1px solid rgba(148,163,255,.18)
}
.cal-row .btns{display:flex;gap:4px;flex:0 0 auto}
.cal-row button{padding:4px 10px;font-size:11px;min-width:30px}

.cal-tools{display:flex;flex-wrap:wrap;gap:6px;margin-top:10px}
pre{margin-top:6px;background:#020617;border-radius:8px;padding:8px;font-size:11px;max-height:160px;overflow:auto;border:1px solid rgba(30,64,175,.8)}
.auto-wrap{display:flex;flex-direction:column;gap:6px;margin-top:4px;font-size:12px}
.input-row{display:flex;gap:6px;align-items:center;flex-wrap:wrap}
input[type="text"],input[type="number"],select{
  flex:1 1 auto;background:#020617;border-radius:999px;border:1px solid rgba(148,163,255,.5);
  padding:6px 10px;color:#e5e7eb;font-size:12px;outline:none
}
input[type="checkbox"]{accent-color:#6366f1}
label{font-size:12px;display:flex;align-items:center;gap:4px}
.badge{display:inline-flex;align-items:center;gap:4px;padding:2px 8px;border-radius:999px;font-size:10px;background:rgba(37,99,235,.16);color:#bfdbfe}
h3.sub{margin-top:10px;font-size:14px}

/* Body-motion arrow pad / Krzyż strzałek ruchu ciała */
.body-arrows{margin-top:6px;display:flex;flex-direction:column;align-items:center;gap:4px}
.body-arrows-row{display:flex;justify-content:center;gap:6px}
.body-arrows button{min-width:44px;font-size:16px;padding:6px 0}
.body-arrows-caption{font-size:11px;color:#9ca3af;margin-top:2px;text-align:center}

/* ToF panel / Panel ToF */
.tof-row{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin-top:8px}
.tof-pill{
  display:inline-flex;align-items:center;gap:6px;
  padding:4px 10px;border-radius:999px;font-size:11px;
  background:rgba(255,255,255,.06);border:1px solid rgba(148,163,255,.22)
}

/* Kora quadruped visual panel / Panel wizualny czworonożnej Kory */
#vizCard{position:relative;overflow:hidden}
#vizCanvas{
  width:100%;height:240px;display:block;border-radius:12px;
  background:
    radial-gradient(ellipse at top, rgba(99,102,241,.22), transparent 55%),
    radial-gradient(circle at 20% 80%, rgba(56,189,248,.08), transparent 45%),
    linear-gradient(180deg, rgba(2,6,23,1), rgba(5,7,17,1));
  border:1px solid rgba(148,163,255,.25);
}
.viz-caption{font-size:11px;color:#a5b4fc;margin-top:6px}
</style>
</head>
<body>
<div class="wrap">
<h1>Vega Spider – Control panel / Panel sterowania</h1>

<div class="panels">

<div class="card" id="vizCard">
  <div class="section-title">
    <h2>Kora — quadruped mode / Kora — tryb czworonożny</h2>
    <span class="badge">client-side</span>
  </div>
  <small>Browser animation; ESP32 only serves the page. / Animacja w przeglądarce; ESP32 tylko podaje stronę.</small>
  <canvas id="vizCanvas" width="960" height="300"></canvas>
  <div class="viz-caption">Kora walking on four articulated legs. / Kora krocząca na czterech przegubowych nogach.</div>

  <div class="tof-row">
    <span class="badge">ToF</span>
    <span class="tof-pill">Mode / Tryb: <b id="tofModeLbl">STABLE</b></span>
    <button id="tofFast" class="secondary">FAST</button>
    <button id="tofStable" class="secondary">STABLE</button>
  </div>

  <div class="tof-row">
    <span class="badge">Explore / Eksploruj</span>
    <button id="discOn"  class="secondary">EXPLORE / EXPLORUJ ON (4 min)</button>
    <button id="discOff" class="secondary">EXPLORE / EXPLORUJ OFF</button>
  </div>
</div>

<div class="card">
<div class="section-title">
  <h2>Motion control / Sterowanie ruchem</h2>
  <span class="badge">online</span>
</div>
<small>Walking commands and tricks; body height and position are below. / Komendy chodu i triki; wysokość i pozycja korpusu są poniżej.</small>

<div class="btn-grid" style="margin-top:10px">
  <button data-cmd="s">Stop / Stój</button>
  <button data-cmd="f">Forward / Naprzód</button>
  <button data-cmd="b">Backward / Wstecz</button>
  <button data-cmd="l">Turn left / Obrót w lewo</button>
  <button data-cmd="r">Turn right / Obrót w prawo</button>
  <button data-cmd="L">Strafe / W bok &larr;</button>
  <button data-cmd="R">Strafe / W bok &rarr;</button>
  <button data-cmd="J">Jump / Skok</button>
  <button data-cmd="S">Spin</button>
</div>

<h3 class="sub">Main animations and tricks / Główne animacje i triki</h3>
<div class="btn-grid">
  <button data-cmd="m">Happy dance</button>
  <button data-cmd="1">Bow / Ukłon</button>
  <button data-cmd="2">Wave combo</button>
  <button data-cmd="3">Double jump</button>
  <button data-cmd="4">Happy dance+</button>
  <button data-cmd="5">Breathing / Oddychanie</button>
  <button data-cmd="H">Hello combo</button>
  <button data-cmd="G">Guard scan</button>
</div>

<h3 class="sub">More tricks / Więcej trików</h3>
<div class="btn-grid">
  <button class="secondary" data-cmd="n">Body sway / Kołysanie ciała</button>
  <button class="secondary" data-cmd="o">Moonwalk</button>
  <button class="secondary" data-cmd="c">Circle walk / Spacer w kółko</button>
  <button class="secondary" data-cmd="q">Sidewalk</button>
  <button class="secondary" data-cmd="j">Walking wave / Fala chodu</button>
  <button class="secondary" data-cmd="W">Rear-leg wave / Fala tylnymi</button>
  <button class="secondary" data-cmd="X">Rear wave 2 / Fala tylna 2</button>
  <button class="secondary" data-cmd="v">Paw wave / Machanie łapą</button>
  <button class="secondary" data-cmd="k">Paw shake / Uścisk łapy</button>
  <button class="secondary" data-cmd="u">Four-leg sequence / Sekwencja 4 nóg</button>
  <button class="secondary" data-cmd="x">Right sequence / Sekwencja prawa</button>
  <button class="secondary" data-cmd="z">Left sequence / Sekwencja lewa</button>
  <button class="secondary" data-cmd="t">Rear squat / Przysiad tył</button>
  <button class="secondary" data-cmd="T">Front squat / Przysiad przód</button>
  <button class="secondary" data-cmd="B">Low rear squat / Niski przysiad tył</button>
</div>

<h3 class="sub">Walking speed / Prędkość chodu</h3>
<div class="btn-grid">
  <button data-cmd="+">Faster / Szybciej</button>
  <button data-cmd="-">Slower / Wolniej</button>
</div>

<h3 class="sub">Body height (Z axis) / Wysokość ciała (oś Z)</h3>
<div class="btn-grid">
  <button id="bodyUp" class="secondary">Body up / Ciało w górę (+Z)</button>
  <button id="bodyDown" class="secondary">Body down / Ciało w dół (−Z)</button>
  <button data-cmd="E" class="secondary">High / Wysoko (preset)</button>
  <button data-cmd="F" class="secondary">Low / Nisko (preset)</button>
</div>

<h3 class="sub">Body motion – forward/back/sides / Ruch ciała – przód/tył/boki</h3>
<div class="body-arrows">
  <div class="body-arrows-row">
    <button id="bodyFwd" class="secondary">↑</button>
  </div>
  <div class="body-arrows-row">
    <button id="bodyLeft" class="secondary">←</button>
    <button id="bodyRight" class="secondary">→</button>
  </div>
  <div class="body-arrows-row">
    <button id="bodyBack" class="secondary">↓</button>
  </div>
  <div class="body-arrows-row">
    <button id="bodyCenter" class="secondary">●</button>
  </div>
  <div class="body-arrows-caption">Gentle body shifts and tilts; ● resets the position. / Delikatne ruchy korpusu; ● resetuje pozycję.</div>
</div>

<div id="status"></div>
</div>

<div class="card">
<div class="section-title">
  <h2>Servo calibration / Kalibracja serw</h2>
  <label><input type="checkbox" id="calMode"> calibration mode / tryb kalibracji</label>
</div>
<small>In calibration mode IK stops and servos hold 90° plus offset. / W trybie kalibracji IK zatrzymuje się, a serwa stoją na 90° plus offset.</small>

<div class="cal-grid">
  <div class="cal-leg">
    <h3>Front right / Przednia prawa</h3>
    <div class="cal-row">
      <span class="label">Servo 0</span>
      <span class="value" data-offset data-leg="0" data-joint="0">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="0" data-joint="0" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="0" data-joint="0" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 1 (tip / końcówka)</span>
      <span class="value" data-offset data-leg="0" data-joint="1">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="0" data-joint="1" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="0" data-joint="1" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 2 (near body / przy ciele)</span>
      <span class="value" data-offset data-leg="0" data-joint="2">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="0" data-joint="2" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="0" data-joint="2" data-delta="1">+</button>
      </div>
    </div>
  </div>

  <div class="cal-leg">
    <h3>Rear right / Tylna prawa</h3>
    <div class="cal-row">
      <span class="label">Servo 0</span>
      <span class="value" data-offset data-leg="1" data-joint="0">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="1" data-joint="0" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="1" data-joint="0" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 1 (tip / końcówka)</span>
      <span class="value" data-offset data-leg="1" data-joint="1">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="1" data-joint="1" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="1" data-joint="1" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 2 (near body / przy ciele)</span>
      <span class="value" data-offset data-leg="1" data-joint="2">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="1" data-joint="2" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="1" data-joint="2" data-delta="1">+</button>
      </div>
    </div>
  </div>

  <div class="cal-leg">
    <h3>Front left / Przednia lewa</h3>
    <div class="cal-row">
      <span class="label">Servo 0</span>
      <span class="value" data-offset data-leg="2" data-joint="0">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="2" data-joint="0" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="2" data-joint="0" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 1 (tip / końcówka)</span>
      <span class="value" data-offset data-leg="2" data-joint="1">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="2" data-joint="1" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="2" data-joint="1" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 2 (near body / przy ciele)</span>
      <span class="value" data-offset data-leg="2" data-joint="2">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="2" data-joint="2" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="2" data-joint="2" data-delta="1">+</button>
      </div>
    </div>
  </div>

  <div class="cal-leg">
    <h3>Rear left / Tylna lewa</h3>
    <div class="cal-row">
      <span class="label">Servo 0</span>
      <span class="value" data-offset data-leg="3" data-joint="0">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="3" data-joint="0" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="3" data-joint="0" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 1 (tip / końcówka)</span>
      <span class="value" data-offset data-leg="3" data-joint="1">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="3" data-joint="1" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="3" data-joint="1" data-delta="1">+</button>
      </div>
    </div>
    <div class="cal-row">
      <span class="label">Servo 2 (near body / przy ciele)</span>
      <span class="value" data-offset data-leg="3" data-joint="2">0</span>
      <div class="btns">
        <button class="secondary" data-cal-btn data-leg="3" data-joint="2" data-delta="-1">−</button>
        <button class="secondary" data-cal-btn data-leg="3" data-joint="2" data-delta="1">+</button>
      </div>
    </div>
  </div>
</div>

<div class="cal-tools">
  <button id="calReset" class="secondary">Reset offsets / Reset offsetów</button>
  <button id="calExport" class="secondary">Show calValues code / Pokaż kod calValues</button>
</div>
<pre id="calCode"></pre>
</div>

</div>

<div class="card" style="margin-top:12px">
<div class="section-title">
  <h2>Automatic moves / Auto-ruchy</h2>
  <span class="badge">experimental / eksperymentalne</span>
</div>
<small>The robot can execute a programmed command sequence at a selected interval. / Robot może wykonywać sekwencję komend w zadanym odstępie.</small>
<div class="auto-wrap">
  <div class="input-row">
    <label for="seqInput" style="flex:0 0 80px">Sequence / Sekwencja</label>
    <input id="seqInput" type="text" placeholder="e.g. / np. s,f,f,1,m,J">
  </div>
  <div class="input-row">
    <label for="seqInterval">Interval / Odstęp [s]</label>
    <input id="seqInterval" type="number" min="1" value="8" style="max-width:120px">
    <label><input type="checkbox" id="seqEnabled"> enable auto / włącz auto</label>
  </div>
  <div class="input-row" style="justify-content:flex-end">
    <button id="seqSave" class="secondary">Save auto-move / Zapisz auto-ruch</button>
  </div>
</div>

<h3 class="sub" style="margin-top:10px">Command-sequence recording / Nagrywanie sekwencji komend</h3>
<small>Every command clicked in the panel can be recorded and replayed. / Każdą komendę klikniętą w panelu można zapisać i odtworzyć.</small>
<div class="auto-wrap">
  <div class="input-row">
    <button id="recStart" class="secondary">Start recording / Start nagrywania</button>
    <button id="recStop"  class="secondary">Stop</button>
    <button id="recClear" class="secondary">Clear / Wyczyść</button>
    <button id="recPlay">Play sequence / Odtwórz sekwencję</button>
  </div>
  <pre id="recView">(no commands / brak komend)</pre>

  <div class="input-row">
    <label for="recSlot" style="flex:0 0 80px">Slot</label>
    <select id="recSlot">
      <option value="0">Slot 1</option>
      <option value="1">Slot 2</option>
      <option value="2">Slot 3</option>
      <option value="3">Slot 4</option>
      <option value="4">Slot 5</option>
    </select>
  </div>
  <div class="input-row">
    <label for="recName" style="flex:0 0 80px">Name / Nazwa</label>
    <input id="recName" type="text" placeholder="e.g. Show 1 / np. Pokaz 1">
  </div>
  <div class="input-row">
    <button id="recSaveSlot" class="secondary">Save to slot / Zapisz do slotu</button>
    <button id="recLoadSlot" class="secondary">Load slot / Wczytaj slot</button>
    <button id="recPlaySlot">Play slot / Odtwórz slot</button>
  </div>
  <pre id="slotView">(no saved slots / brak zapisanych slotów)</pre>
</div>
</div>

</div>

<script>
const statusEl=document.getElementById('status');
function showStatus(msg){ if(statusEl) statusEl.textContent=msg; }

// --- Cache prevention / zapobieganie pamięci podręcznej ---
function sendCmd(c){
 const url = '/cmd?c='+encodeURIComponent(c)+'&t='+Date.now();
 fetch(url, { cache: "no-store" })
  .then(r=>r.json())
  .then(d=>{
    if(d && d.error) showStatus('Error / Błąd: '+d.error);
    else showStatus('Command sent / Komenda wysłana: '+c);
  })
  .catch(_=>showStatus('Command send error / Błąd wysyłania komendy'));
}
document.querySelectorAll('[data-cmd]').forEach(btn=>{
 btn.addEventListener('click',()=>sendCmd(btn.dataset.cmd));
});

// Discover buttons
const discOn=document.getElementById('discOn');
const discOff=document.getElementById('discOff');
if(discOn){
  discOn.addEventListener('click',()=>{
    fetch('/discover/set?on=1&t='+Date.now(), {cache:"no-store"})
      .then(r=>r.json()).then(_=>showStatus('Explore / Eksploruj ON'));
  });
}
if(discOff){
  discOff.addEventListener('click',()=>{
    fetch('/discover/set?on=0&t='+Date.now(), {cache:"no-store"})
      .then(r=>r.json()).then(_=>showStatus('Explore / Eksploruj OFF'));
  });
}

// Step-based body-height Z control / Krokowe sterowanie wysokością ciała Z
const bodyUpBtn=document.getElementById('bodyUp');
const bodyDownBtn=document.getElementById('bodyDown');
if(bodyUpBtn){
  bodyUpBtn.addEventListener('click',()=>{
    fetch('/body/step?dir=up&t='+Date.now(), { cache:"no-store" })
      .then(r=>r.json())
      .then(d=>{ if(d.error) showStatus('Error / Błąd: '+d.error); else showStatus('Body raised one step / Ciało podniesione o krok'); })
      .catch(_=>showStatus('Body raise error / Błąd podnoszenia ciała'));
  });
}
if(bodyDownBtn){
  bodyDownBtn.addEventListener('click',()=>{
    fetch('/body/step?dir=down&t='+Date.now(), { cache:"no-store" })
      .then(r=>r.json())
      .then(d=>{ if(d.error) showStatus('Error / Błąd: '+d.error); else showStatus('Body lowered one step / Ciało opuszczone o krok'); })
      .catch(_=>showStatus('Body lower error / Błąd opuszczania ciała'));
  });
}

// Arrow buttons: body shift/tilt / Strzałki ruchu i pochylenia korpusu
function sendBodyShift(dir,label){
  fetch('/body/shift?dir='+encodeURIComponent(dir)+'&t='+Date.now(), { cache:"no-store" })
    .then(r=>r.json())
    .then(d=>{ if(d.error) showStatus('Error / Błąd: '+d.error); else showStatus(label); })
    .catch(_=>showStatus('Body motion error / Błąd ruchu korpusu'));
}
const bodyFwdBtn=document.getElementById('bodyFwd');
const bodyBackBtn=document.getElementById('bodyBack');
const bodyLeftBtn=document.getElementById('bodyLeft');
const bodyRightBtn=document.getElementById('bodyRight');
const bodyCenterBtn=document.getElementById('bodyCenter');

if(bodyFwdBtn) bodyFwdBtn.addEventListener('click',()=>sendBodyShift('forward','Body forward / Korpus do przodu'));
if(bodyBackBtn) bodyBackBtn.addEventListener('click',()=>sendBodyShift('back','Body backward / Korpus do tyłu'));
if(bodyLeftBtn) bodyLeftBtn.addEventListener('click',()=>sendBodyShift('left','Body left / Korpus w lewo'));
if(bodyRightBtn) bodyRightBtn.addEventListener('click',()=>sendBodyShift('right','Body right / Korpus w prawo'));
if(bodyCenterBtn) bodyCenterBtn.addEventListener('click',()=>sendBodyShift('center','Body centred / Korpus wyśrodkowany'));

// Calibration / Kalibracja
function updateCalFrom(data){
 if(!data||!data.cal)return;
 const vals=data.cal;
 document.querySelectorAll('[data-offset]').forEach(span=>{
  const leg=parseInt(span.dataset.leg);
  const joint=parseInt(span.dataset.joint);
  span.textContent=vals[leg][joint];
 });
}
document.querySelectorAll('[data-cal-btn]').forEach(btn=>{
 btn.addEventListener('click',()=>{
  const leg=btn.dataset.leg;
  const joint=btn.dataset.joint;
  const delta=btn.dataset.delta;
  fetch('/cal/step?leg='+leg+'&joint='+joint+'&delta='+delta+'&t='+Date.now(), { cache:"no-store" })
    .then(r=>r.json())
    .then(data=>updateCalFrom(data));
 });
});
const calMode=document.getElementById('calMode');
if(calMode){
  calMode.addEventListener('change',()=>{
   const on=calMode.checked?1:0;
   fetch('/cal/mode?on='+on+'&t='+Date.now(), { cache:"no-store" })
    .then(_=>showStatus(on?'Calibration mode enabled / Tryb kalibracji włączony':'Calibration mode disabled / Tryb kalibracji wyłączony'));
  });
}
const calReset=document.getElementById('calReset');
if(calReset){
  calReset.addEventListener('click',()=>{
   fetch('/cal/reset?t='+Date.now(), { cache:"no-store" })
    .then(r=>r.json())
    .then(data=>{ updateCalFrom(data); showStatus('Offsets reset / Offsety zresetowane'); });
  });
}
const calExport=document.getElementById('calExport');
if(calExport){
  calExport.addEventListener('click',()=>{
   fetch('/cal/export?t='+Date.now(), { cache:"no-store" })
     .then(r=>r.text())
     .then(t=>{ document.getElementById('calCode').textContent=t; });
  });
}

// Automatic motion / Auto-ruch
const seqSave=document.getElementById('seqSave');
if(seqSave){
  seqSave.addEventListener('click',()=>{
   const seq=document.getElementById('seqInput').value;
   const interval=document.getElementById('seqInterval').value||'8';
   const enabled=document.getElementById('seqEnabled').checked?1:0;
   fetch('/seq/set?seq='+encodeURIComponent(seq)+'&interval='+encodeURIComponent(interval)+'&enabled='+enabled+'&t='+Date.now(), { cache:"no-store" })
    .then(r=>r.json())
    .then(_=>showStatus('Auto-move saved / Auto-ruch zapisany'));
  });
}

// Sequence recording / Nagrywanie sekwencji
function refreshRec(){
 fetch('/rec/get?t='+Date.now(), { cache:"no-store" })
  .then(r=>r.json())
  .then(d=>{
    const pre=document.getElementById('recView');
    if(!pre)return;
    let txt='';
    if(d.seq && d.seq.length){ txt=d.seq; }
    else{ txt='(no commands / brak komend)'; }
    if(d.recording) txt='[RECORDING / NAGRYWANIE]\n'+txt;
    pre.textContent=txt;
  })
  .catch(_=>{});
}
const recStart=document.getElementById('recStart');
const recStop=document.getElementById('recStop');
const recClear=document.getElementById('recClear');
const recPlay=document.getElementById('recPlay');

if(recStart) recStart.addEventListener('click',()=>fetch('/rec/start?t='+Date.now(), { cache:"no-store" }).then(_=>{showStatus('Recording enabled / Nagrywanie włączone'); refreshRec();}));
if(recStop)  recStop.addEventListener('click',()=>fetch('/rec/stop?t='+Date.now(),  { cache:"no-store" }).then(_=>{showStatus('Recording disabled / Nagrywanie wyłączone'); refreshRec();}));
if(recClear) recClear.addEventListener('click',()=>fetch('/rec/clear?t='+Date.now(), { cache:"no-store" }).then(_=>{showStatus('Sequence cleared / Sekwencja wyczyszczona'); refreshRec();}));
if(recPlay)  recPlay.addEventListener('click',()=>fetch('/rec/play?t='+Date.now(),  { cache:"no-store" }).then(_=>showStatus('Playing sequence / Odtwarzanie sekwencji')));

// Sequence slots / Sloty sekwencji
const recSlotSel   = document.getElementById('recSlot');
const recNameInput = document.getElementById('recName');
const recSaveSlot  = document.getElementById('recSaveSlot');
const recLoadSlot  = document.getElementById('recLoadSlot');
const recPlaySlot  = document.getElementById('recPlaySlot');

function refreshSlots(){
 fetch('/rec/slots?t='+Date.now(), { cache:"no-store" })
  .then(r=>r.json())
  .then(d=>{
    const slotView=document.getElementById('slotView');
    if(!d.slots){slotView.textContent='(no saved slots / brak zapisanych slotów)';return;}
    let lines=[];
    d.slots.forEach(s=>{
      let line='Slot '+(s.idx+1)+': ';
      if(s.name && s.name.length) line+=s.name+' ';
      line+='(commands / komend: '+s.len+')';
      lines.push(line);
    });
    slotView.textContent=lines.join('\n');

    const selIdx=parseInt(recSlotSel.value);
    const cur=d.slots.find(s=>s.idx===selIdx);
    if(cur && typeof cur.name==='string'){ recNameInput.value=cur.name; }
  })
  .catch(_=>{});
}
if(recSaveSlot){
  recSaveSlot.addEventListener('click',()=>{
    const idx=recSlotSel.value||'0';
    const name=recNameInput.value||'';
    fetch('/rec/saveSlot?slot='+encodeURIComponent(idx)+'&name='+encodeURIComponent(name)+'&t='+Date.now(), { cache:"no-store" })
      .then(_=>{showStatus('Slot saved / Zapisano slot '+(parseInt(idx)+1)); refreshSlots();});
  });
}
if(recLoadSlot){
  recLoadSlot.addEventListener('click',()=>{
    const idx=recSlotSel.value||'0';
    fetch('/rec/loadSlot?slot='+encodeURIComponent(idx)+'&t='+Date.now(), { cache:"no-store" })
      .then(_=>{showStatus('Slot loaded / Wczytano slot '+(parseInt(idx)+1)); refreshRec(); refreshSlots();});
  });
}
if(recPlaySlot){
  recPlaySlot.addEventListener('click',()=>{
    const idx=recSlotSel.value||'0';
    fetch('/rec/playSlot?slot='+encodeURIComponent(idx)+'&t='+Date.now(), { cache:"no-store" })
      .then(_=>showStatus('Playing slot / Odtwarzanie slotu '+(parseInt(idx)+1)));
  });
}

// ToF mode UI
const tofModeLbl = document.getElementById('tofModeLbl');
function setToFModeUI(m){
  if(tofModeLbl) tofModeLbl.textContent = m;
}
const tofFastBtn = document.getElementById('tofFast');
const tofStableBtn = document.getElementById('tofStable');
if(tofFastBtn){
  tofFastBtn.addEventListener('click',()=>{
    fetch('/tof/mode?m=fast&t='+Date.now(), {cache:"no-store"})
      .then(r=>r.json()).then(_=>{ setToFModeUI('FAST'); showStatus('ToF FAST'); });
  });
}
if(tofStableBtn){
  tofStableBtn.addEventListener('click',()=>{
    fetch('/tof/mode?m=stable&t='+Date.now(), {cache:"no-store"})
      .then(r=>r.json()).then(_=>{ setToFModeUI('STABLE'); showStatus('ToF STABLE'); });
  });
}

// ---------- Kora quadruped Canvas animation / Animacja czworonożnej Kory ----------
(function(){
  const canvas = document.getElementById('vizCanvas');
  if(!canvas) return;
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;

  function roundedRect(x,y,w,h,r){
    const rr = Math.min(r,w/2,h/2);
    ctx.beginPath();
    ctx.moveTo(x+rr,y);
    ctx.lineTo(x+w-rr,y); ctx.quadraticCurveTo(x+w,y,x+w,y+rr);
    ctx.lineTo(x+w,y+h-rr); ctx.quadraticCurveTo(x+w,y+h,x+w-rr,y+h);
    ctx.lineTo(x+rr,y+h); ctx.quadraticCurveTo(x,y+h,x,y+h-rr);
    ctx.lineTo(x,y+rr); ctx.quadraticCurveTo(x,y,x+rr,y);
    ctx.closePath();
  }

  function glowDot(x,y,r,color,blur){
    ctx.save();
    ctx.shadowColor=color; ctx.shadowBlur=blur;
    ctx.fillStyle=color; ctx.beginPath(); ctx.arc(x,y,r,0,Math.PI*2); ctx.fill();
    ctx.restore();
  }

  function drawBackdrop(t){
    ctx.clearRect(0,0,W,H);
    const horizon=218;
    ctx.save();
    ctx.strokeStyle='rgba(125,211,252,.075)'; ctx.lineWidth=1;
    for(let x=-80;x<W+80;x+=64){
      const drift=(t*12)%64;
      ctx.beginPath(); ctx.moveTo(W/2+(x-W/2)*.34+drift*.34,horizon);
      ctx.lineTo(x+drift,H); ctx.stroke();
    }
    for(let y=horizon;y<H;y+=18){
      ctx.globalAlpha=1-(y-horizon)/(H-horizon);
      ctx.beginPath(); ctx.moveTo(0,y); ctx.lineTo(W,y); ctx.stroke();
    }
    ctx.restore();

    const shadow=ctx.createRadialGradient(480,250,12,480,250,250);
    shadow.addColorStop(0,'rgba(0,0,0,.58)'); shadow.addColorStop(1,'rgba(0,0,0,0)');
    ctx.fillStyle=shadow; ctx.beginPath(); ctx.ellipse(480,250,260,29,0,0,Math.PI*2); ctx.fill();
  }

  function legPose(hipX,hipY,groundY,phase,near){
    const wave=Math.sin(phase);
    const lift=Math.max(0,wave);
    const footX=hipX+wave*18+(near?5:-5);
    const footY=groundY-lift*13;
    const kneeX=hipX+(near?-23:23)-wave*5;
    const kneeY=hipY+43-lift*4;
    return {hip:{x:hipX,y:hipY},knee:{x:kneeX,y:kneeY},foot:{x:footX,y:footY}};
  }

  function drawLeg(p,near,wireColor){
    ctx.save();
    ctx.globalAlpha=near?1:.58;
    ctx.lineCap='round'; ctx.lineJoin='round';

    ctx.strokeStyle='#0a0d12'; ctx.lineWidth=17;
    ctx.beginPath(); ctx.moveTo(p.hip.x,p.hip.y); ctx.lineTo(p.knee.x,p.knee.y); ctx.lineTo(p.foot.x,p.foot.y); ctx.stroke();
    ctx.strokeStyle=near?'#dce5eb':'#7f8993'; ctx.lineWidth=10;
    ctx.beginPath(); ctx.moveTo(p.hip.x,p.hip.y); ctx.lineTo(p.knee.x,p.knee.y); ctx.lineTo(p.foot.x,p.foot.y); ctx.stroke();
    ctx.strokeStyle='rgba(255,255,255,.5)'; ctx.lineWidth=2;
    ctx.beginPath(); ctx.moveTo(p.hip.x-2,p.hip.y); ctx.lineTo(p.knee.x-2,p.knee.y); ctx.lineTo(p.foot.x-2,p.foot.y); ctx.stroke();

    ctx.strokeStyle=wireColor; ctx.lineWidth=2.2;
    ctx.beginPath(); ctx.moveTo(p.hip.x+5,p.hip.y+2); ctx.quadraticCurveTo(p.knee.x+11,p.knee.y-5,p.foot.x+4,p.foot.y-7); ctx.stroke();

    [p.hip,p.knee].forEach(function(j){
      ctx.fillStyle='#11151b'; ctx.beginPath(); ctx.arc(j.x,j.y,11,0,Math.PI*2); ctx.fill();
      ctx.strokeStyle=near?'#9aa9b5':'#59636d'; ctx.lineWidth=3; ctx.stroke();
      ctx.fillStyle='#25313b'; ctx.beginPath(); ctx.arc(j.x,j.y,3.5,0,Math.PI*2); ctx.fill();
    });

    ctx.strokeStyle='#080a0e'; ctx.lineWidth=8;
    ctx.beginPath(); ctx.moveTo(p.foot.x-14,p.foot.y+2); ctx.lineTo(p.foot.x+15,p.foot.y+2); ctx.stroke();
    ctx.restore();
  }

  function screw(x,y){
    ctx.fillStyle='#6b7782'; ctx.beginPath(); ctx.arc(x,y,3,0,Math.PI*2); ctx.fill();
    ctx.strokeStyle='#c7d0d8'; ctx.lineWidth=1; ctx.beginPath(); ctx.moveTo(x-2,y); ctx.lineTo(x+2,y); ctx.stroke();
  }

  function drawBody(bob){
    ctx.save(); ctx.translate(0,bob);

    // Black lower chassis / Czarny spód korpusu
    ctx.fillStyle='#090c11';
    roundedRect(286,145,363,51,13); ctx.fill();
    ctx.strokeStyle='#39434c'; ctx.lineWidth=2; ctx.stroke();

    // White elongated body / Biały wydłużony korpus
    const bodyGrad=ctx.createLinearGradient(0,102,0,178);
    bodyGrad.addColorStop(0,'#ffffff'); bodyGrad.addColorStop(.55,'#dce4ea'); bodyGrad.addColorStop(1,'#aeb9c2');
    ctx.fillStyle=bodyGrad;
    roundedRect(274,100,358,80,19); ctx.fill();
    ctx.strokeStyle='#f8fafc'; ctx.lineWidth=2; ctx.stroke();
    ctx.fillStyle='rgba(255,255,255,.55)'; roundedRect(291,112,260,8,4); ctx.fill();
    screw(293,161); screw(606,161);

    // Side identity plate / Boczna tabliczka
    ctx.fillStyle='#1a222b'; roundedRect(402,137,96,26,8); ctx.fill();
    ctx.strokeStyle='#4b5d6c'; ctx.stroke();
    ctx.fillStyle='#a5f3fc'; ctx.font='bold 14px system-ui,sans-serif'; ctx.textAlign='center';
    ctx.fillText('KORA',450,155);

    // Black front panel and red status light / Czarny panel przedni i czerwone światło
    ctx.fillStyle='#070a0f';
    ctx.beginPath(); ctx.moveTo(618,112); ctx.lineTo(655,126); ctx.lineTo(655,170); ctx.lineTo(618,178); ctx.closePath(); ctx.fill();
    ctx.strokeStyle='#343e47'; ctx.lineWidth=2; ctx.stroke();
    glowDot(642,148,5,'#ff3b4d',14);

    // Short white neck / Krótka biała szyja
    const neckGrad=ctx.createLinearGradient(0,72,0,112);
    neckGrad.addColorStop(0,'#f8fafc'); neckGrad.addColorStop(1,'#9aa7b1');
    ctx.fillStyle=neckGrad; roundedRect(554,72,39,43,10); ctx.fill();
    ctx.strokeStyle='#dce6ed'; ctx.lineWidth=2; ctx.stroke();
    ctx.fillStyle='#11161c'; ctx.beginPath(); ctx.ellipse(573.5,111,24,7,0,0,Math.PI*2); ctx.fill();

    // White rounded ears / Białe zaokrąglone uszy
    ctx.fillStyle='#edf2f5'; ctx.strokeStyle='#9eabb5'; ctx.lineWidth=3;
    ctx.beginPath(); ctx.ellipse(518,52,18,28,-.18,0,Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.beginPath(); ctx.ellipse(629,52,18,28,.18,0,Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.fillStyle='#aebac3';
    ctx.beginPath(); ctx.ellipse(520,53,7,17,-.18,0,Math.PI*2); ctx.fill();
    ctx.beginPath(); ctx.ellipse(627,53,7,17,.18,0,Math.PI*2); ctx.fill();

    // Black square head / Czarna kwadratowa głowa
    const headGrad=ctx.createLinearGradient(0,25,0,96);
    headGrad.addColorStop(0,'#252b33'); headGrad.addColorStop(1,'#05070a');
    ctx.fillStyle=headGrad; roundedRect(526,18,96,82,18); ctx.fill();
    ctx.strokeStyle='#53616c'; ctx.lineWidth=3; ctx.stroke();
    ctx.fillStyle='rgba(255,255,255,.08)'; roundedRect(538,28,70,8,4); ctx.fill();

    // Always-open light-blue eyes / Zawsze otwarte jasnoniebieskie oczy
    glowDot(552,58,13,'#7dd3fc',20);
    glowDot(596,58,13,'#7dd3fc',20);
    ctx.fillStyle='#e9fbff';
    ctx.beginPath(); ctx.arc(548,54,4,0,Math.PI*2); ctx.fill();
    ctx.beginPath(); ctx.arc(592,54,4,0,Math.PI*2); ctx.fill();
    ctx.restore();
  }

  function draw(){
    const t=performance.now()/1000;
    const gait=t*3.1;
    const bob=Math.sin(gait*2)*2.2;
    drawBackdrop(t);

    // Exactly four legs: two far, then body, then two near / Dokładnie cztery nogi
    const rearFar =legPose(320,171+bob,242,gait+Math.PI,false);
    const frontFar=legPose(520,171+bob,242,gait,false);
    const rearNear=legPose(390,177+bob,253,gait,false);
    const frontNear=legPose(600,177+bob,253,gait+Math.PI,true);
    drawLeg(rearFar,false,'#f97316');
    drawLeg(frontFar,false,'#ef4444');
    drawBody(bob);
    drawLeg(rearNear,true,'#f59e0b');
    drawLeg(frontNear,true,'#38bdf8');

    ctx.fillStyle='rgba(186,230,253,.78)'; ctx.font='11px system-ui,sans-serif'; ctx.textAlign='left';
    ctx.fillText('KORA // QUADRUPED MODE',22,278);
    ctx.fillStyle='rgba(148,163,184,.62)'; ctx.textAlign='right';
    ctx.fillText('4 LEGS • 4 NOGI',W-22,278);
    requestAnimationFrame(draw);
  }
  draw();
})();

window.addEventListener('load',()=>{
  fetch('/cal/get?t='+Date.now(), { cache: "no-store" }).then(r=>r.json()).then(updateCalFrom).catch(_=>{});
  fetch('/seq/get?t='+Date.now(), { cache:"no-store" }).then(r=>r.json()).then(d=>{
    if('seq' in d) document.getElementById('seqInput').value=d.seq;
    if('interval' in d) document.getElementById('seqInterval').value=Math.round(d.interval/1000)||8;
    if('enabled' in d) document.getElementById('seqEnabled').checked=!!d.enabled;
  }).catch(_=>{});
  fetch('/tof/get?t='+Date.now(), {cache:"no-store"})
    .then(r=>r.json()).then(d=>{ if(d && d.mode) setToFModeUI(d.mode.toUpperCase()); })
    .catch(_=>{});
  refreshRec();
  refreshSlots();
});
</script>
</body>
</html>)=====";

// -------------------- SETUP / KONFIGURACJA STARTOWA --------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Robot starts initialization");

  randomSeed(esp_random());

  SerialBT.begin("WegaV");

  // UART2
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);

  // XSHUT
  pinMode(XSHUT_L_PIN, OUTPUT);
  pinMode(XSHUT_R_PIN, OUTPUT);
  xshutAllOff();

  // I2C init
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // OLED
  oledInit();

  // Calibration NVS / NVS kalibracji
  prefsCal.begin("robotCal", false);
  loadCalibration();

  // Sequence and auto-move NVS / NVS sekwencji i auto-ruchu
  prefsSeq.begin("robotSeq", false);
  loadRecordedSlots();
  loadAutoFromNVS();

  // Set initial body-height logic / Ustaw początkową logikę wysokości
  updateBodyHeight(z_default_base);

  // Calm, slightly lowered initial IK pose / Spokojna, lekko obniżona pozycja startowa IK
  custom_set_site(0, x_default - x_offset, y_start + y_step, z_startup_relaxed);
  custom_set_site(1, x_default - x_offset, y_start + y_step, z_startup_relaxed);
  custom_set_site(2, x_default + x_offset, y_start,         z_startup_relaxed);
  custom_set_site(3, x_default + x_offset, y_start,         z_startup_relaxed);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      site_now[i][j] = site_expect[i][j];
    }
  }

  xTaskCreate(
    servo_service,
    "TaskOne",
    3 * 1024,
    NULL,
    1,
    &Task_HandleOne
  );
  Serial.println("Servo service started");

  servo_attach();
  writeCurrentIKPoseImmediate();
  delay(350);

  // Soft start after power-on / Miękki start po włączeniu zasilania:
  // first settle gently into a slightly lowered pose, / najpierw spokojne wejście w lekko obniżoną pozycję,
  // then move smoothly to the default pose. / potem płynne przejście do pozycji wyjściowej.
  move_speed = 0.45;
  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
  move_speed = leg_move_speed;

  Serial.println("Servos initialized");

  setupWiFi();
  initEspNow();
  setupWebServer();

  markActivity();
  oledSetAlert("BOOT OK");
  oledRender();

  // Wake the head after startup / Obudź głowę po starcie
  neckSendSeq("OQP");

  Serial.println("Robot initialization Complete");
}

// -------------------- LOOP --------------------
void loop() {
  if (hasEspNowCmd) {
    char ch = espNowCmd;
    hasEspNowCmd   = false;
    isBlueTooth    = false;
    isSerialControl = true;
    controlOperations(ch);
  }

  if (SerialBT.available()) {
    isBlueTooth     = true;
    isSerialControl = false;
    bluetoothOper();
  }

  if (Serial2.available()) {
    isBlueTooth     = false;
    isSerialControl = true;
    uartOper();
  }

  // Simple Serial parser / Prosty parser Serial
  static String serialCmd = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialCmd.length() > 0) {
        Serial.print("Serial command: ");
        Serial.println(serialCmd);

        if (serialCmd.equalsIgnoreCase("IP")) {
          printWiFiAddress();
        } else if (serialCmd == "US") {
          enterCalibrationMode();
        } else if (serialCmd == "UE") {
          exitCalibrationMode();
        } else if (serialCmd.length() == 1) {
          char ch = serialCmd[0];
          isBlueTooth     = false;
          isSerialControl = true;
          controlOperations(ch);
        } else {
          Serial.println("Unknown Serial text command / Nieznana komenda tekstowa z Serial.");
        }
      }
      serialCmd = "";
    } else {
      serialCmd += c;
    }
  }

  // Periodic ToF reads and stationary reaction / Okresowe odczyty ToF i reakcja podczas postoju
  if(!calibrationMode && pcaReady){
    unsigned long now = millis();
    if(now - lastToFReadMs >= tofReadPeriodMs()){
      lastToFReadMs = now;
      readToFPairFiltered();
      handleToFApproachWhileStanding();
    }
  }

  handleWiFiStatus();
  server.handleClient();

  handleDiscoverMode();
  handleSleepSnore();
  handleAutoSequence();
  handleIdlePowerSave();
  neckKeepAliveTick();
  oledTick();
}

// -------------------- WiFi + WEB / WWW --------------------
void printWiFiAddress() {
  Serial.println();
  Serial.println("========================================");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected / WiFi polaczone.");
    Serial.print("ESP32 IP address / Adres IP ESP32: ");
    Serial.println(WiFi.localIP());
    Serial.print("WEB PANEL / PANEL WWW: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    Serial.println("Enter this address in a phone or computer browser / Wpisz ten adres w przegladarce telefonu lub komputera.");
  } else {
    Serial.println("No WiFi connection; the IP address is not available yet / Brak polaczenia z WiFi; adres IP nie jest jeszcze dostepny.");
  }

  Serial.println("To show the address again, type IP and press Enter / Aby pokazac adres ponownie, wpisz IP i nacisnij Enter.");
  Serial.println("========================================");
  Serial.println();
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi / Laczenie z WiFi: ");
  Serial.println(WIFI_SSID);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiWasConnected = true;
    printWiFiAddress();
    oledSetAlert("WiFi OK");
  } else {
    wifiWasConnected = false;
    lastWifiRetryMs = millis();
    Serial.println("WiFi connection failed; ESP-NOW still works and the web panel will start after connection / Nie udalo sie polaczyc z WiFi; ESP-NOW dziala.");
    oledSetAlert("WiFi FAIL");
  }
}

void handleWiFiStatus() {
  const bool isConnected = (WiFi.status() == WL_CONNECTED);

  // Detect a new connection or WiFi recovery after a router outage. / Wykrycie nowego połączenia lub powrotu WiFi.
  if (isConnected) {
    if (!wifiWasConnected) {
      wifiWasConnected = true;
      printWiFiAddress();
      oledSetAlert("WiFi OK");
    }
    return;
  }

  // One-time message after connection loss. / Jednorazowy komunikat po utracie połączenia.
  if (wifiWasConnected) {
    wifiWasConnected = false;
    Serial.println("WiFi disconnected; reconnecting / WiFi rozlaczone; ponawiam polaczenie...");
    oledSetAlert("WiFi LOST");
  }

  // Extra retry every 10 seconds; setAutoReconnect() remains enabled. / Dodatkowa próba co 10 sekund.
  const unsigned long now = millis();
  if (now - lastWifiRetryMs >= WIFI_RETRY_INTERVAL_MS) {
    lastWifiRetryMs = now;
    Serial.print("WiFi retry / Ponowna proba WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.reconnect();
  }
}

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", MAIN_page);
}

// -------------------- CMD HTTP --------------------
void handleCmd() {
  if (!server.hasArg("c")) {
    server.send(400, "application/json", "{\"error\":\"missing c\"}");
    return;
  }
  String c = server.arg("c");
  c.trim();
  if (c.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"empty c\"}");
    return;
  }
  char cmd = c[0];

  controlOperations(cmd);
  recordEvent(String("CMD:") + cmd);

  server.send(200, "application/json", "{\"ok\":true}");
}

// -------------------- Step-based body height Z (web) / Krokowa wysokość ciała Z --------------------
void handleBodyStep() {
  if (calibrationMode) {
    server.send(409, "application/json", "{\"error\":\"calibration\"}");
    return;
  }
  if (!pcaReady) {
    server.send(409, "application/json", "{\"error\":\"servos_off_use_E\"}");
    return;
  }
  if (!server.hasArg("dir")) {
    server.send(400, "application/json", "{\"error\":\"missing dir\"}");
    return;
  }

  String dir = server.arg("dir");
  float dz = 0.0;

  if (dir == "up") {
    dz = -5.0;
    recordEvent("BODY_STEP:UP");
  } else if (dir == "down") {
    dz = 5.0;
    recordEvent("BODY_STEP:DOWN");
  } else {
    server.send(400, "application/json", "{\"error\":\"bad dir\"}");
    return;
  }

  move_speed = stand_seat_speed;
  updateBodyHeight(bodyZCurrent + dz);

  for (int leg = 0; leg < 4; leg++) {
    custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  }
  custom_wait_all_reach();

  markActivity();
  oledSetAlert("BODY STEP");
  server.send(200, "application/json", "{\"ok\":true}");
}

// -------------------- Web body shift/tilt arrows / Strzałki ruchu i pochylenia ciała --------------------
void handleBodyShift() {
  if (calibrationMode) {
    server.send(409, "application/json", "{\"error\":\"calibration\"}");
    return;
  }
  if (!pcaReady) {
    server.send(409, "application/json", "{\"error\":\"servos_off_use_E\"}");
    return;
  }
  if (!server.hasArg("dir")) {
    server.send(400, "application/json", "{\"error\":\"missing dir\"}");
    return;
  }
  String dir = server.arg("dir");

  recordEvent(String("BODY_SHIFT:") + dir);

  if      (dir == "left")    body_left(BODY_SHIFT_STEP);
  else if (dir == "right")   body_right(BODY_SHIFT_STEP);
  else if (dir == "forward") body_front(BODY_SHIFT_STEP);
  else if (dir == "back")    body_back(BODY_SHIFT_STEP);
  else if (dir == "center")  reset_body_pose();
  else {
    server.send(400, "application/json", "{\"error\":\"bad dir\"}");
    return;
  }

  markActivity();
  server.send(200, "application/json", "{\"ok\":true}");
}

// -------------------- HTTP CALIBRATION / KALIBRACJA HTTP --------------------
void handleCalGet() {
  String json = "{\"cal\":[";
  for (int leg = 0; leg < 4; leg++) {
    json += "[";
    for (int joint = 0; joint < 3; joint++) {
      json += String(calValues[leg][joint]);
      if (joint < 2) json += ",";
    }
    json += "]";
    if (leg < 3) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleCalStep() {
  if (!server.hasArg("leg") || !server.hasArg("joint") || !server.hasArg("delta")) {
    server.send(400, "text/plain", "missing params");
    return;
  }
  int leg   = server.arg("leg").toInt();
  int joint = server.arg("joint").toInt();
  int delta = server.arg("delta").toInt();
  if (leg < 0 || leg > 3 || joint < 0 || joint > 2) {
    server.send(400, "text/plain", "bad index");
    return;
  }
  calValues[leg][joint] += delta;
  if (calValues[leg][joint] > 90)  calValues[leg][joint] = 90;
  if (calValues[leg][joint] < -90) calValues[leg][joint] = -90;
  saveCalibration();
  applyCalibrationPose();
  handleCalGet();
}

void handleCalMode() {
  bool on = server.hasArg("on") && server.arg("on") == "1";
  if (on) enterCalibrationMode();
  else    exitCalibrationMode();
  String json = String("{\"calibration\":") + (on ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleCalExport() {
  String txt = "int calValues[4][3] = {\n";
  for (int leg = 0; leg < 4; leg++) {
    txt += "  {";
    for (int joint = 0; joint < 3; joint++) {
      txt += String(calValues[leg][joint]);
      if (joint < 2) txt += ", ";
    }
    txt += "}";
    if (leg < 3) txt += ",";
    txt += "\n";
  }
  txt += "};\n";
  server.send(200, "text/plain", txt);
}

void handleCalReset() {
  for (int leg = 0; leg < 4; leg++)
    for (int joint = 0; joint < 3; joint++)
      calValues[leg][joint] = 0;
  saveCalibration();
  applyCalibrationPose();
  handleCalGet();
}

// -------------------- AUTO HTTP --------------------
void handleSeqGet() {
  String json = "{";
  String seqEsc = autoSeq;
  seqEsc.replace("\"", "'");
  json += "\"seq\":\"" + seqEsc + "\",";
  json += "\"interval\":" + String(autoIntervalMs) + ",";
  json += "\"enabled\":" + String(autoEnabled ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleSeqSet() {
  if (server.hasArg("seq")) {
    autoSeq = server.arg("seq");
  }
  if (server.hasArg("interval")) {
    long sec = server.arg("interval").toInt();
    if (sec < 1) sec = 1;
    autoIntervalMs = (unsigned long)sec * 1000UL;
  }
  if (server.hasArg("enabled")) {
    autoEnabled = (server.arg("enabled") == "1");
  }
  autoIndex      = 0;
  lastAutoMillis = millis();
  saveAutoToNVS();
  handleSeqGet();
}

// -------------------- HTTP RECORDING / NAGRYWANIE HTTP --------------------
void handleRecStart() { recordOn = true;  server.send(200, "application/json", "{\"recording\":true}"); }
void handleRecStop()  { recordOn = false; server.send(200, "application/json", "{\"recording\":false}"); }
void handleRecClear() { recordSeq = "";   server.send(200, "application/json", "{\"cleared\":true}"); }

void handleRecGet() {
  String json = "{\"recording\":";
  json += (recordOn ? "true" : "false");
  json += ",\"seq\":\"";
  json += recordSeq;
  json += "\"}";
  server.send(200, "application/json", json);
}

void handleRecPlay() {
  bool wasRecording = recordOn;
  recordOn          = false;
  server.send(200, "application/json", "{\"playing\":true}");
  playRecordedSequence(recordSeq);
  recordOn          = wasRecording;
}

void handleRecSlots() {
  String json = "{\"slots\":[";
  for (int i = 0; i < MAX_REC_SLOTS; i++) {
    String s = recSlotSeq[i];
    String n = recSlotName[i];
    int len = 0;
    if (s.length() > 0) {
      len = 1;
      for (size_t j = 0; j < s.length(); j++) if (s[j] == ',') len++;
    }
    String nEsc = n;
    nEsc.replace("\"", "'");
    json += "{\"idx\":" + String(i) + ",\"name\":\"" + nEsc + "\",\"len\":" + String(len) + "}";
    if (i < MAX_REC_SLOTS - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleRecSaveSlot() {
  if (!server.hasArg("slot")) {
    server.send(400, "application/json", "{\"error\":\"missing slot\"}");
    return;
  }
  int idx = server.arg("slot").toInt();
  if (idx < 0 || idx >= MAX_REC_SLOTS) {
    server.send(400, "application/json", "{\"error\":\"bad slot\"}");
    return;
  }
  recSlotSeq[idx] = recordSeq;
  if (server.hasArg("name")) {
    recSlotName[idx] = server.arg("name");
  }
  saveRecordedSlot(idx);
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleRecLoadSlot() {
  if (!server.hasArg("slot")) {
    server.send(400, "application/json", "{\"error\":\"missing slot\"}");
    return;
  }
  int idx = server.arg("slot").toInt();
  if (idx < 0 || idx >= MAX_REC_SLOTS) {
    server.send(400, "application/json", "{\"error\":\"bad slot\"}");
    return;
  }
  recordSeq = recSlotSeq[idx];
  recordOn  = false;
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleRecPlaySlot() {
  if (!server.hasArg("slot")) {
    server.send(400, "application/json", "{\"error\":\"missing slot\"}");
    return;
  }
  int idx = server.arg("slot").toInt();
  if (idx < 0 || idx >= MAX_REC_SLOTS) {
    server.send(400, "application/json", "{\"error\":\"bad slot\"}");
    return;
  }
  String seq = recSlotSeq[idx];
  bool wasRecording = recordOn;
  recordOn          = false;
  server.send(200, "application/json", "{\"playing\":true}");
  playRecordedSequence(seq);
  recordOn          = wasRecording;
}

// -------------------- ToF HTTP --------------------
void handleToFGet(){
  String modeStr = (tofMode == TOF_FAST) ? "fast" : "stable";
  String json = "{";
  json += "\"mode\":\"" + modeStr + "\",";
  json += "\"filtL\":" + String((filtL < 0) ? 0 : (int)filtL) + ",";
  json += "\"filtR\":" + String((filtR < 0) ? 0 : (int)filtR);
  json += "}";
  server.send(200, "application/json", json);
}
void handleToFMode(){
  if(!server.hasArg("m")){
    server.send(400, "application/json", "{\"error\":\"missing m\"}");
    return;
  }
  String m = server.arg("m");
  m.toLowerCase();
  if(m == "fast") tofMode = TOF_FAST;
  else tofMode = TOF_STABLE;

  filtL = filtR = -1;
  handleToFGet();
}

// -------------------- DISCOVER HTTP --------------------
void handleDiscoverGet(){
  unsigned long now = millis();
  unsigned long elapsed = (discoverRunning ? (now - discoverStartMs) : 0);
  long remaining = (discoverRunning ? (long)(DISCOVER_DURATION_MS - elapsed) : (discoverEnabled ? (long)DISCOVER_DURATION_MS : 0));
  if(remaining < 0) remaining = 0;

  String json = "{";
  json += "\"enabled\":" + String(discoverEnabled ? "true" : "false") + ",";
  json += "\"running\":" + String(discoverRunning ? "true" : "false") + ",";
  json += "\"remaining_ms\":" + String(remaining);
  json += "}";
  server.send(200, "application/json", json);
}

void handleDiscoverSet(){
  bool on = server.hasArg("on") && server.arg("on") == "1";
  if(on){
    discoverEnabled = true;
    startDiscover();
  }else{
    stopDiscover(false);
  }
  handleDiscoverGet();
}

// -------------------- WEB SERVER SETUP --------------------
void setupWebServer() {
  server.on("/",             HTTP_GET, handleRoot);
  server.on("/cmd",          HTTP_GET, handleCmd);
  server.on("/body/step",    HTTP_GET, handleBodyStep);
  server.on("/body/shift",   HTTP_GET, handleBodyShift);

  server.on("/cal/get",      HTTP_GET, handleCalGet);
  server.on("/cal/step",     HTTP_GET, handleCalStep);
  server.on("/cal/mode",     HTTP_GET, handleCalMode);
  server.on("/cal/export",   HTTP_GET, handleCalExport);
  server.on("/cal/reset",    HTTP_GET, handleCalReset);

  server.on("/seq/get",      HTTP_GET, handleSeqGet);
  server.on("/seq/set",      HTTP_GET, handleSeqSet);

  server.on("/rec/start",    HTTP_GET, handleRecStart);
  server.on("/rec/stop",     HTTP_GET, handleRecStop);
  server.on("/rec/clear",    HTTP_GET, handleRecClear);
  server.on("/rec/get",      HTTP_GET, handleRecGet);
  server.on("/rec/play",     HTTP_GET, handleRecPlay);
  server.on("/rec/slots",    HTTP_GET, handleRecSlots);
  server.on("/rec/saveSlot", HTTP_GET, handleRecSaveSlot);
  server.on("/rec/loadSlot", HTTP_GET, handleRecLoadSlot);
  server.on("/rec/playSlot", HTTP_GET, handleRecPlaySlot);

  server.on("/tof/get",      HTTP_GET, handleToFGet);
  server.on("/tof/mode",     HTTP_GET, handleToFMode);

  server.on("/discover/get", HTTP_GET, handleDiscoverGet);
  server.on("/discover/set", HTTP_GET, handleDiscoverSet);

  server.begin();
  Serial.println("HTTP server started");
}

// -------------------- AUTOMATIC SEQUENCE LOGIC / LOGIKA AUTO-SEKWENCJI --------------------
void handleAutoSequence() {
  if (discoverEnabled) return;
  if (sleepActive) return;
  if (!autoEnabled) return;
  if (autoSeq.length() == 0) return;
  if (!pcaReady) return;

  unsigned long now = millis();
  if (now - lastAutoMillis < autoIntervalMs) return;
  lastAutoMillis = now;

  String seqClean;
  for (size_t i = 0; i < autoSeq.length(); i++) {
    char c = autoSeq[i];
    if (c == ' ' || c == ',' || c == '\t' || c == '\r' || c == '\n') continue;
    seqClean += c;
  }
  if (seqClean.length() == 0) return;
  if (autoIndex >= (int)seqClean.length()) autoIndex = 0;

  char cmd = seqClean[autoIndex++];
  controlOperations(cmd);

  markActivity();
}

// -------------------- IDLE POWER SAVE --------------------
void handleIdlePowerSave() {
  if (discoverEnabled) return;
  if (sleepActive) return;
  if (calibrationMode) return;
  if (!pcaReady) return;
  if (autoEnabled && autoSeq.length() > 0) return;

  unsigned long now = millis();
  if (now - lastActivityMs < IDLE_TIMEOUT_MS) return;

  Serial.println("[IDLE] No commands: HIGH pose + servos OFF / Brak komend: pozycja WYSOKO + serwa OFF");
  oledSetAlert("IDLE -> OFF");

  body_up();
  scaledDelay(80);

  servo_detach();
  idleDetached = true;

  // Leave the head half-awake without a hard Z command / Zostaw głowę w pół-czuwaniu bez twardego Z
  neckSend('B');

  markActivity();
}

// -------------------- Recording logic / Logika nagrywania --------------------
void recordEvent(const String &event) {
  if (!recordOn) return;
  if (recordSeq.length() > 0) recordSeq += ",";
  recordSeq += event;
}

void playRecordedSequence(const String &seq) {
  if (seq.length() == 0) {
    Serial.println("No recorded sequence to play");
    return;
  }

  if (!pcaReady) {
    Serial.println("Servos are OFF; use E before playing a sequence / Serwa OFF; uzyj E przed odtworzeniem sekwencji.");
    return;
  }

  int start = 0;
  while (start < (int)seq.length()) {
    int idx = seq.indexOf(',', start);
    String token;
    if (idx == -1) {
      token = seq.substring(start);
      start = seq.length();
    } else {
      token = seq.substring(start, idx);
      start = idx + 1;
    }
    token.trim();
    if (token.length() == 0) continue;

    if (token.startsWith("CMD:")) {
      if (token.length() >= 5) {
        char c = token.charAt(4);
        controlOperations(c);
      }
    } else if (token == "BODY_STEP:UP") {
      updateBodyHeight(bodyZCurrent - 5.0);
      move_speed = stand_seat_speed;
      for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
      custom_wait_all_reach();
    } else if (token == "BODY_STEP:DOWN") {
      updateBodyHeight(bodyZCurrent + 5.0);
      move_speed = stand_seat_speed;
      for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
      custom_wait_all_reach();
    } else if (token.startsWith("BODY_SHIFT:")) {
      String dir = token.substring(11);
      if      (dir == "left")    body_left(BODY_SHIFT_STEP);
      else if (dir == "right")   body_right(BODY_SHIFT_STEP);
      else if (dir == "forward") body_front(BODY_SHIFT_STEP);
      else if (dir == "back")    body_back(BODY_SHIFT_STEP);
      else if (dir == "center")  reset_body_pose();
    }

    markActivity();
    scaledDelay(200);
  }
}

// -------------------- ESP-NOW INIT --------------------
void initEspNow() {
  WiFi.mode(WIFI_STA);
  Serial.print("Robot STA MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    espNowOk = false;
    return;
  }

  espNowOk = true;
  esp_now_register_recv_cb(onEspNowRecv);

  // Peer 1: transmitter/remote / Nadajnik/pilot
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, sender_mac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer (sender)");
  } else {
    Serial.println("ESP-NOW peer (sender) added");
  }

  // Peer 2: neck + eyes + ears / Szyja + oczy + uszy
  esp_now_peer_info_t headPeer = {};
  memcpy(headPeer.peer_addr, neckEyesAddress, 6);
  headPeer.channel = 0;
  headPeer.encrypt = false;

  if (esp_now_add_peer(&headPeer) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer (neck/eyes)");
  } else {
    Serial.println("ESP-NOW peer (neck/eyes) added");
  }
}

void onEspNowRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  (void)info;
  if (len <= 0) return;
  char c = (char)incomingData[0];

  espNowCmd    = c;
  hasEspNowCmd = true;

  Serial.print("ESP-NOW RX cmd: ");
  Serial.println(c);
}

void reportDone() {
  if (!espNowOk) return;
  const char msg[4] = {'d','o','n','e'};
  esp_now_send(sender_mac, (const uint8_t*)msg, sizeof(msg));
}

static inline bool isIgnoredMotionChar(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ',' || ch == ';';
}


// -------------------- SERVOS ON/OFF / SERWA WŁ./WYŁ. --------------------
void servo_attach(void) {
  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);
  pcaReady = true;
}

void servo_detach(void) {
  for (uint8_t ch = 0; ch < 16; ch++) {
    pwm.setPWM(ch, 0, 0);
  }
  pcaReady = false;
}

// -------------------- CALIBRATION MODE / TRYB KALIBRACJI --------------------
void enterCalibrationMode() {
  Serial.println("\n=== CALIBRATION MODE / TRYB KALIBRACJI (US/WEB) ===");
  calibrationMode = true;

  if (!pcaReady) {
    Serial.println("Servos were OFF; enabling for calibration / Serwa OFF; wlaczam na czas kalibracji.");
    servo_attach();
  }

  for (int leg = 0; leg < 4; leg++) {
    for (int joint = 0; joint < 3; joint++) {
      float neutral = 90 + calValues[leg][joint];
      setServoAngle(leg, joint, neutral);
    }
  }

  oledSetAlert("CAL MODE");
}

void exitCalibrationMode() {
  Serial.println("\nLeaving calibration mode / Wyjscie z trybu kalibracji.\n");

  custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
  custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
  custom_set_site(2, x_default + x_offset, y_start,         bodyZCurrent);
  custom_set_site(3, x_default + x_offset, y_start,         bodyZCurrent);

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 3; j++)
      site_now[i][j] = site_expect[i][j];

  calibrationMode = false;
  markActivity();
  oledSetAlert("");
}

void applyCalibrationPose() {
  if (!calibrationMode) return;
  if (!pcaReady) return;

  for (int leg = 0; leg < 4; leg++) {
    for (int joint = 0; joint < 3; joint++) {
      float neutral = 90 + calValues[leg][joint];
      setServoAngle(leg, joint, neutral);
    }
  }
}

void loadCalibration() {
  for (int leg = 0; leg < 4; leg++) {
    for (int joint = 0; joint < 3; joint++) {
      char key[5];
      sprintf(key, "c%d%d", leg, joint);
      calValues[leg][joint] = prefsCal.getShort(key, 0);
    }
  }
}

void saveCalibration() {
  for (int leg = 0; leg < 4; leg++) {
    for (int joint = 0; joint < 3; joint++) {
      char key[5];
      sprintf(key, "c%d%d", leg, joint);
      prefsCal.putShort(key, (int16_t)calValues[leg][joint]);
    }
  }
}

// -------------------- SEQUENCE NVS / NVS SEKWENCJI --------------------
void loadRecordedSlots() {
  for (int i = 0; i < MAX_REC_SLOTS; i++) {
    char keySeq[8];
    char keyName[8];
    sprintf(keySeq,  "s%d", i);
    sprintf(keyName, "n%d", i);
    recSlotSeq[i]  = prefsSeq.getString(keySeq,  "");
    recSlotName[i] = prefsSeq.getString(keyName, "");
  }
}

void saveRecordedSlot(int idx) {
  if (idx < 0 || idx >= MAX_REC_SLOTS) return;
  char keySeq[8];
  char keyName[8];
  sprintf(keySeq,  "s%d", idx);
  sprintf(keyName, "n%d", idx);
  prefsSeq.putString(keySeq,  recSlotSeq[idx]);
  prefsSeq.putString(keyName, recSlotName[idx]);
}

void loadAutoFromNVS() {
  autoSeq        = prefsSeq.getString("autoSeq", "");
  autoIntervalMs = prefsSeq.getUInt("autoInt", 8000);
  autoEnabled    = prefsSeq.getBool("autoEn",  false);
}

void saveAutoToNVS() {
  prefsSeq.putString("autoSeq", autoSeq);
  prefsSeq.putUInt("autoInt",  autoIntervalMs);
  prefsSeq.putBool("autoEn",   autoEnabled);
}

// -------------------- INPUT OPERATIONS / OPERACJE WEJŚĆ --------------------
void uartOper() {
  char ch = Serial2.read();
  if (isIgnoredMotionChar(ch)) return;
  Serial.print("UART2: ");
  Serial.println(ch);
  controlOperations(ch);
}

void bluetoothOper() {
  char ch = SerialBT.read();
  if (isIgnoredMotionChar(ch)) return;
  Serial.print("BT: ");
  Serial.println(ch);
  controlOperations(ch);
}

// -------------------- HEIGHT PRESETS / PRESETY WYSOKOŚCI --------------------
void body_up() {
  Serial.println("Raising the body (preset high)");
  updateBodyHeight(z_up_base);
  move_speed = stand_seat_speed;

  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
}

void body_down() {
  Serial.println("Lowering the body (preset low)");
  updateBodyHeight(z_boot_base);
  move_speed = stand_seat_speed;

  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
}

// -------------------- SWAYING / KOŁYSANIE --------------------
void sway_body() {
  Serial.println("Swaying body side to side");
  const float lean_angle = 15;
  const int   lean_delay = 600;

  for (int leg = 0; leg < 4; leg++) {
    if (leg % 2 == 0) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent + lean_angle);
    else              custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent - lean_angle);
  }
  custom_wait_all_reach();
  scaledDelay(lean_delay);

  for (int leg = 0; leg < 4; leg++) {
    if (leg % 2 == 0) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent - lean_angle);
    else              custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent + lean_angle);
  }
  custom_wait_all_reach();
  scaledDelay(lean_delay);

  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
  custom_wait_all_reach();
}

// -------------------- DISCOVER MODE LOGIC / LOGIKA TRYBU EKSPLORACJI --------------------
static void startDiscover(){
  sleepActive = false;

  if(!pcaReady){
    servo_attach();
    idleDetached = false;
  }

  // Disable automatic sequences during Discover / Wyłącz auto-sekwencje podczas eksploracji
  autoEnabled = false;

  discoverRunning = true;
  discoverEnabled = true;
  discoverStartMs = millis();
  lastDiscoverActionMs = 0;

  curStatus = 'd';
  oledSetAlert("DISCOVER ON");
  markActivity();

  // Wake the head and set a curious-walk expression / Obudź głowę i ustaw ciekawski spacer
  neckSendSeq("OQPQ");

  // Stabilise the starting pose / Ustabilizuj pozycję startową
  stand();
}

static void stopDiscover(bool naturalSleep){
  discoverEnabled = false;
  discoverRunning = false;

  curStatus = 's';
  oledSetAlert(naturalSleep ? "DISCOVER END" : "DISCOVER OFF");

  if(naturalSleep){
    // The head falls asleep with the spider / Głowa zasypia razem z pająkiem
    neckSendSeq("ZB");
    enterSleepMode();
  }else{
    markActivity();
    stand();
    // After manual shutdown leave the eyes open, no drama / Po ręcznym wyłączeniu zostaw oczy otwarte
    neckSend('O');
    neckSend('P');
  }
}

static void handleDiscoverMode(){
  if(!discoverEnabled) return;

  // Manual commands disable Discover in controlOperations(). / Ręczne komendy wyłączają Discover.

  if(!discoverRunning){
    startDiscover();
    return;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - discoverStartMs;

  if(elapsed >= DISCOVER_DURATION_MS){
    stopDiscover(true);
    return;
  }

  if(!pcaReady || calibrationMode) return;

  if(now - lastDiscoverActionMs < DISCOVER_COOLDOWN_MS) return;
  lastDiscoverActionMs = now;

  // Main motion decision / Główna decyzja ruchu
  if(isObstacleAheadNow()){
    autoAvoidSmart();
    curStatus = 's';
  }else{
    curStatus = 'f';
    neckSend('P');
    neckSend('O');
    step_forward(1);
  }

  markActivity();
}

// -------------------- SLEEP + SNORE --------------------
static void enterSleepMode(){
  sleepActive = true;
  discoverEnabled = false;
  discoverRunning = false;

  // Settle into a calm lower pose / Ułóż się spokojnie w niższej pozycji
  updateBodyHeight(z_boot_base);
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();

  stand();

  // Disable servos for power saving and a sleep effect / Wyłącz serwa dla oszczędzania i efektu snu
  servo_detach();
  idleDetached = true;

  curStatus = 's';
  oledSetAlert("SLEEP");
  markActivity();

  lastSnoreMs = 0;
  snoreFlip = false;

  Serial.println("[SLEEP] Enter sleep mode");
}

static void handleSleepSnore(){
  if(!sleepActive) return;

  unsigned long now = millis();
  if(now - lastSnoreMs < 1200) return;
  lastSnoreMs = now;

  snoreFlip = !snoreFlip;

  // Placeholder for a future MP3 / Miejsce na przyszły MP3
  Serial.println("SNORE...");
  oledSetAlert(snoreFlip ? "Zzz..." : "zzZ...");
}

// -------------------- MAIN CONTROL / GŁÓWNE STEROWANIE --------------------
void controlOperations(char ch) {

  // Every command wakes the robot from sleep / Każda komenda wybudza robota
  if(sleepActive){
    sleepActive = false;
    oledSetAlert("WAKE");
    // Wake the head / Obudź głowę
    neckSendSeq("OQP");
  }

  // A normal command during Discover is treated as a manual override. / Normalna komenda podczas Discover oznacza ręczne sterowanie.
  if(discoverEnabled && ch != 'd'){
    stopDiscover(false);
  }

  // Command 'd' toggles Discover / Komenda 'd' przełącza Discover
  if(ch == 'd'){
    if(discoverEnabled){
      stopDiscover(false);
    }else{
      startDiscover();
    }
    reportDone();
    return;
  }

  // If servos were disabled by idle timeout or sleep / Jeśli serwa wyłączyła bezczynność lub sen:
  if (!pcaReady) {
    if (ch == 'E') {
      Serial.println("[POWER] Servos OFF: command E enables them / Serwa OFF: komenda E je wlacza");
      servo_attach();
      idleDetached = false;
      body_up();
      markActivity();
      oledSetAlert("SERVO ON");

      // Head gesture: awake again / Gest głowy: znowu nie śpię
      neckSendSeq("OQP");

      reportDone();
      return;
    } else {
      Serial.println("[POWER] Servos are OFF; use E to enable them / Serwa sa wylaczone; uzyj E.");
      oledSetAlert("SERVO OFF");
      reportDone();
      return;
    }
  }

  if (calibrationMode) {
    Serial.println("Calibration mode: motion commands ignored / Tryb kalibracji: komendy ruchu sa ignorowane.");
    reportDone();
    return;
  }

  Serial.print("Received command: ");
  Serial.print(ch);
  Serial.print(" | speed x=");
  Serial.println(speed_multiple, 2);

  // Mirror the command to the head before motion so it looks with the body. / Wyślij komendę do głowy przed ruchem.
  neckMirrorRobotCmd(ch);

  bool isLocOrPose =
    (ch == 's' || ch == 'f' || ch == 'b' || ch == 'l' || ch == 'r' ||
     ch == 'L' || ch == 'R' || ch == 't' || ch == 'T' || ch == 'B' ||
     ch == '+' || ch == '-' || ch == 'E' || ch == 'F' || ch == 'P' ||
     ch == 'H' || ch == 'K' || ch == '!' || ch == '0');
  bool known = true;
  float pose[4][3];

  if (!isLocOrPose) savePose(pose);

  switch (ch) {
    case 's':
    case '!':
    case '0':
      safeStopNow("STOP");
      break;

    case 'f':
      if(isObstacleAheadNow()){
        Serial.println("[ToF] Front obstacle: smart avoid / Przeszkoda z przodu: smart avoid");
        autoAvoidSmart();
        curStatus = 's';
      }else{
        curStatus = 'f';
        step_forward(1);
      }
      break;

    case 'b': curStatus = 'b'; step_back(1); break;
    case 'l': curStatus = 'l'; turn_left(1); break;
    case 'r': curStatus = 'r'; turn_right(1); break;
    case 'L': move_left(1); break;
    case 'R': move_right(1); break;

    case 'X': rear_leg_wave2(3); break;
    case 'T': curStatus = 'T'; squat_front_legs(); break;
    case 'B': curStatus = 'B'; squat_back_legs_stop(); break;
    case 't': curStatus = 't'; squat_back_legs(); break;

    case 'E': curStatus = 'E'; body_up(); break;
    case 'F': curStatus = 'F'; body_down(); break;
    case 'P': curStatus = 'P'; reset_body_pose(); break;

    case 'W': rear_legs_wave(3); break;
    case 'v': curStatus = 'v'; hand_wave(3); break;
    case 'k': curStatus = 'k'; hand_shake(3); break;

    case 'u': curStatus = 'u'; sequence_legs_move(); break;
    case 'x': curStatus = 'x'; sequence_right_legs_move(); break;
    case 'z': curStatus = 'z'; sequence_left_legs_move(); break;

    case 'm': curStatus = 'm'; trick_happy_dance(); break;
    case 'n': curStatus = 'n'; sway_body(); break;
    case 'o': curStatus = 'o'; moonwalk(3); break;
    case 'c': curStatus = 'c'; circle_walk(5); break;
    case 'q': curStatus = 'q'; sidewalk(3); break;

    case 'J': curStatus = 'J'; jump_forward(); break;
    case 'S': curStatus = 'S'; spin_on_leg(0, 3); break;
    case 'D': curStatus = 'D'; dance_step(2); break;
    case 'j': curStatus = 'j'; wave_walk(3); break;

    case '+':
      if (speed_multiple < max_speed_multiple) speed_multiple += speed_increment;
      if (speed_multiple > max_speed_multiple) speed_multiple = max_speed_multiple;
      Serial.print("Speed x: "); Serial.println(speed_multiple, 2);
      break;

    case '-':
      if (speed_multiple > min_speed_multiple) speed_multiple -= speed_increment;
      if (speed_multiple < min_speed_multiple) speed_multiple = min_speed_multiple;
      Serial.print("Speed x: "); Serial.println(speed_multiple, 2);
      break;

    case '1': trick_bow_hello(); break;
    case '2': trick_wave_combo(); break;
    case '3': trick_double_jump(); break;
    case '4': trick_happy_dance(); break;
    case '5': trick_idle_breath(); break;
    case 'H': trick_hello_combo(); break;
    case 'G': trick_guard_scan(); break;

    default:
      Serial.println("Unknown command");
      known = false;
      break;
  }

  if (!isLocOrPose && known) {
    move_speed = stand_seat_speed;
    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, pose[leg][0], pose[leg][1], pose[leg][2]);
    custom_wait_all_reach();
  }

  markActivity();
  reportDone();
}

// -------------------- SEQUENCES / SEKWENCJE --------------------
void sequence_legs_move() {
  float lowerHeight = bodyZCurrent;
  float raiseHeight = bodyZUpCurrent;

  for (int leg = 0; leg < 4; leg++) {
    custom_set_site(leg, KEEP, KEEP, lowerHeight);
    custom_wait_all_reach();
    scaledDelay(120);

    custom_set_site(leg, KEEP, KEEP, raiseHeight);
    custom_wait_all_reach();
  }
}

void sequence_right_legs_move() {
  float lowerHeight = bodyZCurrent;
  float raiseHeight = bodyZUpCurrent;

  int rightLegs[] = {1, 3};

  for (int idx = 0; idx < 2; idx++) custom_set_site(rightLegs[idx], KEEP, KEEP, lowerHeight);
  custom_wait_all_reach();
  scaledDelay(800);

  for (int idx = 0; idx < 2; idx++) custom_set_site(rightLegs[idx], KEEP, KEEP, raiseHeight);
  custom_wait_all_reach();
}

void sequence_left_legs_move() {
  float lowerHeight = bodyZCurrent;
  float raiseHeight = bodyZUpCurrent;

  int leftLegs[] = {0, 2};

  for (int idx = 0; idx < 2; idx++) custom_set_site(leftLegs[idx], KEEP, KEEP, lowerHeight);
  custom_wait_all_reach();
  scaledDelay(800);

  for (int idx = 0; idx < 2; idx++) custom_set_site(leftLegs[idx], KEEP, KEEP, raiseHeight);
  custom_wait_all_reach();
}

// -------------------- STAND / SIT --------------------
void sit(void) {
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++) set_site(leg, KEEP, KEEP, bodyZCurrent);
  wait_all_reach();
}

void stand(void) {
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++) set_site(leg, KEEP, KEEP, bodyZCurrent);
  wait_all_reach();
}

void safeStopNow(const char* reason) {
  stopDiscover(false);
  curStatus = 's';
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++) set_site(leg, KEEP, KEEP, bodyZCurrent);
  wait_all_reach();
  oledSetAlert(reason);
  neckSendSeq("C");
}

// -------------------- IK low-level --------------------
void wait_reach(int leg) {
  while (1) {
    if (site_now[leg][0] == site_expect[leg][0] &&
        site_now[leg][1] == site_expect[leg][1] &&
        site_now[leg][2] == site_expect[leg][2]) break;
    delay(1);
  }
}

void wait_all_reach(void) {
  for (int i = 0; i < 4; i++) wait_reach(i);
}

void set_site(int leg, float x, float y, float z) {
  float length_x = 0, length_y = 0, length_z = 0;

  if (x != KEEP) length_x = x - site_now[leg][0];
  if (y != KEEP) length_y = y - site_now[leg][1];
  if (z != KEEP) length_z = z - site_now[leg][2];

  float length = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));

  if (length == 0) {
    temp_speed[leg][0] = temp_speed[leg][1] = temp_speed[leg][2] = 0;
  } else {
    temp_speed[leg][0] = length_x / length * move_speed * speed_multiple;
    temp_speed[leg][1] = length_y / length * move_speed * speed_multiple;
    temp_speed[leg][2] = length_z / length * move_speed * speed_multiple;
  }

  if (x != KEEP) site_expect[leg][0] = x;
  if (y != KEEP) site_expect[leg][1] = y;
  if (z != KEEP) site_expect[leg][2] = z;
}

// -------------------- custom_set_site + wait --------------------
void custom_set_site(int leg, float x, float y, float z) {
  if (x != KEEP) site_expect[leg][0] = x;
  if (y != KEEP) site_expect[leg][1] = y;
  if (z != KEEP) site_expect[leg][2] = z;

  float length_x = (x != KEEP) ? (x - site_now[leg][0]) : 0;
  float length_y = (y != KEEP) ? (y - site_now[leg][1]) : 0;
  float length_z = (z != KEEP) ? (z - site_now[leg][2]) : 0;
  float length   = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));

  if (length == 0) {
    temp_speed[leg][0] = temp_speed[leg][1] = temp_speed[leg][2] = 0;
    return;
  }

  temp_speed[leg][0] = (x != KEEP) ? length_x / length * move_speed * speed_multiple : 0;
  temp_speed[leg][1] = (y != KEEP) ? length_y / length * move_speed * speed_multiple : 0;
  temp_speed[leg][2] = (z != KEEP) ? length_z / length * move_speed * speed_multiple : 0;

  if (temp_speed[leg][0] > max_speed) temp_speed[leg][0] = max_speed;
  if (temp_speed[leg][1] > max_speed) temp_speed[leg][1] = max_speed;
  if (temp_speed[leg][2] > max_speed) temp_speed[leg][2] = max_speed;
}

void custom_wait_all_reach() {
  bool reached = false;
  while (!reached) {
    reached = true;
    for (int i = 0; i < 4; i++) {
      if (fabs(site_now[i][0] - site_expect[i][0]) > 0.1 ||
          fabs(site_now[i][1] - site_expect[i][1]) > 0.1 ||
          fabs(site_now[i][2] - site_expect[i][2]) > 0.1) {
        reached = false;
        break;
      }
    }
    delay(20);
  }
}

// -------------------- WALKING: LEFT / RIGHT / CHÓD: LEWO / PRAWO --------------------
void turn_left(unsigned int step) {
  move_speed = spot_turn_speed;
  while (step-- > 0) {
    if (site_now[3][1] == y_start) {
      custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(1, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(2, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(3, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(3, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(1, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_set_site(2, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(3, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(1, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(1, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_set_site(2, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(3, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(1, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    } else {
      custom_set_site(0, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_set_site(1, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(2, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(3, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(1, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(2, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_set_site(3, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(2, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    }
  }
}

void turn_right(unsigned int step) {
  move_speed = spot_turn_speed;
  while (step-- > 0) {
    bool frontLeftAtFront = fabs(site_now[2][1] - y_start) < 1.0;
    if (frontLeftAtFront) {
      custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(1, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(2, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_set_site(3, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(2, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_set_site(1, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(2, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(3, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_set_site(1, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(2, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(3, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    } else {
      custom_set_site(1, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(1, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_set_site(2, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(3, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(1, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(0, turn_x1 - x_offset, turn_y1, bodyZCurrent);
      custom_set_site(1, turn_x0 - x_offset, turn_y0, bodyZCurrent);
      custom_set_site(2, turn_x1 + x_offset, turn_y1, bodyZCurrent);
      custom_set_site(3, turn_x0 + x_offset, turn_y0, bodyZCurrent);
      custom_wait_all_reach();

      custom_set_site(3, turn_x0 + x_offset, turn_y0, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();

      custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    }
  }
}

// -------------------- WALKING: FORWARD / BACK / CHÓD: PRZÓD / TYŁ --------------------
void step_forward(unsigned int step) {
  move_speed = leg_move_speed;
  while (step-- > 0) {
    if (site_now[2][1] == y_start) {
      custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(2, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(2, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = body_move_speed;

      custom_set_site(0, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(1, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_set_site(2, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(3, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = leg_move_speed;

      custom_set_site(1, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(1, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(1, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    } else {
      custom_set_site(0, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(0, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(0, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = body_move_speed;

      custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(3, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = leg_move_speed;

      custom_set_site(3, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    }
  }
}

void step_back(unsigned int step) {
  move_speed = leg_move_speed;
  while (step-- > 0) {
    if (site_now[3][1] == y_start) {
      custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(3, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(3, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = body_move_speed;

      custom_set_site(0, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_set_site(1, x_default + x_offset, y_start, bodyZCurrent);
      custom_set_site(2, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(3, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = leg_move_speed;

      custom_set_site(0, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(0, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(0, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    } else {
      custom_set_site(1, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(1, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(1, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = body_move_speed;

      custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
      custom_set_site(2, x_default + x_offset, y_start + 2 * y_step, bodyZCurrent);
      custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();

      move_speed = leg_move_speed;

      custom_set_site(2, x_default + x_offset, y_start + 2 * y_step, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
      custom_wait_all_reach();
      custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
      custom_wait_all_reach();
    }
  }
}

// -------------------- SMOOTH TILTS / PŁYNNE PRZECHYŁY --------------------
void body_left(int i) {
  move_speed = body_move_speed;
  const int parts = 3;
  float dx = i / (float)parts;

  float x0_0 = site_now[0][0];
  float x0_1 = site_now[1][0];
  float x0_2 = site_now[2][0];
  float x0_3 = site_now[3][0];

  for (int p = 0; p < parts; ++p) {
    float f = (p + 1) * dx;
    custom_set_site(0, x0_0 + f, KEEP, KEEP);
    custom_set_site(1, x0_1 + f, KEEP, KEEP);
    custom_set_site(2, x0_2 - f, KEEP, KEEP);
    custom_set_site(3, x0_3 - f, KEEP, KEEP);
    custom_wait_all_reach();
  }
}

void body_right(int i) {
  move_speed = body_move_speed;
  const int parts = 3;
  float dx = i / (float)parts;

  float x0_0 = site_now[0][0];
  float x0_1 = site_now[1][0];
  float x0_2 = site_now[2][0];
  float x0_3 = site_now[3][0];

  for (int p = 0; p < parts; ++p) {
    float f = (p + 1) * dx;
    custom_set_site(0, x0_0 - f, KEEP, KEEP);
    custom_set_site(1, x0_1 - f, KEEP, KEEP);
    custom_set_site(2, x0_2 + f, KEEP, KEEP);
    custom_set_site(3, x0_3 + f, KEEP, KEEP);
    custom_wait_all_reach();
  }
}

void body_front(int i) {
  move_speed = body_move_speed;
  const int parts = 3;
  float dy = i / (float)parts;

  float y0_0 = site_now[0][1];
  float y0_1 = site_now[1][1];
  float y0_2 = site_now[2][1];
  float y0_3 = site_now[3][1];

  for (int p = 0; p < parts; ++p) {
    float f = (p + 1) * dy;
    custom_set_site(0, KEEP, y0_0 - f, KEEP);
    custom_set_site(2, KEEP, y0_2 - f, KEEP);
    custom_set_site(1, KEEP, y0_1 + f, KEEP);
    custom_set_site(3, KEEP, y0_3 + f, KEEP);
    custom_wait_all_reach();
  }
}

void body_back(int i) {
  move_speed = body_move_speed;
  const int parts = 3;
  float dy = i / (float)parts;

  float y0_0 = site_now[0][1];
  float y0_1 = site_now[1][1];
  float y0_2 = site_now[2][1];
  float y0_3 = site_now[3][1];

  for (int p = 0; p < parts; ++p) {
    float f = (p + 1) * dy;
    custom_set_site(0, KEEP, y0_0 + f, KEEP);
    custom_set_site(2, KEEP, y0_2 + f, KEEP);
    custom_set_site(1, KEEP, y0_1 - f, KEEP);
    custom_set_site(3, KEEP, y0_3 - f, KEEP);
    custom_wait_all_reach();
  }
}

// Reset body position to neutral while preserving height / Reset korpusu do pozycji neutralnej z zachowaniem wysokości
void reset_body_pose() {
  move_speed = body_move_speed;
  custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
  custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
  custom_set_site(2, x_default + x_offset, y_start,         bodyZCurrent);
  custom_set_site(3, x_default + x_offset, y_start,         bodyZCurrent);
  custom_wait_all_reach();
}

// -------------------- ARM / PAW / RĘKA / ŁAPA --------------------
void hand_wave(int i) {
  float x_tmp, y_tmp, z_tmp;
  move_speed = 1;
  if (site_now[3][1] == y_start) {
    body_right(15);
    x_tmp = site_now[2][0];
    y_tmp = site_now[2][1];
    z_tmp = site_now[2][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++) {
      custom_set_site(2, turn_x1, turn_y1, 50);
      custom_wait_all_reach();
      custom_set_site(2, turn_x0, turn_y0, 50);
      custom_wait_all_reach();
    }
    custom_set_site(2, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_left(15);
  } else {
    body_left(15);
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++) {
      custom_set_site(0, turn_x1, turn_y1, 50);
      custom_wait_all_reach();
      custom_set_site(0, turn_x0, turn_y0, 50);
      custom_wait_all_reach();
    }
    custom_set_site(0, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

void hand_shake(int i) {
  float x_tmp, y_tmp, z_tmp;
  move_speed = 1;
  if (site_now[3][1] == y_start) {
    body_right(15);
    x_tmp = site_now[2][0];
    y_tmp = site_now[2][1];
    z_tmp = site_now[2][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++) {
      custom_set_site(2, x_default - 30, y_start + 2 * y_step, 55);
      custom_wait_all_reach();
      custom_set_site(2, x_default - 30, y_start + 2 * y_step, 10);
      custom_wait_all_reach();
    }
    custom_set_site(2, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_left(15);
  } else {
    body_left(15);
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++) {
      custom_set_site(0, x_default - 30, y_start + 2 * y_step, 55);
      custom_wait_all_reach();
      custom_set_site(0, x_default - 30, y_start + 2 * y_step, 10);
      custom_wait_all_reach();
    }
    custom_set_site(0, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

// -------------------- COMPOUND MOVES / RUCHY ZŁOŻONE --------------------
void move_right(int steps) {
  while (steps-- > 0) {
    turn_right(1);
    step_forward(1);
    turn_left(1);
  }
}

void move_left(int steps) {
  while (steps-- > 0) {
    turn_left(1);
    step_forward(1);
    turn_right(1);
  }
}

void rear_legs_wave(int steps) {
  for (int i = 0; i < steps; i++) {
    custom_set_site(2, KEEP, KEEP, bodyZUpCurrent);
    custom_wait_all_reach();
    scaledDelay(200);

    custom_set_site(2, KEEP, KEEP, bodyZCurrent);
    custom_wait_all_reach();
    scaledDelay(200);

    custom_set_site(3, KEEP, KEEP, bodyZUpCurrent);
    custom_wait_all_reach();
    scaledDelay(200);

    custom_set_site(3, KEEP, KEEP, bodyZCurrent);
    custom_wait_all_reach();
    scaledDelay(200);
  }
}

void rear_leg_wave2(int i) {
  float x_tmp, y_tmp, z_tmp;
  move_speed = 1;

  if (site_now[1][1] == y_start) {
    body_right(15);
    x_tmp = site_now[3][0];
    y_tmp = site_now[3][1];
    z_tmp = site_now[3][2];

    move_speed = body_move_speed;

    for (int j = 0; j < i; j++) {
      custom_set_site(3, turn_x1, turn_y1, 50);
      custom_wait_all_reach();
      custom_set_site(3, turn_x0, turn_y0, 50);
      custom_wait_all_reach();
    }

    custom_set_site(3, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_left(15);
  } else {
    body_left(15);
    x_tmp = site_now[1][0];
    y_tmp = site_now[1][1];
    z_tmp = site_now[1][2];

    move_speed = body_move_speed;

    for (int j = 0; j < i; j++) {
      custom_set_site(1, turn_x1, turn_y1, 50);
      custom_wait_all_reach();
      custom_set_site(1, turn_x0, turn_y0, 50);
      custom_wait_all_reach();
    }

    custom_set_site(1, x_tmp, y_tmp, z_tmp);
    custom_wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

void jump_forward() {
  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
  custom_wait_all_reach();

  for (int leg = 0; leg < 4; leg++)
    custom_set_site(leg, site_now[leg][0] - 30, KEEP, bodyZUpCurrent);
  custom_wait_all_reach();

  for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
}

void spin_on_leg(int leg_to_stay, int steps) {
  for (int leg = 0; leg < 4; leg++) {
    if (leg != leg_to_stay) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
  }
  custom_wait_all_reach();

  for (int i = 0; i < steps; i++) {
    for (int leg = 0; leg < 4; leg++) {
      if (leg != leg_to_stay) {
        custom_set_site(leg, site_now[leg][0] + 20, site_now[leg][1], bodyZUpCurrent);
      }
    }
    custom_wait_all_reach();
  }

  for (int leg = 0; leg < 4; leg++) {
    if (leg != leg_to_stay) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
  }
  custom_wait_all_reach();
}

void dance_step(int steps) {
  move_speed = leg_move_speed / 2;

  for (int i = 0; i < steps; i++) {
    custom_set_site(0, site_now[0][0], site_now[0][1] + 10, bodyZUpCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] - 10, bodyZUpCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] + 10, bodyZUpCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] - 10, bodyZUpCurrent);
    custom_wait_all_reach();

    custom_set_site(0, site_now[0][0], site_now[0][1] + 10, bodyZCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] - 10, bodyZCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] + 10, bodyZCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] - 10, bodyZCurrent);
    custom_wait_all_reach();
  }

  move_speed = leg_move_speed;
}

// -------------------- SQUATS / PRZYSIADY --------------------
void squat_back_legs() {
  float squatHeight = bodyZCurrent - 20;

  custom_set_site(1, KEEP, KEEP, squatHeight);
  custom_set_site(3, KEEP, KEEP, squatHeight);
  custom_wait_all_reach();

  scaledDelay(1400);

  custom_set_site(1, KEEP, KEEP, bodyZCurrent);
  custom_set_site(3, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
}

void squat_back_legs_stop() {
  float squatHeight = z_boot_base;

  custom_set_site(1, KEEP, KEEP, squatHeight);
  custom_set_site(3, KEEP, KEEP, squatHeight);
  custom_wait_all_reach();

  scaledDelay(1400);

  custom_set_site(1, KEEP, KEEP, squatHeight);
  custom_set_site(3, KEEP, KEEP, squatHeight);
  custom_wait_all_reach();
}

void squat_front_legs() {
  float squatHeight = bodyZUpCurrent;

  custom_set_site(0, KEEP, KEEP, squatHeight);
  custom_set_site(2, KEEP, KEEP, squatHeight);
  custom_wait_all_reach();

  scaledDelay(1400);

  custom_set_site(0, KEEP, KEEP, bodyZCurrent);
  custom_set_site(2, KEEP, KEEP, bodyZCurrent);
  custom_wait_all_reach();
}

// -------------------- SHOW MOVES / RUCHY POKAZOWE --------------------
void moonwalk(int steps) {
  for (int i = 0; i < steps; i++) {
    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
    custom_wait_all_reach();

    for (int leg = 0; leg < 4; leg++)
      custom_set_site(leg, x_default + (leg % 2 == 0 ? 20 : -20), KEEP, bodyZUpCurrent);
    custom_wait_all_reach();

    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, x_default, KEEP, bodyZCurrent);
    custom_wait_all_reach();
  }
}

void circle_walk(int steps) {
  for (int i = 0; i < steps; i++) {
    for (int leg = 0; leg < 4; leg++) {
      float angle = (i * 90.0 / steps) * (pi / 180.0);
      float newX  = x_default + cos(angle) * 20;
      float newY  = y_start + sin(angle) * 20;
      custom_set_site(leg, newX, newY, bodyZUpCurrent);
    }
    custom_wait_all_reach();

    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, x_default, y_start, bodyZCurrent);
    custom_wait_all_reach();
  }
}

void sidewalk(int steps) {
  for (int i = 0; i < steps; i++) {
    custom_set_site(0, site_now[0][0], site_now[0][1] - 20, bodyZUpCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] + 20, bodyZUpCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] - 20, bodyZUpCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] + 20, bodyZUpCurrent);
    custom_wait_all_reach();

    custom_set_site(0, site_now[0][0], site_now[0][1] - 20, bodyZCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] + 20, bodyZCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] - 20, bodyZCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] + 20, bodyZCurrent);
    custom_wait_all_reach();

    custom_set_site(0, site_now[0][0], site_now[0][1] + 20, bodyZUpCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] - 20, bodyZUpCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] + 20, bodyZUpCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] - 20, bodyZUpCurrent);
    custom_wait_all_reach();

    custom_set_site(0, site_now[0][0], site_now[0][1] + 20, bodyZCurrent);
    custom_set_site(1, site_now[1][0], site_now[1][1] - 20, bodyZCurrent);
    custom_set_site(2, site_now[2][0], site_now[2][1] + 20, bodyZCurrent);
    custom_set_site(3, site_now[3][0], site_now[3][1] - 20, bodyZCurrent);
    custom_wait_all_reach();
  }
}

void wave_walk(int steps) {
  for (int i = 0; i < steps; i++) {
    for (int leg = 0; leg < 4; leg++) {
      custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
      custom_wait_all_reach();
      scaledDelay(180);
    }
    for (int leg = 0; leg < 4; leg++) {
      custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
      custom_wait_all_reach();
      scaledDelay(180);
    }
  }
}
void strafe_left(unsigned int step) {
  // Strafe left: the robot moves left without turning / Robot jedzie w lewo bez obracania
  // Legs move sideways on the X axis / Nogi przesuwają się bocznie w osi X
  move_speed = leg_move_speed;
  
  while (step-- > 0) {
    // Phase 1: lift legs 0 and 2 (front right and front left), then move left / Unieś przednie nogi i przesuń w lewo
    custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZUpCurrent);
    custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Move the lifted legs left (decrease X) / Przesuń uniesione nogi w lewo
    custom_set_site(0, x_default - x_offset - 25, y_start + y_step, bodyZUpCurrent);
    custom_set_site(2, x_default + x_offset - 25, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Lower the legs / Opuść nogi
    custom_set_site(0, x_default - x_offset - 25, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset - 25, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    // Phase 2: move the body left while legs 1 and 3 push / Przesuń ciało w lewo
    move_speed = body_move_speed;
    custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(1, x_default - x_offset - 25, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
    custom_set_site(3, x_default + x_offset - 25, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    move_speed = leg_move_speed;
    
    // Phase 3: lift legs 1 and 3 (rear right and rear left), then move left / Unieś tylne nogi i przesuń w lewo
    custom_set_site(1, x_default - x_offset - 25, y_start + y_step, bodyZUpCurrent);
    custom_set_site(3, x_default + x_offset - 25, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Move the lifted legs left / Przesuń uniesione nogi w lewo
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZUpCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Lower the legs / Opuść nogi
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    // Phase 4: move the body left while legs 0 and 2 push / Przesuń ciało w lewo
    move_speed = body_move_speed;
    custom_set_site(0, x_default - x_offset - 25, y_start + y_step, bodyZCurrent);
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset - 25, y_start, bodyZCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    move_speed = leg_move_speed;
  }
}

void strafe_right(unsigned int step) {
  // Strafe right: the robot moves right without turning / Robot jedzie w prawo bez obracania
  // Legs move sideways on the X axis / Nogi przesuwają się bocznie w osi X
  move_speed = leg_move_speed;
  
  while (step-- > 0) {
    // Phase 1: lift legs 0 and 2 (front right and front left), then move right / Unieś przednie nogi i przesuń w prawo
    custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZUpCurrent);
    custom_set_site(2, x_default + x_offset, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Move the lifted legs right (increase X) / Przesuń uniesione nogi w prawo
    custom_set_site(0, x_default - x_offset + 25, y_start + y_step, bodyZUpCurrent);
    custom_set_site(2, x_default + x_offset + 25, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Lower the legs / Opuść nogi
    custom_set_site(0, x_default - x_offset + 25, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset + 25, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    // Phase 2: move the body right while legs 1 and 3 push / Przesuń ciało w prawo
    move_speed = body_move_speed;
    custom_set_site(0, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(1, x_default - x_offset + 25, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset, y_start, bodyZCurrent);
    custom_set_site(3, x_default + x_offset + 25, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    move_speed = leg_move_speed;
    
    // Phase 3: lift legs 1 and 3 (rear right and rear left), then move right / Unieś tylne nogi i przesuń w prawo
    custom_set_site(1, x_default - x_offset + 25, y_start + y_step, bodyZUpCurrent);
    custom_set_site(3, x_default + x_offset + 25, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Move the lifted legs right / Przesuń uniesione nogi w prawo
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZUpCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZUpCurrent);
    custom_wait_all_reach();
    
    // Lower the legs / Opuść nogi
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    // Phase 4: move the body right while legs 0 and 2 push / Przesuń ciało w prawo
    move_speed = body_move_speed;
    custom_set_site(0, x_default - x_offset + 25, y_start + y_step, bodyZCurrent);
    custom_set_site(1, x_default - x_offset, y_start + y_step, bodyZCurrent);
    custom_set_site(2, x_default + x_offset + 25, y_start, bodyZCurrent);
    custom_set_site(3, x_default + x_offset, y_start, bodyZCurrent);
    custom_wait_all_reach();
    
    move_speed = leg_move_speed;
  }
}
// -------------------- INIT ZERO --------------------
void initZero() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      float angle = 90 + calValues[i][j];
      setServoAngle(i, j, angle);
      delay(20);
    }
  }
}

// -------------------- SERVO SERVICE (IK) --------------------
void servo_service(void *parm) {
  (void)parm;
  while (1) {
    if (calibrationMode || !pcaReady) {
      delay(20);
      continue;
    }

    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 3; j++) {
        if (fabs(site_now[i][j] - site_expect[i][j]) >= fabs(temp_speed[i][j]))
          site_now[i][j] += temp_speed[i][j];
        else
          site_now[i][j] = site_expect[i][j];
      }

      cartesian_to_polar(g_alpha, g_beta, g_gamma, site_now[i][0], site_now[i][1], site_now[i][2]);
      polar_to_servo(i, g_alpha, g_beta, g_gamma);
    }

    rest_counter++;
    delay(20);
  }
}

// -------------------- KINEMATICS / KINEMATYKA --------------------
void cartesian_to_polar(volatile float &a, volatile float &b, volatile float &c,
                        float x, float y, float z) {
  float v, w;
  w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
  v = w - length_c;
  a = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) /
                               2 / length_a / sqrt(pow(v, 2) + pow(z, 2)));
  b  = acos((pow(length_a, 2) + pow(length_b, 2) - pow(v, 2) - pow(z, 2)) /
                 2 / length_a / length_b);

  c = (w >= 0) ? atan2(y, x) : atan2(-y, -x);

  a = a / pi * 180;
  b = b / pi * 180;
  c = c / pi * 180;
}

void polar_to_servo(int leg, float alpha, float beta, float gamma) {
  if (leg == 0) {
    g_alpha = 90 - alpha;
    g_beta  = beta;
    g_gamma = gamma + 90;
  } else if (leg == 1) {
    g_alpha = alpha + 90;
    g_beta  = 180 - beta;
    g_gamma = 90 - gamma;
  } else if (leg == 2) {
    g_alpha = alpha + 90;
    g_beta  = 180 - beta;
    g_gamma = 90 - gamma;
  } else if (leg == 3) {
    g_alpha = 90 - alpha;
    g_beta  = beta;
    g_gamma = gamma + 90;
  }

  float a = g_alpha + calValues[leg][0];
  float b = g_beta  + calValues[leg][1];
  float c = g_gamma + calValues[leg][2];

  setServoAngle(leg, 0, a);
  setServoAngle(leg, 1, b);
  setServoAngle(leg, 2, c);
}

// -------------------- TRICK COMBOS / KOMBINACJE TRIKÓW --------------------
void trick_bow_hello() {
  for (int i = 0; i < 2; i++) {
    squat_front_legs();
    scaledDelay(200);
  }
}

void trick_wave_combo() {
  hand_wave(3);
  scaledDelay(200);
  rear_legs_wave(2);
}

void trick_double_jump() {
  jump_forward();
  scaledDelay(160);
  jump_forward();
}

void trick_happy_dance() {
  sway_body();
  dance_step(2);
  rear_legs_wave(2);
}

void trick_idle_breath() {
  move_speed = stand_seat_speed;

  const int cycles = 3;
  for (int c = 0; c < cycles; c++) {
    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZUpCurrent);
    custom_wait_all_reach();
    scaledDelay(220);

    for (int leg = 0; leg < 4; leg++) custom_set_site(leg, KEEP, KEEP, bodyZCurrent);
    custom_wait_all_reach();
    scaledDelay(220);
  }
}

void trick_hello_combo() {
  trick_bow_hello();
  scaledDelay(220);
  hand_wave(3);
}

void trick_guard_scan() {
  const int cycles = 2;
  for (int i = 0; i < cycles; i++) {
    turn_left(1);
    scaledDelay(120);
    turn_right(2);
    scaledDelay(120);
    turn_left(1);
    scaledDelay(120);
  }
}
