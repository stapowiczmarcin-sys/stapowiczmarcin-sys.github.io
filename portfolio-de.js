/* Portfolio bootstrap: preserve German translations and add current Kora series episode. */
window.PORTFOLIO_DE = {};
try {
  const xhr = new XMLHttpRequest();
  xhr.open('GET', 'portfolio-de-translations.js?v=20260822-2', false);
  xhr.send(null);
  if ((xhr.status >= 200 && xhr.status < 300) || xhr.status === 0) {
    (0, eval)(xhr.responseText);
  }
} catch (error) {
  console.warn('Portfolio translations fallback:', error);
}

(() => {
  const section = document.getElementById('kora-says');
  if (!section || section.dataset.episode03Injected === '1') return;
  const card = section.querySelector('.series-card');
  const copy = section.querySelector('.series-copy');
  const episode01 = section.querySelector('.series-dialogue');
  if (!card || !copy || !episode01) return;

  section.dataset.episode03Injected = '1';

  const kicker = copy.querySelector('.series-kicker');
  if (kicker) {
    kicker.dataset.en = 'SERIES · EPISODE 03 NOW LIVE';
    kicker.dataset.pl = 'SERIA · ODCINEK 03 JUŻ JEST';
    kicker.textContent = kicker.dataset.en;
  }

  const intro = copy.querySelector('p');
  if (intro) {
    intro.dataset.en = 'Kora answers first — usually dry, sarcastic and slightly insulting — then I show the real build, the parts, the problem and the result. Episode #3 is the first power test of her new metal legs.';
    intro.dataset.pl = 'Najpierw odpowiada Kora — zwykle sucho, sarkastycznie i trochę złośliwie — a potem pokazuję prawdziwy montaż, części, problem i wynik. Odcinek #3 to pierwszy test zasilania nowych metalowych nóg.';
    intro.textContent = intro.dataset.en;
  }

  const actions = copy.querySelector('.series-actions');
  if (actions) {
    const watch = document.createElement('a');
    watch.className = 'button button-primary';
    watch.href = 'https://youtube.com/shorts/lTnTV2sUjIg';
    watch.target = '_blank';
    watch.rel = 'noopener';
    watch.dataset.en = 'Watch episode #3 ↗';
    watch.dataset.pl = 'Obejrzyj odcinek #3 ↗';
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

  const episode03 = document.createElement('div');
  episode03.className = 'series-dialogue series-teaser series-dialogue--latest';
  episode03.setAttribute('aria-label', 'Episode three — published');
  episode03.innerHTML = `
    <span class="series-label" data-pl="ODCINEK 03 · OPUBLIKOWANY" data-en="EPISODE 03 · PUBLISHED">EPISODE 03 · PUBLISHED</span>
    <blockquote data-pl="Bateria → BMS → przetwornica → sterownik serw → pierwszy ruch metalowych nóg." data-en="Battery → BMS → step-down converter → servo controller → first metal-leg movement.">Battery → BMS → step-down converter → servo controller → first metal-leg movement.</blockquote>
    <p data-pl="Około 40 minut pracy skrócone do 1:16. Pakiet z ogniw 18650 z baterii rowerowej, BMS, przetwornica DC-DC step-down i 32-kanałowy sterownik serw zostały połączone w pierwszy prawdziwy test nowego układu nóg. Nogi drgnęły — to jeszcze nie chodzenie, ale cały tor zasilania i sterowania zadziałał fizycznie." data-en="Around 40 minutes of workshop work compressed into 1:16. A pack built from 18650 cells from an e-bike battery, a BMS, a DC-DC step-down converter and the 32-channel servo controller came together for the first real test of the new leg system. The legs twitched — not walking yet, but the complete power-and-control path produced real movement.">Around 40 minutes of workshop work compressed into 1:16. A pack built from 18650 cells from an e-bike battery, a BMS, a DC-DC step-down converter and the 32-channel servo controller came together for the first real test of the new leg system. The legs twitched — not walking yet, but the complete power-and-control path produced real movement.</p>
    <div class="series-parts" aria-label="Parts used in episode three">
      <span>18650</span><span>BMS</span><span>DC-DC STEP-DOWN</span><span>32-CH SERVO CTRL</span><span>METAL LEGS</span>
    </div>
    <div class="series-episode-actions">
      <a class="button button-youtube" href="https://youtube.com/shorts/lTnTV2sUjIg" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Obejrzyj Short #3 ↗" data-en="Watch Short #3 ↗">Watch Short #3 ↗</span></a>
      <a class="button" href="updates/kora-aluminium-2026-08-11/" data-pl="Pełny build log i części →" data-en="Full build log & parts →">Full build log & parts →</a>
    </div>`;

  const episode02 = document.createElement('div');
  episode02.className = 'series-dialogue series-teaser';
  episode02.setAttribute('aria-label', 'Episode two — published');
  episode02.innerHTML = `
    <span class="series-label" data-pl="ODCINEK 02 · OPUBLIKOWANY" data-en="EPISODE 02 · PUBLISHED">EPISODE 02 · PUBLISHED</span>
    <blockquote data-pl="Klamka → za wysoko → plan ucieczki Kory trafia na mechaniczne ograniczenie." data-en="Door handle → too high → Kora's escape plan hits a mechanical limit.">Door handle → too high → Kora's escape plan hits a mechanical limit.</blockquote>
    <p data-pl="Kora odpowiada na pytanie, którego rezultat miał być prosty. Nie był. Jej właściwa odpowiedź zostaje w filmie — bez psucia puenty na stronie." data-en="Kora answers a question that was supposed to have a simple outcome. It did not. Her actual reply stays in the video — no punchline spoiled on the page.">Kora answers a question that was supposed to have a simple outcome. It did not. Her actual reply stays in the video — no punchline spoiled on the page.</p>
    <div class="series-episode-actions">
      <a class="button button-youtube" href="https://youtube.com/shorts/_DWuBdvMbng" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Obejrzyj Short #2 ↗" data-en="Watch Short #2 ↗">Watch Short #2 ↗</span></a>
    </div>`;

  episodes.appendChild(episode03);
  episodes.appendChild(episode02);
  episodes.appendChild(episode01);

  const style = document.createElement('style');
  style.textContent = `
    #kora-says .series-episodes{display:grid;gap:14px;align-content:start;min-width:0}
    #kora-says .series-card{color:#fff!important;background:#101112!important}
    #kora-says .series-copy h2{color:#fff!important}
    #kora-says .series-copy p{color:#c8c9c4!important}
    #kora-says .series-kicker,#kora-says .series-label{color:#d9ff57!important}
    #kora-says .series-dialogue{color:#fff!important;background:#191a1b!important;border-color:#4a4b47!important}
    #kora-says .series-dialogue p{color:#c8c9c4!important}
    #kora-says .series-dialogue blockquote{color:#fff!important;border-left-color:#d9ff57!important}
    #kora-says .series-dialogue--latest{border-color:#d9ff57!important;box-shadow:inset 0 0 0 1px #d9ff57}
    #kora-says .series-parts{display:flex;flex-wrap:wrap;gap:7px;margin:16px 0}
    #kora-says .series-parts span{display:inline-flex;padding:6px 8px;border:1px solid #66675f;color:#d9ff57!important;background:#111!important;font:800 .64rem/1.2 ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.05em}
    #kora-says .series-actions,#kora-says .series-episode-actions{display:flex;flex-wrap:wrap;gap:8px;margin-top:16px}
    #kora-says .button{color:#fff!important;border-color:#76776f!important;background:transparent!important}
    #kora-says .button:hover,#kora-says .button:focus-visible{color:#101112!important;border-color:#d9ff57!important;background:#d9ff57!important}
    #kora-says .button-primary{color:#101112!important;border-color:#d9ff57!important;background:#d9ff57!important}
    #kora-says .button-primary:hover,#kora-says .button-primary:focus-visible{color:#101112!important;background:#fff!important;border-color:#fff!important}
    #kora-says .button-youtube{color:#fff!important;border-color:#d00028!important;background:#d00028!important}
    #kora-says .button-youtube:hover,#kora-says .button-youtube:focus-visible{color:#101112!important;border-color:#d9ff57!important;background:#d9ff57!important}
    @media(max-width:900px){#kora-says .series-card{grid-template-columns:1fr}#kora-says .series-episodes{margin-top:8px}}
  `;
  document.head.appendChild(style);
})();

(() => {
  const section = document.getElementById('kora-says');
  if (!section || section.dataset.episode05StoryInjected === '1') return;

  const card = section.querySelector('.series-card');
  const copy = section.querySelector('.series-copy');
  if (!card || !copy) return;

  section.dataset.episode05StoryInjected = '1';

  const kicker = copy.querySelector('.series-kicker');
  if (kicker) {
    kicker.dataset.en = 'SERIES · EPISODES 01–06 PUBLISHED';
    kicker.dataset.pl = 'SERIA · ODCINKI 01–06 OPUBLIKOWANE';
    kicker.textContent = kicker.dataset.en;
  }

  const intro = copy.querySelector('p');
  if (intro) {
    intro.dataset.en = 'Episode #6 records a new hardware milestone: all 18 MG996R servos worked from one 32-channel controller, and Kora lifted and lowered her entire body on six plastic legs. It is a stand-and-sit test — not walking yet.';
    intro.dataset.pl = 'Odcinek #6 zapisuje nowy etap sprzętowy: wszystkie 18 serw MG996R pracowało z jednym 32-kanałowym sterownikiem, a Kora podniosła i opuściła cały korpus na sześciu plastikowych nogach. To test wstawania i siadania — jeszcze nie chodzenie.';
    intro.textContent = intro.dataset.en;
  }

  const actions = copy.querySelector('.series-actions');
  if (actions && !actions.querySelector('a[href*="IQUURKHAdes"]')) {
    const watch = document.createElement('a');
    watch.className = 'button button-youtube';
    watch.href = 'https://youtube.com/shorts/IQUURKHAdes?feature=share';
    watch.target = '_blank';
    watch.rel = 'noopener';
    watch.innerHTML = '<span class="youtube-mark" aria-hidden="true"></span><span data-pl="Odcinek #5 ↗" data-en="Episode #5 ↗">Episode #5 ↗</span>';
    actions.prepend(watch);
  }

  if (actions && !actions.querySelector('a[href="kora-brain/story-mode.html"]')) {
    const explain = document.createElement('a');
    explain.className = 'button';
    explain.href = 'kora-brain/story-mode.html';
    explain.dataset.en = 'How Kora stories work →';
    explain.dataset.pl = 'Jak Kora tworzy historie →';
    explain.textContent = explain.dataset.en;
    actions.insertBefore(explain, actions.children[1] || null);
  }

  let episodes = section.querySelector('.series-episodes');
  if (!episodes) {
    episodes = document.createElement('div');
    episodes.className = 'series-episodes';
    const firstDialogue = section.querySelector('.series-dialogue');
    if (firstDialogue) card.insertBefore(episodes, firstDialogue);
    else card.appendChild(episodes);
  }

  if (!episodes.querySelector('[data-episode="05"]')) {
    const episode05 = document.createElement('div');
    episode05.className = 'series-dialogue series-teaser series-dialogue--latest series-story-test';
    episode05.dataset.episode = '05';
    episode05.setAttribute('aria-label', 'Episode five — servo swap and Kora story test');
    episode05.innerHTML = `
      <span class="series-label" data-pl="ODCINEK 05 · TEST + ZABAWA" data-en="EPISODE 05 · TEST + PLAY">EPISODE 05 · TEST + PLAY</span>
      <blockquote data-pl="Przekładam serwa. Kora dostaje urwane „banan i ba…” i zamiast się zatrzymać — zaczyna własną dziwną historię." data-en="I am swapping servos. Kora gets a cut-off “banana and ba…” prompt and, instead of stopping, starts her own strange story.">I am swapping servos. Kora gets a cut-off “banana and ba…” prompt and, instead of stopping, starts her own strange story.</blockquote>
      <p data-pl="Po dymie ze starego przewodu aluminiowego zestawu wymieniam serwa i ruch najpierw sprawdzam na plastikowych nogach. W tym samym czasie wykorzystuję każdą chwilę do testowania rozmowy Kory: czy trzyma kontekst, czy improwizuje i czy nie wpada w tryb grzecznej sekretarki. Ten film pokazuje oba testy naraz — sprzętowy i językowo-charakterologiczny." data-en="After smoke from an old cable in the aluminium setup, I am replacing the servos and checking movement on the plastic legs first. At the same time I use every spare moment to test Kora's conversation: whether she keeps context, improvises and avoids slipping into polite-secretary mode. This Short shows both tests at once — hardware and language/personality.">After smoke from an old cable in the aluminium setup, I am replacing the servos and checking movement on the plastic legs first. At the same time I use every spare moment to test Kora's conversation: whether she keeps context, improvises and avoids slipping into polite-secretary mode. This Short shows both tests at once — hardware and language/personality.</p>
      <div class="series-parts" aria-label="Episode five test areas"><span>SERVO SWAP</span><span>PLASTIC LEGS</span><span>STORY MODE</span><span>MOOD PANEL</span><span>CONTEXT</span></div>
      <div class="series-episode-actions"><a class="button button-youtube" href="https://youtube.com/shorts/IQUURKHAdes?feature=share" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Obejrzyj Short #5 ↗" data-en="Watch Short #5 ↗">Watch Short #5 ↗</span></a><a class="button" href="kora-brain/story-mode.html" data-pl="Dlaczego historie wychodzą inaczej →" data-en="Why the stories come out differently →">Why the stories come out differently →</a></div>`;
    episodes.prepend(episode05);
  }

  if (!episodes.querySelector('[data-episode="06"]')) {
    const episode06 = document.createElement('div');
    episode06.className = 'series-dialogue series-teaser series-dialogue--latest series-stand-test';
    episode06.dataset.episode = '06';
    episode06.setAttribute('aria-label', 'Episode six — six plastic legs stand and sit test');
    episode06.innerHTML = `
      <span class="series-label" data-pl="ODCINEK 06 · PEŁNE WSTANIE I SIADANIE" data-en="EPISODE 06 · FULL STAND AND SIT">EPISODE 06 · FULL STAND AND SIT</span>
      <blockquote data-pl="18 serw MG996R. Jeden 32-kanałowy sterownik. Sześć plastikowych nóg podniosło i ponownie opuściło cały korpus Kory." data-en="18 MG996R servos. One 32-channel controller. Six plastic legs lifted and lowered Kora's entire body.">18 MG996R servos. One 32-channel controller. Six plastic legs lifted and lowered Kora's entire body.</blockquote>
      <p data-pl="To pierwszy potwierdzony pełny test wstawania i siadania tej konfiguracji. Film pokazuje rzeczywisty rezultat, ale nie nazywam go jeszcze chodzeniem." data-en="This is the first confirmed full stand-and-sit test of this configuration. The video shows the real result, but I am not calling it walking yet.">This is the first confirmed full stand-and-sit test of this configuration. The video shows the real result, but I am not calling it walking yet.</p>
      <div class="series-parts" aria-label="Episode six hardware"><span>18 × MG996R</span><span>32-CH CONTROLLER</span><span>6 PLASTIC LEGS</span><span>STAND + SIT</span></div>
      <div class="series-episode-actions"><a class="button button-youtube" href="https://youtube.com/shorts/P-dCpj5-U8c" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Obejrzyj Short #6 ↗" data-en="Watch Short #6 ↗">Watch Short #6 ↗</span></a></div>`;
    episodes.prepend(episode06);
  }

  if (!episodes.querySelector('.story-engine-note')) {
    const note = document.createElement('div');
    note.className = 'series-dialogue story-engine-note';
    note.innerHTML = `
      <span class="series-label" data-pl="STORY MODE · CO SIĘ ZMIENIA" data-en="STORY MODE · WHAT CHANGES">STORY MODE · WHAT CHANGES</span>
      <blockquote data-pl="Kora nie odtwarza gotowej historyjki z listy. Punkt startowy to pytanie, bieżący kontekst, stan Panelu Nastroju i generowana odpowiedź." data-en="Kora is not replaying a fixed story from a list. The starting point is the prompt, current context, Mood Panel state and a generated reply.">Kora is not replaying a fixed story from a list. The starting point is the prompt, current context, Mood Panel state and a generated reply.</blockquote>
      <p data-pl="Panel Nastroju ma wpływać na charakter, nie na samo rozpoznanie pytania: humor, sarkazm, ciepło, ciekawość i energia mogą zmienić ton oraz kierunek opowieści. Ponieważ wejście i generowanie zmieniają się między próbami, podobny temat może skończyć się inną historią. Nie nazywam tego jednak matematyczną gwarancją unikalności — twardy anty-duplikator z pamięcią/hashowaniem byłby osobną, mierzalną funkcją." data-en="The Mood Panel is meant to shape character, not decide whether the request was understood: humour, sarcasm, warmth, curiosity and energy can change the tone and direction of a story. Because the inputs and generation vary between runs, a similar topic can end as a different story. I do not call that a mathematical uniqueness guarantee — a strict memory/hash anti-duplicate layer would be a separate, measurable feature.">The Mood Panel is meant to shape character, not decide whether the request was understood: humour, sarcasm, warmth, curiosity and energy can change the tone and direction of a story. Because the inputs and generation vary between runs, a similar topic can end as a different story. I do not call that a mathematical uniqueness guarantee — a strict memory/hash anti-duplicate layer would be a separate, measurable feature.</p>
      <p data-pl="Chcesz historię tylko dla siebie? Zostaw imię i trzy słowa pod Shortem. Wybrane propozycje podam Korze przy jej aktualnym nastroju i zobaczymy, gdzie tym razem pójdzie." data-en="Want a story made for you? Leave a name and three words under the Short. I will feed selected prompts to Kora in her current mood and see where she takes them this time.">Want a story made for you? Leave a name and three words under the Short. I will feed selected prompts to Kora in her current mood and see where she takes them this time.</p>`;
    episodes.insertBefore(note, episodes.children[1] || null);
  }

  const style = document.createElement('style');
  style.textContent = `
    #kora-says .series-stand-test{border-color:#ff8a00!important;box-shadow:inset 0 0 0 1px rgba(255,138,0,.9)!important}
    #kora-says .series-stand-test blockquote{border-left-color:#ff8a00!important}
    #kora-says .series-story-test{border-color:#a28bff!important;box-shadow:inset 0 0 0 1px rgba(162,139,255,.9)!important}
    #kora-says .story-engine-note{border-color:#6ee7c3!important;background:linear-gradient(145deg,rgba(110,231,195,.07),rgba(162,139,255,.055))!important}
    #kora-says .story-engine-note blockquote{border-left-color:#6ee7c3!important}
  `;
  document.head.appendChild(style);
})();