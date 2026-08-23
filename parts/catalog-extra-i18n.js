"use strict";

window.CATALOG_I18N = window.CATALOG_I18N || { categories: {}, projects: {}, statuses: {}, parts: {} };
window.CATALOG_I18N.categories = window.CATALOG_I18N.categories || {};
window.CATALOG_I18N.parts = window.CATALOG_I18N.parts || {};

Object.assign(window.CATALOG_I18N.categories, {
  "Narzędzia": "Tools",
  "Montaż": "Mounting"
});

Object.assign(window.CATALOG_I18N.parts, {
  "cam3": {
    name: "Raspberry Pi Camera Module 3 Wide",
    short: "Wide-angle daytime camera with autofocus",
    spec: "Sony IMX708 • 12 MP • autofocus • wide field of view"
  },
  "cam3-noir": {
    name: "Raspberry Pi Camera Module 3 Wide NoIR",
    short: "Wide-angle night camera without an IR filter",
    spec: "Sony IMX708 • 12 MP • autofocus • wide field of view • infrared sensitivity"
  },
  "uni-t-ut61b-plus": {
    name: "UNI-T UT61B+ — True RMS digital multimeter",
    short: "Workshop meter for electronics and power diagnostics",
    spec: "6000 counts • True RMS • USB • NCV • V / mV / A / mA / µA • Hz / % • continuity / diode • REL • MAX/MIN • HOLD"
  },
  "bgtxingi-m25-standoff-kit-280": {
    name: "M2.5 standoff kit — 503 pieces",
    short: "Brass standoffs, screws and nuts for mounting Raspberry Pi, UPS and Hailo",
    spec: "503 pieces • M2.5 thread • brass male-female and female-female standoffs • screws and nuts"
  },
  "neewer-lm2": {
    name: "NEEWER LM2 — USB lavalier microphone",
    short: "Tested microphone for voice and video recording",
    spec: "USB-A • omnidirectional microphone • 2 m cable • 3.5 mm monitoring jack • plug & play"
  },
  "dc-pigtail-female-5521-2pk": {
    name: "sourcing map — 5.5 × 2.1 mm female DC pigtail, 2-pack",
    short: "Female barrel jack leads for detachable power connections",
    spec: "2 pieces • female 5.5 × 2.1 mm • 30 cm cable • bare-wire ends"
  },
  "charger-2s-84v-1a-uk": {
    name: "8.4 V / 1 A UK charger for a 2S pack",
    short: "External charger for 2S Li-ion packs through the correct BMS",
    spec: "UK AC input • 8.4 V DC output • 1000 mA • 2S 18650 packs"
  },
  "qury-bms-2s-20a-balanced-5pk": {
    name: "Qury — 2S 20 A balancing BMS, 5-pack",
    short: "Protection and balancing board for a 2S Li-ion pack",
    spec: "5 pieces • 2S • 7.4 V nominal / 8.4 V max • claimed 20 A • balancing"
  },
  "dc-pigtail-male-5525-5pk": {
    name: "sourcing map — 5.5 × 2.5 mm male DC pigtail, 5-pack",
    short: "Male barrel plug leads for power connections",
    spec: "5 pieces • male 5.5 × 2.5 mm • 50 cm cable • bare-wire ends"
  }
});
