"use strict";

window.PARTS_CATALOG = window.PARTS_CATALOG || [];

// Central runtime overrides for catalogue entries whose store / affiliate link changes.
const catalogueOverrides = {
  "cam3": {
    name: "Raspberry Pi Camera Module 3 Wide",
    short: "Szerokokątna kamera dzienna z autofokusem",
    supplier: "Amazon",
    spec: "Sony IMX708 • 12 MP • autofocus • szerokie pole widzenia",
    url: "https://amzn.to/4coJbtR",
    youtubePl: "Obraz dzienny Kory zapewnia szerokokątna Raspberry Pi Camera Module 3 Wide z autofokusem i sensorem IMX708.",
    youtubeEn: "Kora's daytime vision uses a wide-angle Raspberry Pi Camera Module 3 Wide with autofocus and an IMX708 sensor."
  },
  "cam3-noir": {
    supplier: "Amazon",
    url: "https://amzn.to/4xgsdpC"
  }
};

Object.entries(catalogueOverrides).forEach(([id, override]) => {
  const part = window.PARTS_CATALOG.find((item) => item.id === id);
  if (part) Object.assign(part, override);
});

const daytimeCamera = window.PARTS_CATALOG.find((item) => item.id === "cam3");
if (daytimeCamera) delete daytimeCamera.price;

