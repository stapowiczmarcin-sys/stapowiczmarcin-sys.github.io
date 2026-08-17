from pathlib import Path

INDEX = Path("index.html")
text = INDEX.read_text(encoding="utf-8")

MARKER = "professional-nav-2026-08-17"
if MARKER in text:
    print("Navigation patch already applied; nothing to do.")
    raise SystemExit(0)

header_start = text.find('  <header class="topbar">')
if header_start < 0:
    raise SystemExit("Expected topbar header not found; refusing to patch.")
header_end_token = "  </header>"
header_end = text.find(header_end_token, header_start)
if header_end < 0:
    raise SystemExit("Topbar closing tag not found; refusing to patch.")
header_end += len(header_end_token)

old_header = text[header_start:header_end]
required_header_tokens = [
    'class="brand"',
    'class="top-actions"',
    'id="languageSelect"',
    'button-gotchas',
    'button-youtube',
]
missing = [token for token in required_header_tokens if token not in old_header]
if missing:
    raise SystemExit(f"Topbar shape changed ({missing}); refusing to patch.")

new_header = '''  <!-- professional-nav-2026-08-17 -->
  <header class="topbar">
    <div class="shell topbar-inner">
      <a class="brand" href="#start" aria-label="Marcin — Maker Portfolio">
        <span class="brand-mark">M</span>
        <span>Marcin / Maker Portfolio</span>
      </a>

      <button class="nav-menu-toggle" id="navMenuToggle" type="button" aria-expanded="false" aria-controls="siteNavPanel" aria-label="Open navigation">
        <span class="nav-menu-icon" aria-hidden="true"><i></i><i></i><i></i></span>
        <span>Menu</span>
      </button>

      <div class="top-actions nav-shell" id="siteNavPanel">
        <nav class="site-nav" aria-label="Portfolio navigation">
          <a href="#cnc">CNC</a>
          <a href="#kora">Kora</a>
          <a href="#radio" data-pl="Elektronika" data-en="Electronics">Electronics</a>
          <a href="https://stapowiczmarcin-sys.github.io/parts/#catalog" target="_blank" rel="noopener" data-pl="Części" data-en="Parts">Parts</a>
          <a href="kora-brain/" data-pl="Mózg Kory" data-en="Kora brain">Kora brain</a>
        </nav>

        <span class="nav-separator" aria-hidden="true"></span>

        <div class="nav-utilities">
          <a class="nav-utility nav-warning" href="https://stapowiczmarcin-sys.github.io/parts/#gotchas" target="_blank" rel="noopener" data-pl="Pułapki" data-en="Gotchas" title="Parts: what went wrong">Gotchas</a>
          <a class="nav-utility nav-youtube" href="https://www.youtube.com/@kreatywnymorrcin" target="_blank" rel="noopener" aria-label="YouTube — Kreatywny Morrcin" title="YouTube — Kreatywny Morrcin">
            <span class="youtube-mark" aria-hidden="true"></span><span>YouTube</span>
          </a>
          <label class="language-picker" aria-label="Language"><span class="language-globe" aria-hidden="true">🌐</span><select id="languageSelect" aria-label="Language"><option value="en">EN</option><option value="pl">PL</option><option value="de">DE</option></select></label>
          <button class="nav-utility nav-print" type="button" onclick="window.print()" aria-label="Print or save as PDF" title="PDF"><span aria-hidden="true">⎙</span><span>PDF</span></button>
        </div>
      </div>
    </div>
  </header>'''

