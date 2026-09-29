from pathlib import Path
import hashlib, subprocess

HEADER = b'// SLEEP10 update 2026-09-29: default inactivity sleep = 10000 ms.\r\n'
def C(s):
    return s.replace('\n', '\r\n').encode('utf-8')

OPS = [
    (C('static uint32_t wakeTimeoutMs = 0;\n'), C('static constexpr uint32_t DEFAULT_IDLE_SLEEP_MS = 10000; // 10 s without external activity.\nstatic uint32_t wakeTimeoutMs = DEFAULT_IDLE_SLEEP_MS;\n')),
    (C('static constexpr uint32_t BOOT_AWAKE_MS=0; // 0 = stay awake until Pi requests sleep.\n'), C('static constexpr uint32_t BOOT_AWAKE_MS=DEFAULT_IDLE_SLEEP_MS; // Auto-sleep after 10 s idle.\n')),
    (C('''  if (fsrPressed) {
    lastInteractionMs = now;
    autoSlept = false;

    if (quietMode && !wasPressed) {
      wakeFromPi(5000, "surprised");
    }'''), C('''  if (fsrPressed) {
    // A new press is activity; a continuously held HIGH must not hold the eyes open forever.
    if (!wasPressed) {
      lastInteractionMs = now;
      autoSlept = false;
    }

    if (quietMode && !wasPressed) {
      wakeFromPi(DEFAULT_IDLE_SLEEP_MS, "surprised");
    }''')),
    (C('''  if (!quietMode && wakeTimeoutMs > 0 && now - lastInteractionMs > wakeTimeoutMs) {
    enterPiSleep(true);
    if (!autoSlept) {
      autoSlept = true;
      Serial.println("OK AUTO_SLEEP timeout=" + String(wakeTimeoutMs));
'''), C('''  if (!quietMode && wakeTimeoutMs > 0 && now - lastInteractionMs >= wakeTimeoutMs) {
    const uint32_t elapsedTimeoutMs = wakeTimeoutMs;
    enterPiSleep(true);
    if (!autoSlept) {
      autoSlept = true;
      Serial.println("OK AUTO_SLEEP timeout=" + String(elapsedTimeoutMs));
''')),
    (C('''  if (talkMode && !quietMode && now >= nextTalkMs) {
    nextTalkMs = now + (uint32_t)random(120, 240);
    faceTarget.lookX = clampF(faceTarget.lookX + (float)random(-10, 11) / 100.0f, -0.55f, 0.55f);
    faceTarget.lookY = clampF(faceTarget.lookY + (float)random(-7, 8) / 100.0f, -0.30f, 0.30f);
    setEarLogicalTargets(10 + random(-6, 7), 10 + random(-6, 7));
    lastInteractionMs = now;
  }
'''), C('''  if (talkMode && !quietMode && now >= nextTalkMs) {
    nextTalkMs = now + (uint32_t)random(120, 240);
    faceTarget.lookX = clampF(faceTarget.lookX + (float)random(-10, 11) / 100.0f, -0.55f, 0.55f);
    faceTarget.lookY = clampF(faceTarget.lookY + (float)random(-7, 8) / 100.0f, -0.30f, 0.30f);
    setEarLogicalTargets(10 + random(-6, 7), 10 + random(-6, 7));
    // Only incoming TALK/AUDIO activity refreshes the idle timer, not this animation.
  }
''')),
    (C('''static void wakeFromPi(uint32_t ms = 0, const String &moodName = "idle_watch") {
  wakeTimeoutMs = ms;
'''), C('''static void wakeFromPi(uint32_t ms = DEFAULT_IDLE_SLEEP_MS, const String &moodName = "idle_watch") {
  wakeTimeoutMs = ms ? ms : DEFAULT_IDLE_SLEEP_MS; // WAKE / HTTP ms=0 uses the default.
''')),
    (C('  wakeTimeoutMs = 0;\n'), C('  wakeTimeoutMs = DEFAULT_IDLE_SLEEP_MS; // Also arm direct MOOD/API wake paths.\n')),
    (C('    wakeFromPi(8000, "human_found");\n'), C('    wakeFromPi(DEFAULT_IDLE_SLEEP_MS, "human_found");\n')),
    (C('    if (quietMode) wakeFromPi(8000, "idle_watch");\n'), C('    if (quietMode) wakeFromPi(DEFAULT_IDLE_SLEEP_MS, "idle_watch");\n')),
]

TARGETS = {
    'eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino': '8937eb33509f947d373ac6186a71f47fbb8e4d78485918ea30a62e1bef9d4860',
    'eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino': '13f72fb4d64b27c8ea2ca35be0ccdb71654ee5781467e473ced2253061835f41',
    'eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino': '430be6047c35da725f7e2be533906b701c22abb26bd0d7c761d69622f060c219',
}
human_old = C('// WAKE / WAKE UP: brighten closed lids, then open, stay awake.\n')
human_new = C('// WAKE / WAKE UP: brighten closed lids, then open; auto-sleep after 10 s without external activity.\n')

for target, expected in TARGETS.items():
    p = Path(target)
    b = p.read_bytes()
    if b.startswith(HEADER):
        raise SystemExit(f'Already updated unexpectedly: {target}')
    if 'KoraHumanMetal' in target:
        if b.count(human_old) != 1:
            raise SystemExit(f'Human header anchor mismatch: {target}')
        b = b.replace(human_old, human_new, 1)
    for old, new in OPS:
        count = b.count(old)
        if count != 1:
            raise SystemExit(f'Anchor count {count}, expected 1: {target}: {old[:100]!r}')
        b = b.replace(old, new, 1)
    b = HEADER + b
    got = hashlib.sha256(b).hexdigest()
    if got != expected:
        raise SystemExit(f'HASH MISMATCH {target}: {got} != {expected}')
    p.write_bytes(b)
    print(f'VERIFIED {target} sha256={got} bytes={len(b)}')

for d in ['KoraMetalEyesV18_S3_PUBLIC','KoraHumanMetalV18_S3_PUBLIC','KoraMonsterEyesV18_S3_PUBLIC']:
    z = Path('eyes') / (d + '.zip')
    if z.exists():
        z.unlink()
    subprocess.run(['zip','-qr',z.name,d], cwd='eyes', check=True)

Path('.github/workflows/eyes-sleep10-upgrade.yml').unlink(missing_ok=True)
Path('.github/eyes_sleep10_patch.py').unlink(missing_ok=True)
