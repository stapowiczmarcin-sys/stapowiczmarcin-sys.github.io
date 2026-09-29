from pathlib import Path, PurePosixPath
import hashlib
import re
import shutil
import subprocess
import zipfile

BASE = '30db11179c660ea46acd371936a95bb2afc1a1d3'
V18 = [
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino'),
]
ANIMAL = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino')


def base_bytes(path: Path) -> bytes:
    return subprocess.check_output(['git', 'show', f'{BASE}:{path.as_posix()}'])


def replace_once(data: bytes, old: bytes, new: bytes, label: str) -> bytes:
    count = data.count(old)
    if count != 1:
        raise SystemExit(f'{label}: expected one match, got {count}')
    return data.replace(old, new, 1)


def regex_sub_once(data: bytes, pattern: bytes, repl, label: str) -> bytes:
    out, count = re.subn(pattern, repl, data, count=1)
    if count != 1:
        raise SystemExit(f'{label}: expected one regex match, got {count}')
    return out


# Restore original bytes first, then make only the intended V18 replacements.
for p in V18:
    data = base_bytes(p)
    data = replace_once(
        data,
        b'static constexpr int PIN_LIGHT_L=-1, PIN_LIGHT_R=-1;',
        b'static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;',
        f'{p} light pins',
    )
    data = replace_once(
        data,
        b'// Disabled until the actual sensor GPIOs and divider calibration are supplied.',
        b'// Ambient light sensors / czujniki swiatla: LEFT GPIO3, RIGHT GPIO4.',
        f'{p} light comment',
    )
    if b'analogRead(PIN_LIGHT_L)' not in data or b'analogRead(PIN_LIGHT_R)' not in data:
        raise SystemExit(f'{p}: ADC reads missing')
    p.write_bytes(data)
    print('V18 BYTE-PRESERVED:', p)

# Animal V14: restore original bytes and insert the small light module using its native newline style.
data = base_bytes(ANIMAL)
nl = b'\r\n' if b'\r\n' in data else b'\n'
if b'PIN_LIGHT_L' in data or b'updateLightPupil()' in data:
    raise SystemExit('Base Animal unexpectedly already contains a light module')

block_text = '''

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
block = block_text.replace('\n', nl.decode('ascii')).encode('utf-8')

# Match Animal anchors by meaning, not by exact spaces/comments.
data = regex_sub_once(
    data,
    rb'(?m)^(static constexpr int PIN_FSR_R\s*=\s*2;[^\r\n]*)(\r?\n)',
    lambda m: m.group(1) + m.group(2) + block,
    'Animal light block',
)

data = regex_sub_once(
    data,
    rb'(?m)^([ \t]*)float\s+pupilL\s*=\s*clampF\(faceNow\.pupil\s*\+\s*pupilBreath\s*-\s*sqL\s*\*\s*0\.30f\s*,\s*0\.05f\s*,\s*1\.0f\s*\);',
    lambda m: m.group(1) + b'float pupilL = clampF(faceNow.pupil + pupilBreath + lightPupilOffset - sqL * 0.30f, 0.05f, 1.0f);',
    'Animal pupil render',
)

data = regex_sub_once(
    data,
    rb'(?m)^([ \t]*analogSetPinAttenuation\(\s*PIN_FSR_R\s*,\s*ADC_11db\s*\);)(\r?\n)',
    lambda m: m.group(1) + m.group(2) + b'  setupLightSensors();' + m.group(2),
    'Animal setup',
)

data = regex_sub_once(
    data,
    rb'(?m)^(void\s+loop\s*\(\s*\)\s*\{)(\r?\n)',
    lambda m: m.group(1) + m.group(2) + b'  updateLightPupil();' + m.group(2),
    'Animal loop',
)

for needle in [
    b'static constexpr int PIN_LIGHT_L = 3;',
    b'static constexpr int PIN_LIGHT_R = 4;',
    b'analogRead(PIN_LIGHT_L)',
    b'analogRead(PIN_LIGHT_R)',
    b'setupLightSensors();',
    b'updateLightPupil();',
    b'+ lightPupilOffset - sqL * 0.30f',
]:
    if needle not in data:
        raise SystemExit(f'Animal validation missing {needle!r}')
ANIMAL.write_bytes(data)
print('ANIMAL BYTE-PRESERVED:', ANIMAL, 'newline=', 'CRLF' if nl == b'\r\n' else 'LF')

# Restore original ZIP packages, then replace exactly their INO member with corrected source bytes.
zip_map = {
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC.zip'): V18[0],
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC.zip'): V18[1],
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC.zip'): V18[2],
    Path('eyes/KoraAnimalEyesV14_S3_PUBLIC.zip'): ANIMAL,
}
for zpath, ino in zip_map.items():
    zpath.write_bytes(base_bytes(zpath))
    tmp = zpath.with_name(zpath.name + '.tmp')
    replaced = 0
    with zipfile.ZipFile(zpath, 'r') as src, zipfile.ZipFile(tmp, 'w') as dst:
        for info in src.infolist():
            member = src.read(info.filename)
            if PurePosixPath(info.filename).name == ino.name:
                member = ino.read_bytes()
                replaced += 1
            dst.writestr(info, member)
    if replaced != 1:
        tmp.unlink(missing_ok=True)
        raise SystemExit(f'{zpath}: expected one INO member, got {replaced}')
    shutil.move(tmp, zpath)
    with zipfile.ZipFile(zpath, 'r') as check:
        bad = check.testzip()
        if bad is not None:
            raise SystemExit(f'{zpath}: CRC failure in {bad}')
    print('ZIP CRC OK:', zpath)

# Preserve the manifest formatting/newlines from the original commit and replace only three digests.
manifest_path = Path('eyes/SHA256_V18.json')
manifest = base_bytes(manifest_path).decode('utf-8')
for zpath in list(zip_map)[:3]:
    digest = hashlib.sha256(zpath.read_bytes()).hexdigest()
    manifest_pattern = rf'("{re.escape(zpath.name)}"\s*:\s*")[0-9a-f]{{64}}(")'
    manifest, count = re.subn(manifest_pattern, rf'\g<1>{digest}\g<2>', manifest, count=1)
    if count != 1:
        raise SystemExit(f'Manifest entry replacement failed for {zpath.name}')
    print('SHA256', zpath.name, digest)
manifest_path.write_bytes(manifest.encode('utf-8'))

for p in V18:
    t = p.read_bytes()
    if t.count(b'static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;') != 1:
        raise SystemExit(f'{p}: final GPIO declaration validation failed')
    if b'PIN_LIGHT_L=-1' in t or b'PIN_LIGHT_R=-1' in t:
        raise SystemExit(f'{p}: disabled GPIO declaration still present')

print('BYTE-PRESERVING CLEANUP VALIDATION PASSED')