if (!window.PARTS_CATALOG.some((part) => part.id === "uni-t-ut61b-plus")) {
  window.PARTS_CATALOG.push({
    "id": "uni-t-ut61b-plus",
    "name": "UNI-T UT61B+ — multimetr cyfrowy True RMS",
    "short": "Miernik warsztatowy do diagnostyki elektroniki i zasilania",
    "category": "Narzędzia",
    "supplier": "Amazon",
    "project": "Warsztat",
    "status": "Używam",
    "accent": "amber",
    "spec": "6000 counts • True RMS • USB • NCV • V / mV / A / mA / µA • Hz / % • ciągłość / dioda • REL • MAX/MIN • HOLD",
    "url": "https://www.amazon.co.uk/s?k=UNI-T%20UT61B%2B%20digital%20multimeter%20True%20RMS&tag=marcinkorapro-21",
    "youtubePl": "Do pomiarów i diagnostyki elektroniki używam multimetru UNI-T UT61B+ True RMS z 6000-countowym wyświetlaczem, pomiarem częstotliwości i funkcją NCV.",
    "youtubeEn": "For electronics measurements and diagnostics I use a UNI-T UT61B+ True RMS multimeter with a 6000-count display, frequency measurement and NCV."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "bgtxingi-m25-standoff-kit-280")) {
  window.PARTS_CATALOG.push({
    "id": "bgtxingi-m25-standoff-kit-280",
    "name": "Zestaw dystansów M2.5 — 503 elementy",
    "short": "Mosiężne dystanse, śruby i nakrętki do montażu Raspberry Pi, UPS i Hailo",
    "category": "Montaż",
    "supplier": "Amazon",
    "project": "Kora / VEGA",
    "status": "Mam",
    "accent": "slate",
    "spec": "503 elementy • gwint M2.5 • mosiężne dystanse męsko-żeńskie i żeńsko-żeńskie • śruby i nakrętki",
    "url": "https://amzn.to/4y1Rvbf",
    "youtubePl": "Do pewnego montażu Raspberry Pi, UPS-a i Hailo używam zestawu 503 elementów M2.5 z mosiężnymi dystansami oraz stalowymi śrubami i nakrętkami.",
    "youtubeEn": "For secure Raspberry Pi, UPS and Hailo mounting I use a 503-piece M2.5 kit with brass standoffs and stainless-steel screws and nuts."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "neewer-lm2")) {
  window.PARTS_CATALOG.push({
    "id": "neewer-lm2",
    "name": "NEEWER LM2 — mikrofon krawatowy USB",
    "short": "Sprawdzony mikrofon do nagrywania głosu i filmów",
    "category": "Audio",
    "supplier": "Amazon",
    "project": "Audio",
    "status": "Używam",
    "accent": "pink",
    "spec": "USB-A • mikrofon dookólny • przewód 2 m • odsłuch 3.5 mm • plug & play",
    "url": "https://amzn.to/4qr6Z5K",
    "youtubePl": "Do nagrywania głosu i filmów używam mikrofonu krawatowego NEEWER LM2 USB. U mnie sprawdził się dobrze — jest prosty, przewodowy i działa bez kombinowania.",
    "youtubeEn": "For voice and video recording I use the NEEWER LM2 USB lavalier microphone. It has worked well for me — simple, wired and plug-and-play."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "bms-13s-48v-mnine")) {
  window.PARTS_CATALOG.push({
    "id": "bms-13s-48v-mnine",
    "name": "BMS 13S / 48 V — rower elektryczny",
    "short": "Sprawdzony BMS do pakietu 13S użyty w M-Nine",
    "category": "Zasilanie",
    "supplier": "Amazon",
    "project": "M-Nine e-bike",
    "status": "Używam",
    "accent": "amber",
    "spec": "13S • 48 V nominalnie • balansowanie ogniw • B− / P− / C−",
    "url": "https://amzn.to/4g9cS4m",
    "youtubePl": "W akumulatorze roweru M-Nine używam sprawdzonego BMS-a 13S / 48 V. Przy montażu najważniejsze jest poprawne rozróżnienie B−, P− i C− oraz kolejności przewodów balansujących.",
    "youtubeEn": "In the M-Nine e-bike battery I use a tested 13S / 48 V BMS. Correct identification of B−, P− and C− and the balance-wire order is critical during installation."
  });
}


if (!window.PARTS_CATALOG.some((part) => part.id === "dc-pigtail-female-5521-2pk")) {
  window.PARTS_CATALOG.push({
    "id": "dc-pigtail-female-5521-2pk",
    "name": "sourcing map — przewód DC żeński 5.5 × 2.1 mm, 2 szt.",
    "short": "Żeńskie gniazdo baryłkowe na przewodzie do połączeń zasilania",
    "category": "Zasilanie",
    "supplier": "Amazon",
    "project": "Kora Brain",
    "status": "Mam",
    "accent": "amber",
    "spec": "2 szt. • żeńskie 5.5 × 2.1 mm • przewód 30 cm • otwarte końce",
    "url": "https://amzn.to/4gedq95",
    "youtubePl": "Żeńskie przewody DC 5,5 × 2,1 mm wykorzystuję do rozłącznych połączeń zasilania. Przed podłączeniem zawsze sprawdzam biegunowość miernikiem.",
    "youtubeEn": "I use 5.5 × 2.1 mm female DC pigtails for detachable power connections. I always verify polarity with a multimeter before connection."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "charger-2s-84v-1a-uk")) {
  window.PARTS_CATALOG.push({
    "id": "charger-2s-84v-1a-uk",
    "name": "Ładowarka UK 8.4 V / 1 A do pakietu 2S",
    "short": "Zewnętrzna ładowarka do pakietów Li-ion 2S przez właściwy BMS",
    "category": "Zasilanie",
    "supplier": "Amazon",
    "project": "Kora Brain",
    "status": "Mam",
    "accent": "amber",
    "spec": "wejście UK AC • wyjście 8.4 V DC • 1000 mA • pakiety 18650 2S",
    "url": "https://amzn.to/4gftVSk",
    "youtubePl": "Pakiet 2S ładuję zewnętrzną ładowarką 8,4 V / 1 A przez BMS. Przed użyciem sprawdzam napięcie wyjściowe, biegunowość i zgodność wtyku.",
    "youtubeEn": "I charge the 2S pack with an external 8.4 V / 1 A charger through the BMS. Before use I verify output voltage, polarity and plug compatibility."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "qury-bms-2s-20a-balanced-5pk")) {
  window.PARTS_CATALOG.push({
    "id": "qury-bms-2s-20a-balanced-5pk",
    "name": "Qury — BMS 2S 20 A z balansowaniem, 5 szt.",
    "short": "Płytka ochronna i balansująca do pakietu Li-ion 2S",
    "category": "Zasilanie",
    "supplier": "Amazon",
    "project": "Kora Brain",
    "status": "Mam",
    "accent": "amber",
    "spec": "5 szt. • 2S • 7.4 V nominalnie / 8.4 V maks. • deklarowane 20 A • balansowanie",
    "url": "https://amzn.to/4gteGnB",
    "youtubePl": "W pakiecie 2S stosuję płytkę BMS z balansowaniem. Oznaczenia pól i rzeczywisty limit prądu trzeba sprawdzić na konkretnej wersji płytki — nie zgadywać po samym tytule oferty.",
    "youtubeEn": "I use a balancing BMS board in the 2S pack. Terminal labels and the real current limit must be checked on the exact board version rather than assumed from the listing title."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "dc-pigtail-male-5525-5pk")) {
  window.PARTS_CATALOG.push({
    "id": "dc-pigtail-male-5525-5pk",
    "name": "sourcing map — przewód DC męski 5.5 × 2.5 mm, 5 szt.",
    "short": "Męski wtyk baryłkowy na przewodzie do połączeń zasilania",
    "category": "Zasilanie",
    "supplier": "Amazon",
    "project": "Warsztat",
    "status": "Mam",
    "accent": "amber",
    "spec": "5 szt. • męskie 5.5 × 2.5 mm • przewód 50 cm • otwarte końce",
    "url": "https://amzn.to/4hQwnzM",
    "youtubePl": "Przewód ma wtyk 5,5 × 2,5 mm. Nie opisuję go jako bezpośrednio zgodnego z X1203, którego oficjalne gniazdo ma 5,5 × 2,1 mm.",
    "youtubeEn": "This lead has a 5.5 × 2.5 mm plug. I do not list it as a direct X1203 lead because the official X1203 jack is 5.5 × 2.1 mm."
  });
}


const esp32Wroom38 = window.PARTS_CATALOG.find((part) => part.id === "esp32-wroom-usbc");
if (esp32Wroom38) {
  Object.assign(esp32Wroom38, {
    name: "AYWHP ESP32-WROOM-32 — 38 pin, USB-C",
    short: "Pełnowymiarowa płytka ESP32 używana w prototypach Kory",
    supplier: "Amazon",
    status: "Używam",
    spec: "38 pinów • 4 MB Flash • Wi-Fi 2,4 GHz • Bluetooth • CP2102 • USB-C",
    url: "https://amzn.to/3SeHqIP",
    youtubePl: "Do części prototypów Kory używam 38-pinowej płytki ESP32-WROOM-32 z USB-C, Wi-Fi i Bluetooth.",
    youtubeEn: "For some Kora prototypes I use a 38-pin ESP32-WROOM-32 board with USB-C, Wi-Fi and Bluetooth."
  });
  delete esp32Wroom38.price;
}

const quadrupedServo = window.PARTS_CATALOG.find((part) => part.id === "rds3115");
if (quadrupedServo) {
  Object.assign(quadrupedServo, {
    supplier: "Amazon",
    status: "Używałem",
    url: "https://amzn.to/4iws8cT",
    youtubePl: "W czteronożnej wersji Kory używałem dwuosiowych serw RDS3115MG z metalowymi przekładniami.",
    youtubeEn: "The four-legged Kora used RDS3115MG dual-shaft servos with metal gears."
  });
  delete quadrupedServo.price;
}

if (!window.PARTS_CATALOG.some((part) => part.id === "esp32-38pin-breakout-kit")) {
  window.PARTS_CATALOG.push({
    id: "esp32-38pin-breakout-kit",
    name: "ESP32 38-pin + terminal breakout board",
    short: "Płytka ESP32 wraz z podstawką wyprowadzającą zaciski",
    category: "Sterowanie",
    supplier: "Amazon",
    project: "Kora / VEGA",
    status: "Używam",
    accent: "green",
    spec: "ESP32-WROOM-32D • 38 pinów • CP2102 • Wi-Fi i Bluetooth • terminalowa podstawka",
    url: "https://amzn.to/3Urmwqs",
    youtubePl: "Do wygodnego prototypowania używam 38-pinowej płytki ESP32 wraz z terminalową podstawką, która upraszcza podłączanie przewodów.",
    youtubeEn: "For easier prototyping I use a 38-pin ESP32 board with a terminal breakout base that simplifies wiring."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "female-charging-lead")) {
  window.PARTS_CATALOG.push({
    id: "female-charging-lead",
    name: "Żeński przewód do ładowania",
    short: "Rozłączne połączenie ładowarki z układem zasilania",
    category: "Zasilanie",
    supplier: "Amazon",
    project: "Kora / VEGA",
    status: "Używam",
    accent: "amber",
    spec: "Żeńskie złącze zasilania • przewód do ładowania • przed użyciem sprawdzić średnicę wtyku i biegunowość",
    url: "https://amzn.to/4yatjU8",
    youtubePl: "Do rozłącznego podłączenia ładowarki używam żeńskiego przewodu zasilającego; przed podłączeniem sprawdzam biegunowość miernikiem.",
    youtubeEn: "I use a female power lead for a detachable charger connection and verify polarity with a multimeter before connecting it."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "oled12864-sh1106-13")) {
  window.PARTS_CATALOG.push({
    id: "oled12864-sh1106-13",
    name: "OLED 1.3″ 128×64 — SH1106",
    short: "Monochromatyczny ekran używany do informacji i prostych animacji",
    category: "Wyświetlacze",
    supplier: "Amazon",
    project: "Kora / VEGA",
    status: "Używam",
    accent: "blue",
    spec: "1,3 cala • 128×64 px • OLED • I²C • kontroler SH1106",
    url: "https://amzn.to/4gMEhIr",
    youtubePl: "Informacje i proste animacje wyświetlam na 1,3-calowym ekranie OLED 128×64 z kontrolerem SH1106.",
    youtubeEn: "I display information and simple animations on a 1.3-inch 128×64 OLED screen driven by the SH1106 controller."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "anycubic-photon-mono-4")) {
  window.PARTS_CATALOG.push({
    id: "anycubic-photon-mono-4",
    name: "ANYCUBIC Photon Mono 4 — drukarka żywiczna 10K",
    short: "Obecna drukarka 3D użyta do wykonania plastikowych nóg Kory",
    category: "Druk 3D",
    supplier: "Amazon",
    project: "Kora / VEGA",
    status: "Używam",
    accent: "violet",
    spec: "7-calowy ekran mono 10K • pole robocze 153,4 × 87 × 165 mm • druk żywiczny 405 nm",
    url: "https://amzn.to/4zN5Dqw",
    youtubePl: "Plastikowe nogi Kory wydrukowałem na żywicznej drukarce ANYCUBIC Photon Mono 4 z ekranem 10K.",
    youtubeEn: "I printed Kora's plastic legs on an ANYCUBIC Photon Mono 4 resin printer with a 10K screen."
  });
}

if (!window.PARTS_CATALOG.some((part) => part.id === "anycubic-standard-resin-black-1kg")) {
  window.PARTS_CATALOG.push({
    id: "anycubic-standard-resin-black-1kg",
    name: "ANYCUBIC Standard Resin — czarna, 1 kg",
    short: "Żywica użyta do wydrukowania plastikowych nóg Kory",
    category: "Druk 3D",
    supplier: "Amazon",
    project: "Kora / VEGA",
    status: "Używam",
    accent: "violet",
    spec: "405 nm • standardowa żywica fotopolimerowa • czarna • 1 kg • LCD / DLP / SLA",
    url: "https://amzn.to/4gnNt71",
    youtubePl: "Plastikowe nogi Kory wydrukowałem z czarnej standardowej żywicy ANYCUBIC 405 nm.",
    youtubeEn: "I printed Kora's plastic legs using black ANYCUBIC 405 nm standard resin."
  });
}

window.PART_IMAGES = window.PART_IMAGES || {};
window.PART_IMAGES["neewer-lm2"] = "https://de.neewer.com/cdn/shop/files/1_42e1f1da-e75d-44cc-be2b-aa2225eb4d2a.jpg?v=1746761338&width=1600";
