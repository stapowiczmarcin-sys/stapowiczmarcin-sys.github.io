"use strict";

(() => {
  const tabs = document.querySelector("#categoryTabs");
  if (!tabs) return;

  let gotchasActive = false;

  const copy = {
    pl: {
      tab: "🤬 DLACZEGO SIĘ WKURZYŁEM",
      index: "MARCINEK ODKRYŁ / PUŁAPKI SPRZĘTOWE",
      title: "Czego sprzedający i konstruktorzy nie przewidzieli.",
      lead: "Nie katalogowa teoria. Rzeczy, które wychodzą dopiero wtedy, kiedy naprawdę próbujesz to złożyć, przykręcić i uruchomić.",
      cards: [
        {
          kicker: "NA PAPIERZE: PASUJE. NA STOLE: AHA, JASNE.",
          title: "UPS X1203 + Raspberry Pi + HAT / Hailo — mechanika mówi: nie tak szybko.",
          body: "Sam UPS z Raspberry Pi da się złożyć sensownie. Problem zaczyna się przy dokładaniu HAT-a: w zestawie brakuje dodatkowych dystansów potrzebnych do złożenia pełnego stacka. M3 nie pasuje do mocowania UPS-a. M2 / M2.5 trzeba dobrać i potwierdzić gwint przed skręceniem — bez wciskania czegokolwiek na siłę.",
          discovery: "Odkrycie Marcinka",
          discoveryText: "Kompatybilność elektryczna ≠ kompatybilność mechaniczna. Zanim kupisz HAT, UPS i obudowę, sprawdź także wysokość stacka, dostęp do śrub i gwint dystansów.",
          workaround: "Obejście",
          workaroundText: "Dłuższe dystanse i osobny zestaw M2.5 są teraz w magazynie. Najpierw test gwintu, potem skręcanie. Młotek nie jest standardem Raspberry Pi.",
          link: "Zobacz zestaw dystansów M2.5 →",
          url: "https://amzn.eu/d/0ftWKzE5"
        },
        {
          kicker: "HEXAPOD FREENOVE — ZESTAW, KTÓRY NAJLEPIEJ SKŁADA SIĘ NA ZDJĘCIU PRODUKTOWYM.",
          title: "18 serw, kruche plastiki, niedopasowane elementy i zasilanie, przy którym zaczynasz liczyć minuty.",
          body: "Moje doświadczenie z hexapodem Freenove jest po prostu złe. Dystanse i mechanika nie zostały dopracowane tak, żeby robot dawał się bezproblemowo złożyć. Część plastikowych elementów jest na tyle krucha, że potrafi pęknąć już podczas normalnego montażu. Do tego dochodzą śruby i dystanse, których długości nie tworzą sensownego stacka. Efekt? Zamiast składać robota, zaczynasz projektować poprawki do zestawu, który miał być gotowy do złożenia.",
          images: [
            { src: "assets/gotchas/freenove-hexapod-gotcha-01.jpg", alt: "Freenove hexapod — warstwy elektroniki i okablowanie nad Raspberry Pi", caption: "Stack elektroniki: Raspberry Pi, płytki sterujące, przewody i coraz mniej miejsca." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-02.jpg", alt: "Freenove hexapod — Raspberry Pi z radiatorem i płytkami rozszerzeń", caption: "Raspberry Pi i kolejne moduły — kompatybilność na papierze nie rozwiązuje mechaniki." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-03.webp", alt: "Freenove hexapod — mały prześwit pomiędzy piętrami konstrukcji", caption: "Między płytami każdy milimetr zaczyna mieć znaczenie." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-04.webp", alt: "Freenove hexapod — całe podwozie z serwami i wielopoziomową elektroniką", caption: "18 serw i wielopoziomowa elektronika. Tu teoria spotyka stół montażowy." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-05.webp", alt: "Freenove hexapod — mocowanie serwa i śruba M2 x 16", caption: "Mocowanie serwa i M2×16 — kolejny detal, przy którym zaczyna się kombinowanie." }
          ],
          discovery: "Lista absurdów",
          discoveryText: "Hailo nie da się po prostu elegancko dołożyć do całego układu. Masz 18 serw, a fabryczna koncepcja zasilania opiera się na zaledwie 2× 18650 dla części napędowej oraz kolejnych ogniwach dla Raspberry Pi. Przy robocie tej wielkości brzmi to bardziej jak demonstracja niż zasilanie do poważnego chodzenia.",
          workaround: "Werdykt Marcinka",
          workaroundText: "W praktyce przebudowa mechaniki, własne dystanse i osobny, wydajny tor zasilania serw są rozsądniejsze niż ślepe trzymanie się zestawu. A „może dwie minuty chodzenia” zostawiam jako żart, nie pomiar — ale sam fakt, że taki żart przychodzi do głowy, mówi sporo o projekcie.",
          link: "",
          url: ""
        }
      ],
      note: "Ta sekcja będzie rosła. Każda rzecz, która wygląda dobrze w opisie produktu, a potem robi niespodziankę na stole, trafia właśnie tutaj. Bez pudrowania."
    },
    en: {
      tab: "🤬 WHY I GOT ANNOYED",
      index: "MARCIN FOUND IT / HARDWARE GOTCHAS",
      title: "What sellers and designers did not account for.",
      lead: "Not catalogue theory. These are the problems that only appear when you actually try to assemble, bolt together and run the hardware.",
      cards: [
        {
          kicker: "ON PAPER: COMPATIBLE. ON THE BENCH: YEAH, RIGHT.",
          title: "X1203 UPS + Raspberry Pi + HAT / Hailo — the mechanics say: not so fast.",
          body: "The UPS and Raspberry Pi can be assembled neatly on their own. The problem starts when a HAT is added: standard standoffs do not solve the whole stack as elegantly as the compatibility list suggests. M3 does not fit the UPS mounting points. M2 / M2.5 hardware needs to be selected and the thread confirmed before tightening — never force it.",
          discovery: "Marcin's finding",
          discoveryText: "Electrical compatibility ≠ mechanical compatibility. Before buying a HAT, UPS and enclosure, also check stack height, screw access and standoff thread size.",
          workaround: "Workaround",
          workaroundText: "Longer standoffs and a separate M2.5 assortment are now in the parts store. Test the thread first, tighten second. A hammer is not part of the Raspberry Pi standard.",
          link: "See the M2.5 standoff kit →",
          url: "https://amzn.eu/d/0ftWKzE5"
        },
        {
          kicker: "FREENOVE HEXAPOD — A KIT THAT ASSEMBLES BEST IN THE PRODUCT PHOTO.",
          title: "18 servos, brittle plastic, mismatched hardware and a power concept that makes you count minutes.",
          body: "My experience with the Freenove hexapod has been poor. The standoffs and mechanics are not refined enough for the robot to go together cleanly. Some plastic parts are brittle enough to crack during normal assembly. Screw and standoff lengths also fail to create a sensible stack. Instead of assembling the robot, you end up redesigning the kit that was supposed to be ready to build.",
          images: [
            { src: "assets/gotchas/freenove-hexapod-gotcha-01.jpg", alt: "Freenove hexapod — stacked electronics and wiring above the Raspberry Pi", caption: "The electronics stack: Raspberry Pi, controller boards, wiring and less room with every layer." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-02.jpg", alt: "Freenove hexapod — Raspberry Pi heatsink and expansion boards", caption: "Raspberry Pi plus more modules — compatibility on paper does not solve the mechanics." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-03.webp", alt: "Freenove hexapod — tight clearance between chassis layers", caption: "Between the plates, every millimetre starts to matter." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-04.webp", alt: "Freenove hexapod — complete chassis with servos and multi-level electronics", caption: "18 servos and multi-level electronics. This is where the brochure meets the workbench." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-05.webp", alt: "Freenove hexapod — servo mounting and M2 x 16 screw", caption: "Servo mounting and M2×16 — another small detail that turns into redesign work." }
          ],
          discovery: "The absurdity list",
          discoveryText: "Hailo cannot simply be added neatly to the complete stack. There are 18 servos, while the stock power concept relies on only 2× 18650 cells for the drive side plus additional cells for the Raspberry Pi. For a robot of this size, that feels more like a demonstration setup than serious walking power.",
          workaround: "Marcin's verdict",
          workaroundText: "In practice, rebuilding the mechanics, using your own standoffs and giving the servos a separate high-current power rail makes more sense than blindly following the kit. And the 'maybe two minutes of walking' line stays a joke, not a measured runtime — but the fact that the joke feels plausible says enough.",
          link: "",
          url: ""
        }
      ],
      note: "This section will grow. Anything that looks perfect in the product listing but produces a surprise on the workbench ends up here. No polishing the story."
    }
  };

  const hiddenWhileActive = [".filters", "#budgetSummary", ".results-line", "#partsGrid", "#emptyState"];

  function language() {
    return document.documentElement.lang === "en" ? "en" : "pl";
  }

  function ensurePanel() {
    let panel = document.querySelector("#gotchaPanel");
    if (panel) return panel;

    panel = document.createElement("section");
    panel.id = "gotchaPanel";
    panel.className = "gotcha-panel";
    panel.hidden = true;
    tabs.insertAdjacentElement("afterend", panel);
    return panel;
  }

  function cardHtml(card) {
    const gallery = Array.isArray(card.images) && card.images.length
      ? `<div class="gotcha-gallery">${card.images.map((image, index) => `
          <figure class="gotcha-photo${index === 0 ? " gotcha-photo--lead" : ""}">
            <a href="${image.src}" target="_blank" rel="noreferrer">
              <img src="${image.src}" alt="${image.alt}" loading="lazy" decoding="async">
            </a>
            <figcaption>${image.caption}</figcaption>
          </figure>`).join("")}</div>`
      : "";
    const link = card.url && card.link
      ? `<a class="gotcha-link" href="${card.url}" target="_blank" rel="noreferrer">${card.link}</a>`
      : "";
    return `
      <article class="gotcha-card">
        <div class="gotcha-stripe">⚠ ${card.kicker}</div>
        <div class="gotcha-card-body">
          <h4>${card.title}</h4>
          <p class="gotcha-main">${card.body}</p>
          ${gallery}
          <div class="gotcha-grid">
            <div><span>${card.discovery}</span><strong>${card.discoveryText}</strong></div>
            <div><span>${card.workaround}</span><strong>${card.workaroundText}</strong></div>
          </div>
          ${link}
        </div>
      </article>`;
  }

  function renderPanel() {
    const t = copy[language()];
    const panel = ensurePanel();
    panel.innerHTML = `
      <div class="gotcha-hero">
        <span class="gotcha-index">${t.index}</span>
        <h3>${t.title}</h3>
        <p>${t.lead}</p>
      </div>
      ${t.cards.map(cardHtml).join("")}
      <p class="gotcha-note">${t.note}</p>`;
  }

  function ensureTab() {
    let button = document.querySelector("#gotchaTab");
    if (!button) {
      button = document.createElement("button");
      button.type = "button";
      button.id = "gotchaTab";
      button.className = "gotcha-tab";
      tabs.prepend(button);
    }
    button.textContent = copy[language()].tab;
    button.classList.toggle("active", gotchasActive);
  }

  function setNormalContentVisible(visible) {
    hiddenWhileActive.forEach((selector) => {
      const element = document.querySelector(selector);
      if (element) element.style.display = visible ? "" : "none";
    });
  }

  function showGotchas() {
    gotchasActive = true;
    ensureTab();
    renderPanel();
    setNormalContentVisible(false);
    ensurePanel().hidden = false;
    document.querySelectorAll("#categoryTabs [data-category]").forEach((button) => button.classList.remove("active"));
  }

  function hideGotchas() {
    if (!gotchasActive) return;
    gotchasActive = false;
    ensurePanel().hidden = true;
    setNormalContentVisible(true);
    ensureTab();
  }

  tabs.addEventListener("click", (event) => {
    if (event.target.closest("#gotchaTab")) {
      showGotchas();
      return;
    }
    if (event.target.closest("[data-category]")) hideGotchas();
  }, true);

  const observer = new MutationObserver(() => {
    ensureTab();
    if (gotchasActive) {
      renderPanel();
      setNormalContentVisible(false);
    }
  });
  observer.observe(tabs, { childList: true });

  ensureTab();
  renderPanel();
  if (location.hash === "#gotchas" || new URLSearchParams(location.search).get("view") === "gotchas") {
    showGotchas();
  }
})();
