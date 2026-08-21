/* Portfolio bootstrap: preserve German translations and add current Kora series episode. */
window.PORTFOLIO_DE = {};
try {
  const xhr = new XMLHttpRequest();
  xhr.open('GET', 'portfolio-de-translations.js?v=20260821-1', false);
  xhr.send(null);
  if ((xhr.status >= 200 && xhr.status < 300) || xhr.status === 0) {
    (0, eval)(xhr.responseText);
  }
} catch (error) {
  console.warn('Portfolio translations fallback:', error);
}

(() => {
  const section = document.getElementById('kora-says');
  if (!section || section.dataset.episode02Injected === '1') return;
  const card = section.querySelector('.series-card');
  const copy = section.querySelector('.series-copy');
  const episode01 = section.querySelector('.series-dialogue');
  if (!card || !copy || !episode01) return;

  section.dataset.episode02Injected = '1';

  const kicker = copy.querySelector('.series-kicker');
  if (kicker) {
    kicker.dataset.en = 'SERIES · EPISODE 02 NOW LIVE';
    kicker.dataset.pl = 'SERIA · ODCINEK 02 JUŻ JEST';
    kicker.textContent = kicker.dataset.en;
  }

  const intro = copy.querySelector('p');
  if (intro) {
    intro.dataset.en = 'Kora answers first — usually dry, sarcastic and slightly insulting — then I show the real build, the parts, the problem and the result. Episode #2 is the first power test of her new metal legs.';
    intro.dataset.pl = 'Najpierw odpowiada Kora — zwykle sucho, sarkastycznie i trochę złośliwie — a potem pokazuję prawdziwy montaż, części, problem i wynik. Odcinek #2 to pierwszy test zasilania nowych metalowych nóg.';
    intro.textContent = intro.dataset.en;
  }

  const actions = copy.querySelector('.series-actions');
  if (actions) {
    const watch = document.createElement('a');
    watch.className = 'button button-primary';
    watch.href = 'https://youtube.com/shorts/lTnTV2sUjIg';
    watch.target = '_blank';
    watch.rel = 'noopener';
    watch.dataset.en = 'Watch episode #2 ↗';
    watch.dataset.pl = 'Obejrzyj odcinek #2 ↗';
    watch.textContent = watch.dataset.en;
    actions.prepend(watch);

    const build = document.createElement('a');
    build.className = 'button';
    build.href = 'updates/kora-aluminium-2026-08-11/';
    build.dataset.en = 'Battery, BMS & parts →';
    build.dataset.pl = 'Bateria, BMS i części →';
    build.textContent = build.dataset.en;
    actions.insertBefore(build, actions.children[1] || null);
  }

  const episodes = document.createElement('div');
  episodes.className = 'series-episodes';
  card.insertBefore(episodes, episode01);

  const episode02 = document.createElement('div');
  episode02.className = 'series-dialogue series-teaser series-dialogue--latest';
  episode02.setAttribute('aria-label', 'Episode two — published');
  episode02.innerHTML = `
    <span class="series-label" data-pl="ODCINEK 02 · OPUBLIKOWANY" data-en="EPISODE 02 · PUBLISHED">EPISODE 02 · PUBLISHED</span>
    <blockquote data-pl="Bateria → BMS → przetwornica → sterownik serw → pierwszy ruch metalowych nóg." data-en="Battery → BMS → step-down converter → servo controller → first metal-leg movement.">Battery → BMS → step-down converter → servo controller → first metal-leg movement.</blockquote>
    <p data-pl="Około 40 minut pracy skrócone do 1:16. Pakiet z ogniw 18650 z baterii rowerowej, BMS, przetwornica DC-DC step-down i 32-kanałowy sterownik serw zostały połączone w pierwszy prawdziwy test nowego układu nóg. Nogi drgnęły — to jeszcze nie chodzenie, ale cały tor zasilania i sterowania zadziałał fizycznie." data-en="Around 40 minutes of workshop work compressed into 1:16. A pack built from 18650 cells from an e-bike battery, a BMS, a DC-DC step-down converter and the 32-channel servo controller came together for the first real test of the new leg system. The legs twitched — not walking yet, but the complete power-and-control path produced real movement.">Around 40 minutes of workshop work compressed into 1:16. A pack built from 18650 cells from an e-bike battery, a BMS, a DC-DC step-down converter and the 32-channel servo controller came together for the first real test of the new leg system. The legs twitched — not walking yet, but the complete power-and-control path produced real movement.</p>
    <div class="series-parts" aria-label="Parts used in episode two">
      <span>18650</span><span>BMS</span><span>DC-DC STEP-DOWN</span><span>32-CH SERVO CTRL</span><span>METAL LEGS</span>
    </div>
    <div class="series-episode-actions">
      <a class="button button-youtube" href="https://youtube.com/shorts/lTnTV2sUjIg" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Obejrzyj Short #2 ↗" data-en="Watch Short #2 ↗">Watch Short #2 ↗</span></a>
      <a class="button" href="updates/kora-aluminium-2026-08-11/" data-pl="Pełny build log i części →" data-en="Full build log & parts →">Full build log & parts →</a>
    </div>`;

  episodes.appendChild(episode02);
  episodes.appendChild(episode01);

  const style = document.createElement('style');
  style.textContent = `
    #kora-says .series-episodes{display:grid;gap:14px;align-content:start;min-width:0}
    #kora-says .series-dialogue--latest{border-color:var(--acid);box-shadow:inset 0 0 0 1px var(--acid)}
    #kora-says .series-parts{display:flex;flex-wrap:wrap;gap:7px;margin:16px 0}
    #kora-says .series-parts span{display:inline-flex;padding:6px 8px;border:1px solid #55564f;color:#d9ff57;background:#111;font:800 .64rem/1.2 ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.05em}
    #kora-says .series-episode-actions{display:flex;flex-wrap:wrap;gap:8px;margin-top:16px}
    @media(max-width:900px){#kora-says .series-card{grid-template-columns:1fr}#kora-says .series-episodes{margin-top:8px}}
  `;
  document.head.appendChild(style);
})();
