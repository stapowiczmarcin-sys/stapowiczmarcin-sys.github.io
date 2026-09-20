/* Portfolio bootstrap wrapper — preserve the existing portfolio-de.js logic and apply the 2026-09-20 Kora homepage status patch. */
(() => {
  try {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', 'portfolio-de-base-20260920.js?v=20260920-1', false);
    xhr.send(null);
    if ((xhr.status >= 200 && xhr.status < 300) || xhr.status === 0) {
      (0, eval)(xhr.responseText);
    } else {
      console.error('Portfolio base script failed to load:', xhr.status);
    }
  } catch (error) {
    console.error('Portfolio base script load error:', error);
  }

  const kora = document.getElementById('kora');
  if (!kora || kora.dataset.walkingHomepage20260920 === '1') return;
  kora.dataset.walkingHomepage20260920 = '1';

  const sectionIntro = kora.querySelector('.section-head p');
  if (sectionIntro) {
    sectionIntro.dataset.en = 'Kora now walks and turns on six legs. Her head and moving eyes are installed; the next tests focus on foot switches, LiDAR-driven reactions and mood-driven body language.';
    sectionIntro.dataset.pl = 'Kora już chodzi i skręca na sześciu nogach. Głowa i ruchome oczy są zamontowane; kolejne testy dotyczą switchy w stopach, reakcji z LiDAR-em i mowy ciała zależnej od nastroju.';
    sectionIntro.textContent = sectionIntro.dataset.en;
  }

  const heroCopy = kora.querySelector('.kora-hero p');
  if (heroCopy) {
    heroCopy.dataset.en = 'The six-legged MG996R platform now walks and turns in real tests. Kora also has her head and moving eyes installed. Current work is moving from basic locomotion toward foot contact sensing, LiDAR reactions and body language linked to mood.';
    heroCopy.dataset.pl = 'Sześcionożna platforma z MG996R już chodzi i skręca w realnych testach. Kora ma też zamontowaną głowę i ruchome oczy. Obecne prace przechodzą od podstawowego chodu do czucia kontaktu stóp, reakcji z LiDAR-em i mowy ciała zależnej od nastroju.';
    heroCopy.textContent = heroCopy.dataset.en;
  }

  const statusCards = Array.from(kora.querySelectorAll('.hardware-status article'));
  const mobilityCard = statusCards.find((article) => {
    const title = article.querySelector('strong');
    return title && (title.dataset.en === 'Latest leg test' || title.dataset.pl === 'Najnowszy test nóg');
  });

  if (mobilityCard) {
    const title = mobilityCard.querySelector('strong');
    const text = mobilityCard.querySelector('span');
    if (title) {
      title.dataset.en = 'Current mobility';
      title.dataset.pl = 'Aktualny ruch';
      title.textContent = title.dataset.en;
    }
    if (text) {
      text.dataset.en = 'Walking and turning are now confirmed in real tests. The head and moving eyes are installed. Foot-switch tests are next; LiDAR is active, with movement-reaction integration still in progress.';
      text.dataset.pl = 'Chodzenie i skręcanie są już potwierdzone w realnych testach. Głowa i ruchome oczy są zamontowane. Następne są testy switchy w stopach; LiDAR działa, a integracja reakcji ruchowych jest nadal w toku.';
      text.textContent = text.dataset.en;
    }
  }

  const buttons = kora.querySelector('.hero-buttons');
  if (buttons && !buttons.querySelector('[data-kora-latest-update="20260920"]')) {
    const latest = document.createElement('a');
    latest.className = 'button button-primary';
    latest.href = 'updates/kora-walking-2026-09-20/';
    latest.dataset.koraLatestUpdate = '20260920';
    latest.dataset.en = 'Latest Kora update →';
    latest.dataset.pl = 'Najnowsza aktualizacja Kory →';
    latest.textContent = latest.dataset.en;
    buttons.prepend(latest);
  }
})();