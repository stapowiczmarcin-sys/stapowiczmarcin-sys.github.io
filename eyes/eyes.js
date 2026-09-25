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
      ? 'Dwoje turkusowych oczu na jednym prostokątnym ekranie — ilustracja'
      : 'Two cyan eyes on one rectangular display — concept illustration';
    updateStatus();
  }

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
