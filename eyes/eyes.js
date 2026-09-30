(() => {
  'use strict';
  const params = new URLSearchParams(window.location.search);
  let language = params.get('lang') === 'pl' ? 'pl' : 'en';
  const direct = document.getElementById('directDemo');
  const buffered = document.getElementById('bufferedDemo');
  let step = 0;
  let generation = 0;
  const messages = {
    en: ['A complete eye is visible. Click to start a new redraw.',
      '1 / 4 · Clear. Direct drawing exposes black; the canvas screen keeps its previous eye.',
      '2 / 4 · Draw the glow and iris. Only the direct screen shows unfinished work.',
      '3 / 4 · Add the pupil and highlights. The canvas is still being built in RAM.',
      '4 / 4 · Add eyelids, then copy. Both screens now show the completed new eye.'],
    pl: ['Widać gotowe oko. Kliknij, aby rozpocząć rysowanie kolejnej klatki.',
      '1 / 4 · Kasowanie. Bezpośrednio widać czerń; ekran z canvasem zachowuje poprzednie oko.',
      '2 / 4 · Poświata i tęczówka. Tylko bezpośredni ekran pokazuje niedokończony obraz.',
      '3 / 4 · Źrenica i refleksy. Canvas nadal powstaje w RAM.',
      '4 / 4 · Powieki, potem kopiowanie. Oba ekrany pokazują gotowe nowe oko.']
  };

  function installPiDirectorSection() {
    const nav = document.querySelector('.top nav');
    const languages = nav && nav.querySelector('.languages');
    if (nav && languages && !nav.querySelector('a[href="#pi-control"]')) {
      const link = document.createElement('a');
      link.href = '#pi-control';
      link.dataset.en = 'Pi director';
      link.dataset.pl = 'Panel na Pi';
      link.textContent = 'Pi director';
      nav.insertBefore(link, languages);
    }

    const canvas = document.getElementById('canvas');
    if (canvas && !document.getElementById('pi-control')) {
      const section = document.createElement('section');
      section.className = 'section';
      section.id = 'pi-control';
      section.innerHTML = `
        <div class="wrap">
          <div class="kicker" data-en="RASPBERRY PI / USB EYE DIRECTOR / 29 SEP 2026" data-pl="RASPBERRY PI / PANEL USB DO OCZU / 29 WRZ 2026">RASPBERRY PI / USB EYE DIRECTOR / 29 SEP 2026</div>
          <h2 data-en="One panel. Four eye personalities. One USB cable." data-pl="Jeden panel. Cztery osobowości oczu. Jeden kabel USB.">One panel. Four eye personalities. One USB cable.</h2>
          <p class="section-intro" data-en="Kora Eye Film Show is the Raspberry Pi panel used to demonstrate and film the Human, Metal, Monster and Animal eye builds. It sends simple serial commands to the ESP32-S3 and gives each eye set a repeatable SHORT or LONG performance." data-pl="Kora Eye Film Show to panel na Raspberry Pi używany do prezentowania i nagrywania oczu Human, Metal, Monster i Animal. Wysyła proste komendy szeregowe do ESP32-S3 i daje każdej wersji powtarzalny pokaz SHORT albo LONG.">Kora Eye Film Show is the Raspberry Pi panel used to demonstrate and film the Human, Metal, Monster and Animal eye builds. It sends simple serial commands to the ESP32-S3 and gives each eye set a repeatable SHORT or LONG performance.</p>

          <div class="facts">
            <div class="fact"><b>115200 USB</b><span data-en="Pi ↔ ESP32-S3 serial" data-pl="Serial Pi ↔ ESP32-S3">Pi ↔ ESP32-S3 serial</span></div>
            <div class="fact"><b>SHORT / LONG</b><span data-en="Four ready-made filming routines" data-pl="Cztery gotowe sekwencje nagrań">Four ready-made filming routines</span></div>
            <div class="fact"><b>GPIO3 / GPIO4</b><span data-en="Real light sensors — mapping follows each sketch" data-pl="Prawdziwe czujniki światła — mapowanie zgodne z danym szkicem">Real light sensors — mapping follows each sketch</span></div>
          </div>

          <div class="split">
            <div>
              <h3 data-en="What the panel controls" data-pl="Co steruje panel">What the panel controls</h3>
              <p data-en="WAKE and SLEEP, manual blink, nine gaze directions, precise LOOK X Y control and a live serial log. Human, Metal and Monster V18 also get iris controls; Animal keeps its own amber animal-eye character." data-pl="WAKE i SLEEP, ręczne mrugnięcie, dziewięć kierunków patrzenia, precyzyjne LOOK X Y i podgląd logu serial. Human, Metal i Monster V18 mają także sterowanie tęczówką; Animal zachowuje własny bursztynowy, zwierzęcy charakter.">WAKE and SLEEP, manual blink, nine gaze directions, precise LOOK X Y control and a live serial log. Human, Metal and Monster V18 also get iris controls; Animal keeps its own amber animal-eye character.</p>
              <p data-en="SHORT moves quickly through the strongest expressions for vertical video. LONG slows the same ideas down, adds more gaze and iris shots and finishes with a real light-sensor demonstration." data-pl="SHORT szybko pokazuje najmocniejsze reakcje do pionowego filmu. LONG zwalnia te same ruchy, dodaje więcej ujęć spojrzenia i tęczówek, a na końcu wykonuje prawdziwy pokaz czujników światła.">SHORT moves quickly through the strongest expressions for vertical video. LONG slows the same ideas down, adds more gaze and iris shots and finishes with a real light-sensor demonstration.</p>
            </div>
            <div>
              <h3 data-en="Safe by design" data-pl="Bezpieczny z założenia">Safe by design</h3>
              <p data-en="The panel does not open a serial port or send an eye command at startup. You choose the device and press CONNECT yourself. Names that look like Servo2040 / MicroPython / Pimoroni are filtered from the eye candidates." data-pl="Po uruchomieniu panel nie otwiera portu i nie wysyła żadnej komendy do oczu. Sam wybierasz urządzenie i naciskasz CONNECT. Nazwy wyglądające na Servo2040 / MicroPython / Pimoroni są filtrowane z kandydatów na port oczu.">The panel does not open a serial port or send an eye command at startup. You choose the device and press CONNECT yourself. Names that look like Servo2040 / MicroPython / Pimoroni are filtered from the eye candidates.</p>
              <p class="notice" data-en="This is an eye-display tool only. It does not control Kora’s mechanical head, legs or Servo2040. USB tty numbers can change after reconnecting hardware, so always verify the selected port." data-pl="To narzędzie wyłącznie do wyświetlanych oczu. Nie steruje mechaniczną głową Kory, nogami ani Servo2040. Numery tty mogą zmienić się po przepięciu USB, dlatego zawsze sprawdź wybrany port.">This is an eye-display tool only. It does not control Kora’s mechanical head, legs or Servo2040. USB tty numbers can change after reconnecting hardware, so always verify the selected port.</p>
            </div>
          </div>

          <div class="code-box">
            <div class="code-label">RASPBERRY PI · RUN</div>
            <pre><code>sudo apt install -y python3-serial
cd /home/marcin/vega_robot
python3 kora_eye_film_show.py</code></pre>
          </div>

          <p data-en="LIGHT DEMO deliberately does not send a synthetic LIGHT value. Shine a real flashlight on the GPIO3/GPIO4 sensors or cover them and let the firmware react. The automatic pupils change size from the real sensor readings, while the eyelids progressively squint in strong light. GPIO3/GPIO4 left-right assignment follows the specific sketch." data-pl="LIGHT DEMO celowo nie wysyła sztucznej wartości LIGHT. Poświeć prawdziwą latarką na czujniki GPIO3/GPIO4 albo je zasłoń i pozwól firmware’owi zareagować. Automatyczne źrenice zmieniają rozmiar na podstawie prawdziwych odczytów, a przy mocnym świetle powieki stopniowo się przymykają. Przypisanie lewy/prawy dla GPIO3/GPIO4 jest zgodne z konkretnym szkicem.">LIGHT DEMO deliberately does not send a synthetic LIGHT value. Shine a real flashlight on the GPIO3/GPIO4 sensors or cover them and let the firmware react. The automatic pupils change size from the real sensor readings, while the eyelids progressively squint in strong light. GPIO3/GPIO4 left-right assignment follows the specific sketch.</p>

          <div class="buttons">
            <a class="btn primary" href="kora_eye_film_show.py" download data-en="Download Pi panel ↓" data-pl="Pobierz panel na Pi ↓">Download Pi panel ↓</a>
            <a class="btn" href="KORA_EYE_FILM_SHOW_EN_PL.md" data-en="EN / PL setup guide →" data-pl="Instrukcja EN / PL →">EN / PL setup guide →</a>
          </div>
        </div>`;
      canvas.parentNode.insertBefore(section, canvas);
    }

    const testedEn = '✓ HARDWARE TESTED ON KORA — 30 SEP 2026';
    const testedPl = '✓ SPRAWDZONE NA KORZE — 30 WRZ 2026';
    ['realistic', 'metal', 'monster', 'animal-v14'].forEach(id => {
      const card = document.getElementById(id);
      if (!card || card.querySelector('.kora-tested-badge')) return;
      const title = card.querySelector('h3');
      if (!title) return;
      const badge = document.createElement('p');
      badge.className = 'small kora-tested-badge';
      badge.dataset.en = testedEn;
      badge.dataset.pl = testedPl;
      badge.textContent = testedEn;
      badge.style.fontWeight = '800';
      badge.style.letterSpacing = '.04em';
      badge.style.color = '#79e7e4';
      title.insertAdjacentElement('afterend', badge);
    });

    const v18NoteEn = 'Hardware-tested on Kora: automatic sleep after 5 seconds of inactivity, real GPIO3/GPIO4 light sensors, automatic pupil response, strong-light eyelid squint and LOOK X Y control. Left/right GPIO3/GPIO4 assignment follows the specific sketch. Set your own AP/OTA passwords before upload.';
    const v18NotePl = 'Sprawdzone na Korie: automatyczne uśpienie po 5 sekundach bezczynności, prawdziwe czujniki światła GPIO3/GPIO4, automatyczna reakcja źrenic, przymykanie powiek przy mocnym świetle i sterowanie LOOK X Y. Przypisanie lewy/prawy GPIO3/GPIO4 jest zgodne z konkretnym szkicem. Przed wgraniem ustaw własne hasła AP/OTA.';
    ['realistic', 'metal', 'monster'].forEach(id => {
      const note = document.querySelector(`#${id} .small:not(.kora-tested-badge)`);
      if (note) {
        note.dataset.en = v18NoteEn;
        note.dataset.pl = v18NotePl;
      }
    });

    const animalNote = document.querySelector('#animal-v14 .small:not(.kora-tested-badge)');
    if (animalNote) {
      animalNote.dataset.en = 'Hardware-tested Animal V14: automatic sleep after 5 seconds of inactivity, GPIO3/GPIO4 light sensors, automatic pupil response, strong-light eyelid squint and LOOK X Y control. Left/right GPIO assignment follows this sketch. USB control is ready; set your own AP/OTA passwords before enabling Wi-Fi.';
      animalNote.dataset.pl = 'Sprawdzone Animal V14: automatyczne uśpienie po 5 sekundach bezczynności, czujniki światła GPIO3/GPIO4, automatyczna reakcja źrenic, przymykanie powiek przy mocnym świetle i sterowanie LOOK X Y. Przypisanie lewy/prawy GPIO jest zgodne z tym szkicem. USB działa od razu; przed włączeniem Wi-Fi ustaw własne hasła AP/OTA.';
    }
  }

  function updateStatus() {
    document.getElementById('demoStatus').textContent = messages[language][step];
  }

  function setLanguage(next) {
    language = next;
    document.documentElement.lang = next;
    document.querySelectorAll('[data-en][data-pl]').forEach(el => {
      el.textContent = el.dataset[next];
    });
    document.querySelectorAll('[data-language]').forEach(el => {
      el.setAttribute('aria-current', String(el.dataset.language === next));
    });
    document.title = next === 'pl'
      ? 'Oczy Kory — piękne oczy bez makeupu | Darmowy kod ESP32'
      : 'Kora Eyes — Beautiful eyes, no makeup required | Free ESP32 code';
    document.querySelector('.hero-visual img').alt = next === 'pl'
      ? 'Kora z dwoma okrągłymi wyświetlaczami i niebieskimi oczami'
      : 'Kora with two round displays showing blue eyes';
    updateStatus();
  }

  installPiDirectorSection();

  document.querySelectorAll('[data-language]').forEach(link => {
    link.addEventListener('click', event => {
      event.preventDefault();
      const next = link.dataset.language;
      const url = new URL(window.location.href);
      url.searchParams.set('lang', next);
      history.replaceState(null, '', url);
      setLanguage(next);
    });
  });

  function drawEye(canvas, phase, eyeGeneration) {
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#020609';
    ctx.fillRect(0, 0, 320, 160);
    if (phase === 1) return;
    const x = eyeGeneration % 2 ? 180 : 146;
    const glow = ctx.createRadialGradient(160, 80, 10, 160, 80, 120);
    glow.addColorStop(0, '#124650'); glow.addColorStop(1, '#020609');
    ctx.fillStyle = glow; ctx.fillRect(0, 0, 320, 160);
    ctx.fillStyle = '#aeedf0';
    ctx.beginPath(); ctx.ellipse(160, 80, 100, 52, 0, 0, Math.PI * 2); ctx.fill();
    const iris = ctx.createRadialGradient(x, 78, 7, x, 78, 38);
    iris.addColorStop(0, '#b1fdff'); iris.addColorStop(.6, '#21cede'); iris.addColorStop(1, '#133541');
    ctx.fillStyle = iris; ctx.beginPath(); ctx.arc(x, 78, 37, 0, Math.PI * 2); ctx.fill();
    if (phase === 2) return;
    ctx.fillStyle = '#030e13';ctx.beginPath();ctx.arc(x,78,18,0,Math.PI*2);ctx.fill();
    ctx.fillStyle = '#fff';ctx.beginPath();ctx.arc(x-11,65,6,0,Math.PI*2);ctx.fill();
    ctx.beginPath();ctx.arc(x+11,88,3,0,Math.PI*2);ctx.fill();
    if (phase === 3) return;
    ctx.fillStyle = '#020609';
    ctx.beginPath();ctx.moveTo(0,0);ctx.lineTo(320,0);ctx.lineTo(320,65);
    ctx.quadraticCurveTo(160,18,0,72);ctx.closePath();ctx.fill();
    ctx.beginPath();ctx.moveTo(0,160);ctx.lineTo(320,160);ctx.lineTo(320,100);
    ctx.quadraticCurveTo(160,141,0,102);ctx.closePath();ctx.fill();
    ctx.strokeStyle = '#6debf0';ctx.lineWidth = 2;
    ctx.beginPath();ctx.moveTo(61,55);ctx.quadraticCurveTo(160,28,259,52);ctx.stroke();
  }

  document.getElementById('demoStep').addEventListener('click', () => {
    if (step === 0 || step === 4) { step = 1; generation += 1; }
    else step += 1;
    drawEye(direct, step, generation);
    if (step === 4) drawEye(buffered, 4, generation);
    updateStatus();
  });

  drawEye(direct, 4, 0);
  drawEye(buffered, 4, 0);
  setLanguage(language);
})();
