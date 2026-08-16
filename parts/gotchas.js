"use strict";

(() => {
  const tabs = document.querySelector("#categoryTabs");
  if (!tabs) return;

  let gotchasActive = false;

  const copy = {
    pl: {
      tab: "🤬 NA CO SIĘ WKURZYŁEM",
      index: "MARCINEK ODKRYŁ / #01",
      title: "Czego sprzedający i konstruktorzy nie przewidzieli.",
      lead: "Nie katalogowa teoria. Rzeczy, które wychodzą dopiero wtedy, kiedy naprawdę próbujesz to złożyć, przykręcić i uruchomić.",
      issueKicker: "NA PAPIERZE: PASUJE. NA STOLE: AHA, JASNE.",
      issueTitle: "UPS X1203 + Raspberry Pi + HAT / Hailo — mechanika mówi: nie tak szybko.",
      issueBody: "Sam UPS z Raspberry Pi da się złożyć sensownie. Problem zaczyna się przy dokładaniu HAT-a: standardowe dystanse nie rozwiązują całego stacka tak elegancko, jak sugerowałaby kompatybilność na papierze. M3 nie pasuje do mocowania UPS-a. M2 / M2.5 trzeba dobrać i potwierdzić gwint przed skręceniem — bez wciskania czegokolwiek na siłę.",
      discovery: "Odkrycie Marcinka",
      discoveryText: "Kompatybilność elektryczna ≠ kompatybilność mechaniczna. Zanim kupisz HAT, UPS i obudowę, sprawdź także wysokość stacka, dostęp do śrub i gwint dystansów.",
      workaround: "Obejście",
      workaroundText: "Dłuższe dystanse i osobny zestaw M2.5 są teraz w magazynie. Najpierw test gwintu, potem skręcanie. Młotek nie jest standardem Raspberry Pi.",
      link: "Zobacz zestaw dystansów M2.5 →",
      note: "Ta sekcja będzie rosła. Każda rzecz, która wygląda dobrze w opisie produktu, a potem robi niespodziankę na stole, trafia właśnie tutaj."
    },
    en: {
      tab: "🤬 WHAT ANNOYED ME",
      index: "MARCIN FOUND IT / #01",
      title: "What sellers and designers did not account for.",
      lead: "Not catalogue theory. These are the problems that only appear when you actually try to assemble, bolt together and run the hardware.",
      issueKicker: "ON PAPER: COMPATIBLE. ON THE BENCH: YEAH, RIGHT.",
      issueTitle: "X1203 UPS + Raspberry Pi + HAT / Hailo — the mechanics say: not so fast.",
      issueBody: "The UPS and Raspberry Pi can be assembled neatly on their own. The problem starts when a HAT is added: standard standoffs do not solve the whole stack as elegantly as the compatibility list suggests. M3 does not fit the UPS mounting points. M2 / M2.5 hardware needs to be selected and the thread confirmed before tightening — never force it.",
      discovery: "Marcin's finding",
      discoveryText: "Electrical compatibility ≠ mechanical compatibility. Before buying a HAT, UPS and enclosure, also check stack height, screw access and standoff thread size.",
      workaround: "Workaround",
      workaroundText: "Longer standoffs and a separate M2.5 assortment are now in the parts store. Test the thread first, tighten second. A hammer is not part of the Raspberry Pi standard.",
      link: "See the M2.5 standoff kit →",
      note: "This section will grow. Anything that looks perfect in the product listing but produces a surprise on the workbench ends up here."
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

  function renderPanel() {
    const t = copy[language()];
    const panel = ensurePanel();
    panel.innerHTML = `
      <div class="gotcha-hero">
        <span class="gotcha-index">${t.index}</span>
        <h3>${t.title}</h3>
        <p>${t.lead}</p>
      </div>
      <article class="gotcha-card">
        <div class="gotcha-stripe">⚠ ${t.issueKicker}</div>
        <div class="gotcha-card-body">
          <h4>${t.issueTitle}</h4>
          <p class="gotcha-main">${t.issueBody}</p>
          <div class="gotcha-grid">
            <div><span>${t.discovery}</span><strong>${t.discoveryText}</strong></div>
            <div><span>${t.workaround}</span><strong>${t.workaroundText}</strong></div>
          </div>
          <a class="gotcha-link" href="https://amzn.eu/d/0ftWKzE5" target="_blank" rel="noreferrer">${t.link}</a>
        </div>
      </article>
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
})();
