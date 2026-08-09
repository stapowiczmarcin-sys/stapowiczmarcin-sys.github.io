/*
  PROSTE RADIO INTERNETOWE
  ESP32-WROOM + PCM5102A + 3 czujniki IR + pasek RGB WS2812B

  IR 1 (GPIO32) -> nastepna stacja
  IR 2 (GPIO33) -> radio ON/OFF
  IR 3 (GPIO14) -> LED ON/OFF

  Biblioteki:
  - ESP32-audioI2S 4.0.0 (schreibfaul1)
  - Adafruit NeoPixel 1.15.5
*/

#include <WiFi.h>
#include <Audio.h>
#include <Adafruit_NeoPixel.h>

// 1. WPISZ SWOJE WIFI
const char* WIFI_SSID     = "WPISZ_NAZWE_WIFI";
const char* WIFI_PASSWORD = "WPISZ_HASLO_WIFI";

// 2. PINY PCM5102A
#define I2S_BCLK 27   // ESP32 GPIO27 -> PCM5102A BCK
#define I2S_LRC  26   // ESP32 GPIO26 -> PCM5102A LCK/LRCK
#define I2S_DOUT 25   // ESP32 GPIO25 -> PCM5102A DIN

// 3. TRZY CZUJNIKI IR (cyfrowe OUT, aktywne stanem LOW)
#define IR_STATION 32
#define IR_POWER   33
#define IR_LEDS    14

// 4. PASEK ADRESOWALNY RGB WS2812B / NEOPIXEL
#define LED_PIN 4
#define LED_COUNT 82                 // wpisz prawdziwa liczbe diod
#define LED_TYPE (NEO_GRB + NEO_KHZ800) // RGB bez osobnego kanalu W

struct Station {
  const char* name;
  const char* stream;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// Bezposrednie strumienie MP3, nie zwykle strony internetowe.
Station stations[] = {
  {"Groove Salad",     "http://ice5.somafm.com/groovesalad-128-mp3",  10, 110, 255},
  {"Drone Zone",       "http://ice5.somafm.com/dronezone-128-mp3",    95,  35, 255},
  {"Indie Pop Rocks!", "http://ice5.somafm.com/indiepop-128-mp3",   255,  45,  95}
};

const uint8_t STATION_COUNT = sizeof(stations) / sizeof(stations[0]);

Audio audio;
Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, LED_TYPE);

uint8_t stationNumber = 0;
bool radioOn = true;
bool ledsOn = true;

// Zapamietuje stan czujnikow, aby jeden gest dal jedno polecenie.
bool previousIrState[3] = {false, false, false};
unsigned long lastIrAction = 0;

bool irGesture(uint8_t pin, uint8_t number) {
  bool active = (digitalRead(pin) == LOW);
  bool newGesture = active && !previousIrState[number];
  previousIrState[number] = active;

  if (newGesture && millis() - lastIrAction > 350) {
    lastIrAction = millis();
    return true;
  }
  return false;
}

void updateLeds() {
  if (!radioOn || !ledsOn) {
    leds.clear();
  } else {
    Station& station = stations[stationNumber];
    leds.fill(leds.Color(station.red, station.green, station.blue));
  }
  leds.show();
}

void playStation() {
  audio.stopSong();
  audio.connecttohost(stations[stationNumber].stream);
  updateLeds();

  Serial.print("Stacja: ");
  Serial.println(stations[stationNumber].name);
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_STATION, INPUT_PULLUP);
  pinMode(IR_POWER, INPUT_PULLUP);
  pinMode(IR_LEDS, INPUT_PULLUP);

  leds.begin();
  leds.setBrightness(40); // ogranicza jasnosc i pobor pradu
  leds.clear();
  leds.show();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Laczenie z Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
  }
  Serial.println(" OK");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // zakres biblioteki: 0..21
  playStation();
}

void loop() {
  audio.loop(); // musi wykonywac sie caly czas

  if (irGesture(IR_STATION, 0)) {
    stationNumber = (stationNumber + 1) % STATION_COUNT;
    if (radioOn) playStation();
  }

  if (irGesture(IR_POWER, 1)) {
    radioOn = !radioOn;
    if (radioOn) playStation();
    else {
      audio.stopSong();
      updateLeds();
    }
  }

  if (irGesture(IR_LEDS, 2)) {
    ledsOn = !ledsOn;
    updateLeds();
  }

  vTaskDelay(1);
}

