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
          kicker: "PYTAŁEM AI TRZY RAZY. POKAZAŁEM NAWET PŁYTKĘ. PEWNOŚĆ ROSŁA, POPRAWNOŚĆ NIE.",
          title: "AI kazało podłączyć krańcówkę do IN i 3.3 V. Miernik powiedział: IN i GND.",
          body: "Kupiłem zestaw sześciu mikrowyłączników ECSiNG do stóp Kory, bo AI uznało je za dobry wybór. Sam wybór części był sensowny. Problem pojawił się przy podłączeniu do Servo 2040: trzy razy pytałem o piny i pokazałem nawet zdjęcie płytki, a mimo to dostałem błędną instrukcję IN + 3.3 V. Zwykły mikrowyłącznik jest pasywnym stykiem — nie trzeba go zasilać.",
          discovery: "Co faktycznie działa",
          discoveryText: "Przy wejściu z podciąganiem styk łączy IN z GND. W spoczynku wejście jest wysokie, po naciśnięciu niskie. Kolor przewodu nie jest dokumentacją: parę COM–NO albo COM–NC najpierw sprawdzam miernikiem. Ten przełącznik potwierdza kontakt z ziemią; oddzielny czujnik nacisku będzie mierzył obciążenie nogi.",
          workaround: "Lekcja za cenę sześciu pstryczków",
          workaroundText: "AI świetnie skraca szukanie, ale nie zastępuje schematu, testu ciągłości ani zdrowej podejrzliwości wobec odpowiedzi wypowiedzianej z przesadną pewnością. Kora dostała czucie w stopach. AI dostało czujnik pokory.",
          link: "ECSiNG — 6 mikrowyłączników na Amazon UK →",
          url: "https://www.amazon.co.uk/dp/B0CXSLG6MY?tag=marcinkorapro-21"
        },
        {
          kicker: "NA PAPIERZE: PASUJE. NA STOLE: AHA, JASNE.",
          title: "UPS X1203 + Raspberry Pi + HAT / Hailo — mechanika mówi: nie tak szybko.",
          body: "Sam UPS z Raspberry Pi da się złożyć sensownie. Problem zaczyna się przy dokładaniu Hailo AI HAT+: pełny stack potrzebuje dodatkowych dystansów M2.5 oraz wysokiego złącza GPIO 2×20. Standardowe piny nie sięgają przez wszystkie warstwy, a bez dodatkowego prześwitu płytki nie układają się poprawnie.",
          discovery: "Odkrycie Marcinka",
          discoveryText: "Kompatybilność elektryczna ≠ kompatybilność mechaniczna. Zanim kupisz HAT, UPS i obudowę, sprawdź także wysokość stacka, dostęp do śrub i gwint dystansów.",
          workaround: "Obejście",
          workaroundText: "Zastosowałem dodatkowe dystanse M2.5 oraz 40-pinowy, wysoki header 2×20 z zestawu Pi Hut. Specjalne cztery piny PoE nie były potrzebne. Najpierw przymierz warstwy, potem dokręcaj — młotek nadal nie jest standardem Raspberry Pi.",
          products: [
            { name: "Dystanse M2.5 — zestaw 503 elementów", image: "https://m.media-amazon.com/images/I/51EsLOX+r1L._AC_SX679_.jpg", url: "https://amzn.to/4y1Rvbf", cta: "Sklep →" },
            { name: "Przedłużka GPIO 2×20 — extra-tall", image: "https://thepihut.com/cdn/shop/products/extra-tall-push-fit-stacking-gpio-header-for-raspberry-pi-double-shroud-the-pi-hut-104260-29471102468291_1000x.jpg?v=1646555760", url: "https://thepihut.com/products/stacking-header-for-pi-a-b-pi-2-pi-3-2x20-extra-tall-header", cta: "Sklep →" },
            { name: "SupTronics X1203 UPS", imageId: "x1203", url: "https://thepihut.com/products/suptronics-x1203-ups-add-on-for-raspberry-pi-5", cta: "Sklep →" },
            { name: "Raspberry Pi AI HAT+ 2 — Hailo", imageId: "ai-hat-2", url: "https://thepihut.com/products/raspberry-pi-ai-hat-2", cta: "Sklep →" },
            { name: "Raspberry Pi 5 — 16 GB", imageId: "rpi5-16", url: "https://amzn.to/4imKH38", cta: "Sklep →" }
          ],
          videoId: "2q_VNgvxkWU",
          videoTitle: "Kora komentuje problem z UPS X1203",
          videoLabel: "Obejrzyj Short: UPS, Hailo i brakujące dystanse →"
        },
        {
          kicker: "HEXAPOD FREENOVE — ZESTAW, KTÓRY NAJLEPIEJ SKŁADA SIĘ NA ZDJĘCIU PRODUKTOWYM.",
          title: "18 serw, kruche plastiki, niedopasowane elementy i zasilanie, przy którym zaczynasz liczyć minuty.",
          body: "Moje doświadczenie z hexapodem Freenove jest po prostu złe. Dystanse i mechanika nie zostały dopracowane tak, żeby robot dawał się bezproblemowo złożyć. Część plastikowych elementów jest na tyle krucha, że potrafi pęknąć już podczas normalnego montażu. Do tego dochodzą śruby i dystanse, których długości nie tworzą sensownego stacka. Efekt? Zamiast składać robota, zaczynasz projektować poprawki do zestawu, który miał być gotowy do złożenia.",
          images: [
            { src: "assets/gotchas/freenove-hexapod-gotcha-01-20260822.jpg", alt: "Freenove hexapod — warstwy elektroniki i okablowanie nad Raspberry Pi", caption: "Stack elektroniki: Raspberry Pi, płytki sterujące, przewody i coraz mniej miejsca." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-02-20260822.jpg", alt: "Freenove hexapod — Raspberry Pi z radiatorem i płytkami rozszerzeń", caption: "Raspberry Pi i kolejne moduły — kompatybilność na papierze nie rozwiązuje mechaniki." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-03.webp", alt: "Freenove hexapod — mały prześwit pomiędzy piętrami konstrukcji", caption: "Między płytami każdy milimetr zaczyna mieć znaczenie." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-04-20260822.jpg", alt: "Freenove hexapod — całe podwozie z serwami i wielopoziomową elektroniką", caption: "18 serw i wielopoziomowa elektronika. Tu teoria spotyka stół montażowy." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-05.webp", alt: "Freenove hexapod — mocowanie serwa i śruba M2 x 16", caption: "Mocowanie serwa i M2×16 — kolejny detal, przy którym zaczyna się kombinowanie." }
          ],
          discovery: "Lista absurdów",
          discoveryText: "Hailo nie da się po prostu elegancko dołożyć do całego układu. Masz 18 serw, a fabryczna koncepcja zasilania opiera się na zaledwie 2× 18650 dla części napędowej oraz kolejnych ogniwach dla Raspberry Pi. Przy robocie tej wielkości brzmi to bardziej jak demonstracja niż zasilanie do poważnego chodzenia.",
          workaround: "Werdykt Marcinka",
          workaroundText: "W praktyce przebudowa mechaniki, własne dystanse i osobny, wydajny tor zasilania serw są rozsądniejsze niż ślepe trzymanie się zestawu. A „może dwie minuty chodzenia” zostawiam jako żart, nie pomiar — ale sam fakt, że taki żart przychodzi do głowy, mówi sporo o projekcie.",
          link: "",
          url: ""
        },
        {
          kicker: "ALUMINIOWE NOGI DZIAŁAJĄ. STARY PRZEWÓD SERWA — JUŻ NIE.",
          title: "Odzyskane serwo zadymiło przez stary przewód zaraz po podłączeniu.",
          body: "W teście z 22.08.2026 aluminiowa mechanika nóg nie była problemem. Po podłączeniu odzyskanego serwa zadymił stary przewód. To jest potwierdzony objaw; bez pomiarów nie przypisuję jeszcze winy sterownikowi, serwu, polaryzacji, izolacji ani złączu.",
          discovery: "Wniosek z testu",
          discoveryText: "Wiek przewodu jest częścią ryzyka tak samo jak stan samego serwa. Wygląd zewnętrzny nie potwierdza poprawnych crimpów, izolacji ani układu pinów.",
          workaround: "Następny bezpieczny krok",
          workaroundText: "Podejrzany przewód zostaje wycofany. Przy odłączonym zasilaniu sprawdzam złącze, piny, ciągłość i zwarcia, a każde odzyskane serwo testuję osobno na torze z ograniczeniem prądu i bezpiecznikiem. Całe nogi dostaną zasilanie dopiero po tych testach.",
          link: "Pełny dziennik aluminiowej Kory →",
          url: "../updates/kora-aluminium-2026-08-11/#servo-lead-incident"
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
          kicker: "I ASKED AI THREE TIMES. I EVEN SHOWED IT THE BOARD. CONFIDENCE WENT UP. ACCURACY DID NOT.",
          title: "AI told me to wire the switch to IN and 3.3 V. The multimeter voted for IN and GND.",
          body: "I bought a six-pack of ECSiNG micro limit switches for Kora's feet because AI suggested they were a good fit. The component choice itself was sensible. The wiring advice was not: I asked about the pins three times and even supplied a photo of the Servo 2040 board, yet still received the wrong IN + 3.3 V instruction. A basic microswitch is a passive contact; it does not need a power supply.",
          discovery: "What actually works",
          discoveryText: "With an input pull-up, the switch connects IN to GND. The input reads high at rest and low when pressed. Wire colour is not documentation: I first identify COM–NO or COM–NC with a multimeter. This switch confirms ground contact; a separate pressure sensor will measure how much load the leg carries.",
          workaround: "A lesson priced at six tiny clicks",
          workaroundText: "AI is excellent for narrowing a search, but it is not a schematic, a continuity tester or a replacement for healthy suspicion when an answer sounds far too confident. Kora gained touch in her feet. AI gained a humility sensor.",
          link: "ECSiNG — 6 micro limit switches on Amazon UK →",
          url: "https://www.amazon.co.uk/dp/B0CXSLG6MY?tag=marcinkorapro-21"
        },
        {
          kicker: "ON PAPER: COMPATIBLE. ON THE BENCH: YEAH, RIGHT.",
          title: "X1203 UPS + Raspberry Pi + HAT / Hailo — the mechanics say: not so fast.",
          body: "The UPS and Raspberry Pi can be assembled neatly on their own. The problem starts when the Hailo AI HAT+ is added: the full stack needs extra M2.5 standoffs and a 2×20 extra-tall GPIO header. Standard pins do not reach through every layer, and without extra clearance the boards cannot sit correctly.",
          discovery: "Marcin's finding",
          discoveryText: "Electrical compatibility ≠ mechanical compatibility. Before buying a HAT, UPS and enclosure, also check stack height, screw access and standoff thread size.",
          workaround: "Workaround",
          workaroundText: "I used extra M2.5 standoffs and the 40-pin, 2×20 extra-tall header from the Pi Hut kit. The special four PoE pins were not required. Dry-fit every layer before tightening — a hammer is still not part of the Raspberry Pi standard.",
          products: [
            { name: "M2.5 standoffs — 503-piece kit", image: "https://m.media-amazon.com/images/I/51EsLOX+r1L._AC_SX679_.jpg", url: "https://amzn.to/4y1Rvbf", cta: "Shop →" },
            { name: "2×20 extra-tall GPIO header", image: "https://thepihut.com/cdn/shop/products/extra-tall-push-fit-stacking-gpio-header-for-raspberry-pi-double-shroud-the-pi-hut-104260-29471102468291_1000x.jpg?v=1646555760", url: "https://thepihut.com/products/stacking-header-for-pi-a-b-pi-2-pi-3-2x20-extra-tall-header", cta: "Shop →" },
            { name: "SupTronics X1203 UPS", imageId: "x1203", url: "https://thepihut.com/products/suptronics-x1203-ups-add-on-for-raspberry-pi-5", cta: "Shop →" },
            { name: "Raspberry Pi AI HAT+ 2 — Hailo", imageId: "ai-hat-2", url: "https://thepihut.com/products/raspberry-pi-ai-hat-2", cta: "Shop →" },
            { name: "Raspberry Pi 5 — 16 GB", imageId: "rpi5-16", url: "https://amzn.to/4imKH38", cta: "Shop →" }
          ],
          videoId: "2q_VNgvxkWU",
          videoTitle: "Kora comments on the X1203 UPS problem",
          videoLabel: "Watch the Short: UPS, Hailo and missing standoffs →"
        },
        {
          kicker: "FREENOVE HEXAPOD — A KIT THAT ASSEMBLES BEST IN THE PRODUCT PHOTO.",
          title: "18 servos, brittle plastic, mismatched hardware and a power concept that makes you count minutes.",
          body: "My experience with the Freenove hexapod has been poor. The standoffs and mechanics are not refined enough for the robot to go together cleanly. Some plastic parts are brittle enough to crack during normal assembly. Screw and standoff lengths also fail to create a sensible stack. Instead of assembling the robot, you end up redesigning the kit that was supposed to be ready to build.",
          images: [
            { src: "assets/gotchas/freenove-hexapod-gotcha-01-20260822.jpg", alt: "Freenove hexapod — stacked electronics and wiring above the Raspberry Pi", caption: "The electronics stack: Raspberry Pi, controller boards, wiring and less room with every layer." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-02-20260822.jpg", alt: "Freenove hexapod — Raspberry Pi heatsink and expansion boards", caption: "Raspberry Pi plus more modules — compatibility on paper does not solve the mechanics." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-03.webp", alt: "Freenove hexapod — tight clearance between chassis layers", caption: "Between the plates, every millimetre starts to matter." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-04-20260822.jpg", alt: "Freenove hexapod — complete chassis with servos and multi-level electronics", caption: "18 servos and multi-level electronics. This is where the brochure meets the workbench." },
            { src: "assets/gotchas/freenove-hexapod-gotcha-05.webp", alt: "Freenove hexapod — servo mounting and M2 x 16 screw", caption: "Servo mounting and M2×16 — another small detail that turns into redesign work." }
          ],
          discovery: "The absurdity list",
          discoveryText: "Hailo cannot simply be added neatly to the complete stack. There are 18 servos, while the stock power concept relies on only 2× 18650 cells for the drive side plus additional cells for the Raspberry Pi. For a robot of this size, that feels more like a demonstration setup than serious walking power.",
          workaround: "Marcin's verdict",
          workaroundText: "In practice, rebuilding the mechanics, using your own standoffs and giving the servos a separate high-current power rail makes more sense than blindly following the kit. And the 'maybe two minutes of walking' line stays a joke, not a measured runtime — but the fact that the joke feels plausible says enough.",
          link: "",
          url: ""
        },
        {
          kicker: "THE ALUMINIUM LEGS WORK. THE OLD SERVO LEAD DOES NOT.",
          title: "A salvaged servo produced smoke from its old lead as soon as it was connected.",
          body: "In the 22 Aug 2026 test, the aluminium leg mechanics were not the problem. An old lead on a salvaged servo emitted smoke after connection. That symptom is confirmed; without measurements I am not yet blaming the controller, servo, polarity, insulation or connector.",
          discovery: "What the test proved",
          discoveryText: "Cable age is part of the risk, just like the condition of the servo itself. A clean exterior does not confirm sound crimps, insulation or pin order.",
          workaround: "Next safe step",
          workaroundText: "The suspect lead is retired. With power disconnected, I will inspect the connector and pins, check continuity and shorts, then test each salvaged servo individually on a current-limited, fused rail. The complete leg system will only be powered after those checks.",
          link: "Full Aluminium Kora build log →",
          url: "../updates/kora-aluminium-2026-08-11/#servo-lead-incident"
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
    const products = Array.isArray(card.products) && card.products.length
      ? `<div class="gotcha-products">${card.products.map((product) => {
          const image = product.image || (window.PART_IMAGES && window.PART_IMAGES[product.imageId]) || "";
          return `<article class="gotcha-product">
            <a class="gotcha-product-image" href="${product.url}" target="_blank" rel="noreferrer">
              <img src="${image}" alt="${product.name}" loading="lazy" decoding="async">
            </a>
            <div class="gotcha-product-copy">
              <strong>${product.name}</strong>
              <a href="${product.url}" target="_blank" rel="noreferrer">${product.cta}</a>
            </div>
          </article>`;
        }).join("")}</div>`
      : "";
    const video = card.videoId
      ? `<div class="gotcha-video">
          <iframe src="https://www.youtube-nocookie.com/embed/${card.videoId}" title="${card.videoTitle}" loading="lazy" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
          <a class="gotcha-video-link" href="https://youtube.com/shorts/${card.videoId}" target="_blank" rel="noreferrer">${card.videoLabel}</a>
        </div>`
      : "";
    return `
      <article class="gotcha-card">
        <div class="gotcha-stripe">⚠ ${card.kicker}</div>
        <div class="gotcha-card-body">
          <h4>${card.title}</h4>
          <p class="gotcha-main">${card.body}</p>
          ${products}
          ${video}
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
