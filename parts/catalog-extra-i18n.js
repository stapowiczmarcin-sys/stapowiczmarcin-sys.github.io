"use strict";

window.CATALOG_I18N = window.CATALOG_I18N || { categories: {}, projects: {}, statuses: {}, parts: {} };
window.CATALOG_I18N.categories = window.CATALOG_I18N.categories || {};
window.CATALOG_I18N.parts = window.CATALOG_I18N.parts || {};

Object.assign(window.CATALOG_I18N.categories, {
  "Narzędzia": "Tools"
});

Object.assign(window.CATALOG_I18N.parts, {
  "uni-t-ut61b-plus": {
    name: "UNI-T UT61B+ — True RMS digital multimeter",
    short: "Workshop meter for electronics and power diagnostics",
    spec: "6000 counts • True RMS • USB • NCV • V / mV / A / mA / µA • Hz / % • continuity / diode • REL • MAX/MIN • HOLD"
  }
});