nav_css = r'''

    /* professional-nav-2026-08-17 */
    .topbar { box-shadow: 0 10px 38px rgba(0,0,0,.16); }
    .topbar-inner {
      min-height: 72px;
      display: flex;
      flex-direction: row;
      align-items: center;
      justify-content: space-between;
      gap: 18px;
      padding: 0;
    }
    .brand { flex: 0 0 auto; }
    .top-actions.nav-shell {
      flex: 1 1 auto;
      width: auto;
      display: flex;
      align-items: center;
      justify-content: flex-end;
      gap: 12px;
      flex-wrap: nowrap;
    }
    .site-nav,
    .nav-utilities {
      display: flex;
      align-items: center;
      gap: 3px;
    }
    .site-nav a,
    .nav-utility {
      min-height: 38px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 7px;
      padding: 8px 10px;
      border: 1px solid transparent;
      border-radius: 10px;
      color: #c8ced8;
      background: transparent;
      text-decoration: none;
      font: inherit;
      font-size: .82rem;
      font-weight: 800;
      line-height: 1;
      white-space: nowrap;
      cursor: pointer;
      transition: color .18s ease, background .18s ease, border-color .18s ease, transform .18s ease;
    }
    .site-nav a:hover,
    .site-nav a:focus-visible,
    .nav-utility:hover,
    .nav-utility:focus-visible {
      color: #fff;
      border-color: rgba(255,255,255,.12);
      background: rgba(255,255,255,.055);
      outline: none;
    }
    .site-nav a:hover,
    .nav-utility:hover { transform: translateY(-1px); }
    .nav-separator {
      width: 1px;
      height: 26px;
      flex: 0 0 1px;
      background: var(--line);
    }
    .nav-warning {
      color: #ffd3c2;
      border-color: rgba(255,116,88,.18);
      background: rgba(255,92,70,.055);
    }
    .nav-warning:hover,
    .nav-warning:focus-visible {
      border-color: rgba(255,116,88,.42);
      background: rgba(255,92,70,.1);
    }
    .nav-youtube .youtube-mark {
      width: 23px;
      height: 16px;
      border-radius: 5px;
      box-shadow: none;
    }
    .nav-youtube .youtube-mark::after {
      border-top-width: 4px;
      border-bottom-width: 4px;
      border-left-width: 6px;
    }
    .nav-print {
      color: #c8ced8;
      border: 1px solid transparent;
    }
    .nav-shell .language-picker {
      min-height: 38px;
      border-radius: 10px;
      padding-inline: 9px;
      background: rgba(255,255,255,.025);
    }
    .nav-menu-toggle {
      display: none;
      min-height: 40px;
      align-items: center;
      justify-content: center;
      gap: 9px;
      padding: 8px 11px;
      border: 1px solid var(--line);
      border-radius: 11px;
      color: var(--text);
      background: rgba(255,255,255,.035);
      font: inherit;
      font-size: .82rem;
      font-weight: 850;
      cursor: pointer;
    }
    .nav-menu-icon { width: 18px; display: grid; gap: 4px; }
    .nav-menu-icon i { display: block; height: 2px; border-radius: 2px; background: currentColor; transition: transform .18s ease, opacity .18s ease; }
    .nav-menu-toggle[aria-expanded="true"] .nav-menu-icon i:nth-child(1) { transform: translateY(6px) rotate(45deg); }
    .nav-menu-toggle[aria-expanded="true"] .nav-menu-icon i:nth-child(2) { opacity: 0; }
    .nav-menu-toggle[aria-expanded="true"] .nav-menu-icon i:nth-child(3) { transform: translateY(-6px) rotate(-45deg); }

    @media (max-width: 1080px) {
      .topbar { position: sticky; }
      .topbar-inner {
        position: relative;
        min-height: 64px;
        flex-direction: row;
        align-items: center;
        gap: 12px;
        padding: 10px 0;
      }
      .brand { min-height: 42px; }
      .nav-menu-toggle { display: inline-flex; margin-left: auto; }
      .top-actions.nav-shell {
        position: absolute;
        top: 100%;
        left: -1px;
        right: -1px;
        z-index: 30;
        width: calc(100% + 2px);
        display: none;
        flex-direction: column;
        align-items: stretch;
        gap: 10px;
        padding: 12px;
        border: 1px solid var(--line);
        border-top: 0;
        border-radius: 0 0 17px 17px;
        background: rgba(8,11,16,.97);
        box-shadow: 0 24px 55px rgba(0,0,0,.38);
        backdrop-filter: blur(20px);
      }
      .top-actions.nav-shell.is-open { display: flex; }
      .site-nav,
      .nav-utilities {
        width: 100%;
        display: grid;
        grid-template-columns: repeat(2, minmax(0,1fr));
        gap: 7px;
      }
      .site-nav a,
      .nav-utility,
      .nav-shell .language-picker {
        width: 100%;
        min-height: 42px;
        justify-content: flex-start;
        padding: 9px 12px;
        border: 1px solid var(--line);
        background: rgba(255,255,255,.025);
      }
      .nav-separator { display: none; }
      .nav-shell .language-picker { border-radius: 10px; }
    }

    @media (max-width: 560px) {
      .brand span:last-child { display: inline; max-width: 150px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
      .site-nav,
      .nav-utilities { grid-template-columns: 1fr 1fr; }
      .site-nav a,
      .nav-utility,
      .nav-shell .language-picker { font-size: .8rem; }
    }

    @media (max-width: 390px) {
      .brand span:last-child { display: none; }
    }
'''

nav_js = r'''

  <script>
    // professional-nav-2026-08-17
    (() => {
      const toggle = document.getElementById('navMenuToggle');
      const panel = document.getElementById('siteNavPanel');
      if (!toggle || !panel) return;

      const closeMenu = () => {
        panel.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      };

      toggle.addEventListener('click', () => {
        const willOpen = !panel.classList.contains('is-open');
        panel.classList.toggle('is-open', willOpen);
        toggle.setAttribute('aria-expanded', String(willOpen));
      });

      panel.querySelectorAll('a').forEach((link) => link.addEventListener('click', closeMenu));

      document.addEventListener('keydown', (event) => {
        if (event.key === 'Escape') closeMenu();
      });

      document.addEventListener('click', (event) => {
        if (window.matchMedia('(max-width: 1080px)').matches && !panel.contains(event.target) && !toggle.contains(event.target)) {
          closeMenu();
        }
      });

      window.addEventListener('resize', () => {
        if (window.innerWidth > 1080) closeMenu();
      });
    })();
  </script>
'''

style_close = text.find("  </style>")
if style_close < 0:
    raise SystemExit("Closing style tag not found; refusing to patch.")
text = text[:style_close] + nav_css + text[style_close:]

# Header offsets remain valid because CSS was inserted before the header.
header_start = text.find('  <header class="topbar">')
header_end = text.find(header_end_token, header_start)
if header_end < 0:
    raise SystemExit("Topbar closing tag disappeared; refusing to patch.")
header_end += len(header_end_token)
text = text[:header_start] + new_header + text[header_end:]

body_close = text.rfind("</body>")
if body_close < 0:
    raise SystemExit("Closing body tag not found; refusing to patch.")
text = text[:body_close] + nav_js + text[body_close:]

checks = {
    "patch marker": text.count(MARKER) >= 3,
    "single language selector": text.count('id="languageSelect"') == 1,
    "single mobile toggle": text.count('id="navMenuToggle"') == 1,
    "single nav panel": text.count('id="siteNavPanel"') == 1,
    "main content preserved": '<main id="start">' in text and '<section id="kora">' in text and '<section id="kontakt">' in text,
    "bilingual nav": 'data-pl="Elektronika" data-en="Electronics"' in text and 'data-pl="Części" data-en="Parts"' in text,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    raise SystemExit(f"Post-patch validation failed: {failed}")

INDEX.write_text(text, encoding="utf-8")
print("Professional navigation patch applied and validated.")
