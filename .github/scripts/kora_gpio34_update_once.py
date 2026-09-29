from pathlib import Path, PurePosixPath
import hashlib
import json
import shutil
import zipfile

V18 = [
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino'),
]
ANIMAL = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino')

for p in V18:
    s = p.read_text(encoding='utf-8')
    old = 'static constexpr int PIN_LIGHT_L=-1, PIN_LIGHT_R=-1;'
    new = 'static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;'
    if s.count(old) != 1:
        raise SystemExit(f'{p}: disabled light declaration count={s.count(old)}')
    s = s.replace(old, new, 1)
    s = s.replace(
        '// Disabled until the actual sensor GPIOs and divider calibration are supplied.',
        '// Ambient light sensors / czujniki swiatla: LEFT GPIO3, RIGHT GPIO4.',
        1,
    )
    if s.count(new) != 1:
        raise SystemExit(f'{p}: new GPIO declaration count={s.count(new)}')
    if 'analogRead(PIN_LIGHT_L)' not in s or 'analogRead(PIN_LIGHT_R)' not in s:
        raise SystemExit(f'{p}: dual ADC reads missing')
    p.write_text(s, encoding='utf-8')
    print('V18 SOURCE OK:', p)

s = ANIMAL.read_text(encoding='utf-8')
if 'PIN_LIGHT_L' in s or 'updateLightPupil()' in s:
    raise SystemExit('Animal V14 already has a light module; refusing duplicate insertion')

pin_anchor = 'static constexpr int PIN_FSR_R = 2;    // ADC1_CH1 - right / prawa\n'
if s.count(pin_anchor) != 1:
    raise SystemExit(f'Animal FSR anchor count={s.count(pin_anchor)}')

animal_light_block = '''

// ---------- ambient light sensors / czujniki swiatla ----------
// Same public wiring as V18: LEFT = GPIO3, RIGHT = GPIO4.
// The two readings are averaged so both pupils react together.
static constexpr int PIN_LIGHT_L = 3;
static constexpr int PIN_LIGHT_R = 4;
static constexpr int LIGHT_DARK_L = 0, LIGHT_BRIGHT_L = 4095;
static constexpr int LIGHT_DARK_R = 0, LIGHT_BRIGHT_R = 4095;
static bool lightLocalReady = false;
static bool lightFilterReady = false;
static float lightFiltered01 = 0.5f;
static float lightPupilOffset = 0.0f;
static uint32_t lightSampleAt = 0;

static float normalizeLight01(int raw, int dark, int bright) {
  if (dark == bright) return 0.5f;
  float v = float(raw - dark) / float(bright - dark);
  if (v < 0.0f) v = 0.0f;
  if (v > 1.0f) v = 1.0f;
  return v;
}

static void setupLightSensors() {
  pinMode(PIN_LIGHT_L, INPUT);
  pinMode(PIN_LIGHT_R, INPUT);
  analogSetPinAttenuation(PIN_LIGHT_L, ADC_11db);
  analogSetPinAttenuation(PIN_LIGHT_R, ADC_11db);
  lightLocalReady = true;
}

static void updateLightPupil() {
  if (!lightLocalReady) return;
  const uint32_t now = millis();
  if (uint32_t(now - lightSampleAt) < 50u) return;
  lightSampleAt = now;

  const float l = normalizeLight01(analogRead(PIN_LIGHT_L), LIGHT_DARK_L, LIGHT_BRIGHT_L);
  const float r = normalizeLight01(analogRead(PIN_LIGHT_R), LIGHT_DARK_R, LIGHT_BRIGHT_R);
  const float ambient = (l + r) * 0.5f;

  if (!lightFilterReady) {
    lightFiltered01 = ambient;
    lightFilterReady = true;
  } else {
    lightFiltered01 += (ambient - lightFiltered01) * 0.16f;
  }

  // Bright -> smaller pupil, dark -> larger pupil.
  // Offset preserves Animal mood and breathing.
  lightPupilOffset = (0.5f - lightFiltered01) * 0.70f;
}
'''

s = s.replace(pin_anchor, pin_anchor + animal_light_block, 1)

render_old = 'float pupilL = clampF(faceNow.pupil + pupilBreath - sqL * 0.30f, 0.05f, 1.0f);'
render_new = 'float pupilL = clampF(faceNow.pupil + pupilBreath + lightPupilOffset - sqL * 0.30f, 0.05f, 1.0f);'
if s.count(render_old) != 1:
    raise SystemExit(f'Animal pupil render anchor count={s.count(render_old)}')
s = s.replace(render_old, render_new, 1)

setup_anchor = 'analogSetPinAttenuation(PIN_FSR_R, ADC_11db);\n'
if s.count(setup_anchor) != 1:
    raise SystemExit(f'Animal setup anchor count={s.count(setup_anchor)}')
s = s.replace(setup_anchor, setup_anchor + '  setupLightSensors();\n', 1)

loop_anchor = 'void loop() {\n'
if s.count(loop_anchor) != 1:
    raise SystemExit(f'Animal loop anchor count={s.count(loop_anchor)}')
s = s.replace(loop_anchor, loop_anchor + '  updateLightPupil();\n', 1)

for needle in [
    'static constexpr int PIN_LIGHT_L = 3;',
    'static constexpr int PIN_LIGHT_R = 4;',
    'analogRead(PIN_LIGHT_L)',
    'analogRead(PIN_LIGHT_R)',
    'setupLightSensors();',
    'updateLightPupil();',
    '+ lightPupilOffset - sqL * 0.30f',
]:
    if needle not in s:
        raise SystemExit(f'Animal validation missing: {needle}')
ANIMAL.write_text(s, encoding='utf-8')
print('ANIMAL SOURCE OK:', ANIMAL)

zip_map = {
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC.zip'): V18[0],
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC.zip'): V18[1],
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC.zip'): V18[2],
    Path('eyes/KoraAnimalEyesV14_S3_PUBLIC.zip'): ANIMAL,
}

for zpath, ino in zip_map.items():
    if not zpath.exists():
        raise SystemExit(f'Missing public ZIP: {zpath}')
    tmp = zpath.with_name(zpath.name + '.tmp')
    replaced = 0
    with zipfile.ZipFile(zpath, 'r') as src, zipfile.ZipFile(tmp, 'w') as dst:
        for info in src.infolist():
            data = src.read(info.filename)
            if PurePosixPath(info.filename).name == ino.name:
                data = ino.read_bytes()
                replaced += 1
            dst.writestr(info, data)
    if replaced != 1:
        tmp.unlink(missing_ok=True)
        raise SystemExit(f'{zpath}: expected one matching INO, got {replaced}')
    shutil.move(tmp, zpath)
    with zipfile.ZipFile(zpath, 'r') as check:
        bad = check.testzip()
        if bad is not None:
            raise SystemExit(f'{zpath}: CRC failure in {bad}')
    print('ZIP OK:', zpath)

manifest_path = Path('eyes/SHA256_V18.json')
manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
for zpath in list(zip_map)[:3]:
    digest = hashlib.sha256(zpath.read_bytes()).hexdigest()
    manifest[zpath.name] = digest
    print('SHA256', zpath.name, digest)
manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')

for p in V18:
    t = p.read_text(encoding='utf-8')
    if t.count('static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;') != 1:
        raise SystemExit(f'{p}: final GPIO declaration validation failed')
    if 'PIN_LIGHT_L=-1' in t or 'PIN_LIGHT_R=-1' in t:
        raise SystemExit(f'{p}: disabled GPIO declaration still present')

print('ALL SOURCE + ZIP VALIDATION PASSED')
