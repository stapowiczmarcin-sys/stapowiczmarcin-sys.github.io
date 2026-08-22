"use strict";

window.CATALOG_I18N = window.CATALOG_I18N || { categories: {}, projects: {}, statuses: {}, parts: {} };
window.CATALOG_I18N.categories = window.CATALOG_I18N.categories || {};
window.CATALOG_I18N.parts = window.CATALOG_I18N.parts || {};

Object.assign(window.CATALOG_I18N.categories, {
  "Narzędzia": "Tools",
  "Montaż": "Mounting"
});

Object.assign(window.CATALOG_I18N.parts, {
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
  }
});
