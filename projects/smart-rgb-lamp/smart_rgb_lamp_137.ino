/*
  SMART RGB LAMP – Marcin / Maker Portfolio
  -----------------------------------------
  REAL LAMP GEOMETRY:
    LED 0..99    = BODY / main vertical lamp section (100 LEDs)
    LED 100..136 = PLUME / "pióropusz" (37 LEDs)
    TOTAL        = 137 LEDs on ONE NeoPixel data line

  Public version: replace credentials before upload.
  Full source includes local WWW control, OLED, OTA and SinricPro/Alexa.
*/

// Download the complete current source from:
// https://stapowiczmarcin-sys.github.io/projects/smart-rgb-lamp/smart_rgb_lamp_137_full.ino

// Geometry constants kept here intentionally so the physical lamp is never
// accidentally treated as one 137-LED linear section.
#define BODY_START 0
#define BODY_COUNT 100
#define PLUME_START 100
#define PLUME_COUNT 37
#define NUMPIXELS 137
