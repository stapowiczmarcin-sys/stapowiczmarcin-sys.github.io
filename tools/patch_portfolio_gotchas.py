from pathlib import Path

path = Path("index.html")
text = path.read_text(encoding="utf-8")

css_anchor = "    .button-primary:hover { background: #ffad67; }"
css_insert = """    .button-primary:hover { background: #ffad67; }
    .button-gotchas {
      border-color: rgba(255,92,92,.72);
      color: #fff;
      background: rgba(255,50,50,.14);
      font-weight: 900;
      white-space: nowrap;
    }
    .button-gotchas:hover { border-color: #ff5c5c; background: rgba(255,50,50,.24); }
"""
if ".button-gotchas {" not in text:
    if css_anchor not in text:
        raise SystemExit("CSS anchor not found")
    text = text.replace(css_anchor, css_insert, 1)

top_anchor = '        <a class="button" href="https://stapowiczmarcin-sys.github.io/parts/#catalog" target="_blank" rel="noopener" data-pl="Katalog części" data-en="Parts catalogue">Katalog części</a>'
top_link = '        <a class="button button-gotchas" href="https://stapowiczmarcin-sys.github.io/parts/#gotchas" target="_blank" rel="noopener" data-pl="🤬 DLACZEGO SIĘ WKURZYŁEM" data-en="🤬 WHY I GOT ANNOYED">🤬 DLACZEGO SIĘ WKURZYŁEM</a>'
if top_link not in text:
    if top_anchor not in text:
        raise SystemExit("Top navigation anchor not found")
    text = text.replace(top_anchor, top_anchor + "\n" + top_link, 1)

hero_anchor = '            <a class="button" href="https://stapowiczmarcin-sys.github.io/parts/#catalog" target="_blank" rel="noopener" data-pl="Katalog użytych części" data-en="Parts used in my projects">Katalog użytych części</a>'
hero_link = '            <a class="button button-gotchas" href="https://stapowiczmarcin-sys.github.io/parts/#gotchas" target="_blank" rel="noopener" data-pl="🤬 DLACZEGO SIĘ WKURZYŁEM" data-en="🤬 WHY I GOT ANNOYED">🤬 DLACZEGO SIĘ WKURZYŁEM</a>'
if hero_link not in text:
    if hero_anchor not in text:
        raise SystemExit("Hero catalogue anchor not found")
    text = text.replace(hero_anchor, hero_anchor + "\n" + hero_link, 1)

mobile_anchor = "      .button-youtube { gap: 7px; padding-inline: 11px; }"
mobile_line = "      .top-actions .button-gotchas { padding-inline: 10px; font-size: .7rem; }"
if mobile_line not in text:
    if mobile_anchor not in text:
        raise SystemExit("Mobile CSS anchor not found")
    text = text.replace(mobile_anchor, mobile_anchor + "\n" + mobile_line, 1)

# Keep the Kora Shorts sequence complete.
episode4 = '              <a class="button button-youtube" href="https://youtube.com/shorts/EDdY38zSmYM" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Odcinek #4 ↗" data-en="Episode #4 ↗">Episode #4 ↗</span></a>'
episode5_url = "https://youtube.com/shorts/IQUURKHAdes"
episode5 = '              <a class="button button-youtube" href="https://youtube.com/shorts/IQUURKHAdes" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Odcinek #5 ↗" data-en="Episode #5 ↗">Episode #5 ↗</span></a>'
if episode5_url not in text:
    if episode4 not in text:
        raise SystemExit("Episode #4 anchor not found")
    text = text.replace(episode4, episode4 + "\n" + episode5, 1)

# Episode #6: temporary plastic legs, all six active, first stand/sit tests.
episode6_url = "https://youtube.com/shorts/P-dCpj5-U8c"
episode6 = '              <a class="button button-youtube" href="https://youtube.com/shorts/P-dCpj5-U8c" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Odcinek #6: 6 plastikowych nóg — wstaje i siada ↗" data-en="Episode #6: 6 plastic legs — stand & sit ↗">Episode #6: 6 plastic legs — stand & sit ↗</span></a>'
old_episode6 = '              <a class="button button-youtube" href="https://youtube.com/shorts/P-dCpj5-U8c" target="_blank" rel="noopener"><span class="youtube-mark" aria-hidden="true"></span><span data-pl="Najnowszy test: 6 plastikowych nóg — wstaje i siada ↗" data-en="Latest test: 6 plastic legs — stand & sit ↗">Latest test: 6 plastic legs — stand & sit ↗</span></a>'
if old_episode6 in text:
    text = text.replace(old_episode6, episode6, 1)
elif episode6_url not in text:
    if episode5 not in text:
        raise SystemExit("Episode #5 anchor not found")
    text = text.replace(episode5, episode5 + "\n" + episode6, 1)

path.write_text(text, encoding="utf-8")

# Make the direct portfolio link open the gotchas panel immediately.
gotchas_path = Path("parts/gotchas.js")
gotchas = gotchas_path.read_text(encoding="utf-8")
old_end = "  ensureTab();\n  renderPanel();\n})();"
new_end = "  ensureTab();\n  renderPanel();\n  if (location.hash === \"#gotchas\" || new URLSearchParams(location.search).get(\"view\") === \"gotchas\") {\n    showGotchas();\n  }\n})();"
if "location.hash === \"#gotchas\"" not in gotchas:
    if old_end not in gotchas:
        raise SystemExit("Gotchas script ending anchor not found")
    gotchas = gotchas.replace(old_end, new_end, 1)
    gotchas_path.write_text(gotchas, encoding="utf-8")

# Bust the browser cache for the updated gotchas script.
parts_index_path = Path("parts/index.html")
parts_index = parts_index_path.read_text(encoding="utf-8")
parts_index = parts_index.replace('gotchas.js?v=20260816-1', 'gotchas.js?v=20260816-2')
parts_index_path.write_text(parts_index, encoding="utf-8")
