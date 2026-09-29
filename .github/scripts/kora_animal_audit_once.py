from pathlib import Path, PurePosixPath
from textwrap import dedent
import hashlib
import shutil
import zipfile

INO = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino')
ZIP = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC.zip')
EXPECTED_SOURCE_SHA256 = 'c0d332562537146318558af11b8d2e5febc9d8048e92966e5c711770a48acbf0'


def nl_for(data: bytes) -> bytes:
    return b'\r\n' if b'\r\n' in data else b'\n'


def block_bytes(text: str, nl: bytes) -> bytes:
    return dedent(text).strip('\n').encode('utf-8').replace(b'\n', nl)


def replace_once(data: bytes, old: bytes, new: bytes, label: str) -> bytes:
    n = data.count(old)
    if n != 1:
        raise RuntimeError(f'{label}: expected exactly one anchor, found {n}')
    return data.replace(old, new, 1)


data = INO.read_bytes()
source_sha = hashlib.sha256(data).hexdigest()
if source_sha != EXPECTED_SOURCE_SHA256:
    raise RuntimeError(f'Animal source changed since audit: {source_sha} != {EXPECTED_SOURCE_SHA256}')
if b'lightSquint' in data:
    raise RuntimeError('Animal already contains lightSquint; refusing duplicate patch')
if b'upper.startsWith("LOOK ")' in data:
    raise RuntimeError('Animal already contains LOOK; refusing duplicate patch')

nl = nl_for(data)

LIGHT_STATE = r'''
// Bright-light reflex: above 65% ambient the lids progressively squint.
// 65-80% = slight squint, 80-92% = clear squint, 92-100% = nearly closed.
static float lightSquint = 0.0f;
static uint32_t lightSquintAt = 0;
static float lightSquintTarget(float pct) {
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;
  if (pct <= 65.0f) return 0.0f;
  if (pct <= 80.0f) return 0.25f * (pct - 65.0f) / 15.0f;
  if (pct <= 92.0f) return 0.25f + 0.35f * (pct - 80.0f) / 12.0f;
  return 0.60f + 0.30f * (pct - 92.0f) / 8.0f;
}
'''

LIGHT_UPDATE = r'''
  // Natural photophobia reflex: close faster in glare, reopen more gently.
  const float squintTarget = lightSquintTarget(lightFiltered01 * 100.0f);
  float squintDt = lightSquintAt ? float(uint32_t(now - lightSquintAt)) * 0.001f : 0.0f;
  lightSquintAt = now;
  if (squintDt > 0.10f) squintDt = 0.10f;
  const float squintTau = (squintTarget > lightSquint) ? 0.16f : 0.70f;
  if (squintDt > 0.0f) lightSquint += (squintTarget - lightSquint) * (squintDt / (squintTau + squintDt));
  else lightSquint = squintTarget;
'''

LIGHT_RENDER = r'''
  // Ambient-light squint is independent from the FSR squash reflex.
  // At maximum glare only about 10% eyelid opening remains.
  if (!quietMode && crtState != 2) {
    oL *= (1.0f - lightSquint);
    oR *= (1.0f - lightSquint);
  }
'''

LOOK_BLOCK = r'''
  if (upper.startsWith("LOOK ")) {
    String arg = upper.substring(5);
    arg.trim();
    int sep = arg.indexOf(' ');
    if (sep <= 0) { Serial.println("ERR LOOK x y (-1.0..1.0)"); return; }
    String sx = arg.substring(0, sep);
    String sy = arg.substring(sep + 1);
    sx.trim(); sy.trim();
    char *endX = nullptr, *endY = nullptr;
    float x = strtof(sx.c_str(), &endX);
    float y = strtof(sy.c_str(), &endY);
    if (!sx.length() || !sy.length() || *endX || *endY || x < -1.0f || x > 1.0f || y < -1.0f || y > 1.0f) {
      Serial.println("ERR LOOK x y (-1.0..1.0)");
      return;
    }
    if (quietMode) wakeFromPi(0, "idle_watch");
    lastInteractionMs = millis();
    autoSlept = false;
    Kora3D::centeredGaze = false;
    faceTarget.lookX = x;
    faceTarget.lookY = y;
    Serial.println("OK LOOK " + String(x, 2) + " " + String(y, 2));
    return;
  }
'''

state_anchor = b'static float lightPupilOffset = 0.0f;' + nl + b'static uint32_t lightSampleAt = 0;'
state_new = b'static float lightPupilOffset = 0.0f;' + nl + block_bytes(LIGHT_STATE, nl) + nl + b'static uint32_t lightSampleAt = 0;'
data = replace_once(data, state_anchor, state_new, 'light squint state')

update_anchor = b'  lightPupilOffset = (0.5f - lightFiltered01) * 0.70f;' + nl + b'}'
update_new = b'  lightPupilOffset = (0.5f - lightFiltered01) * 0.70f;' + nl + block_bytes(LIGHT_UPDATE, nl) + nl + b'}'
data = replace_once(data, update_anchor, update_new, 'light squint update')

render_anchor = (
    b'  oL *= (1.0f - 0.80f * sqL);' + nl +
    b'  oR *= (1.0f - 0.80f * sqR);' + nl + nl +
    b'  oL *= displayPower.lid; oR *= displayPower.lid;'
)
render_new = (
    b'  oL *= (1.0f - 0.80f * sqL);' + nl +
    b'  oR *= (1.0f - 0.80f * sqR);' + nl + nl +
    block_bytes(LIGHT_RENDER, nl) + nl + nl +
    b'  oL *= displayPower.lid; oR *= displayPower.lid;'
)
data = replace_once(data, render_anchor, render_new, 'render squint')

fsr_anchor = b'  if (upper == "FSRSTATUS") {'
if data.count(fsr_anchor) != 1:
    raise RuntimeError(f'FSRSTATUS anchor count={data.count(fsr_anchor)}')
start = data.index(fsr_anchor)
marker = nl + b'  lastInteractionMs = millis();'
pos = data.find(marker, start)
if pos < 0:
    raise RuntimeError('LOOK insertion point not found after FSRSTATUS')
data = data[:pos] + nl + block_bytes(LOOK_BLOCK, nl) + nl + data[pos:]

help_old = b'  Serial.println("GAZE CENTER (default) / GAZE FREE");'
help_new = b'  Serial.println("GAZE CENTER (default) / GAZE FREE | LOOK x y (-1.0..1.0)");'
data = replace_once(data, help_old, help_new, 'startup LOOK help')

for needle, label in [
    (b'static constexpr int PIN_LIGHT_L = 3;', 'GPIO3'),
    (b'static constexpr int PIN_LIGHT_R = 4;', 'GPIO4'),
    (b'analogRead(PIN_LIGHT_L)', 'left ADC'),
    (b'analogRead(PIN_LIGHT_R)', 'right ADC'),
    (b'static float lightSquintTarget(float pct)', 'squint curve'),
    (b'lightSquint += (squintTarget - lightSquint)', 'squint smoothing'),
    (b'oL *= (1.0f - lightSquint);', 'left lid squint'),
    (b'oR *= (1.0f - lightSquint);', 'right lid squint'),
    (b'if (upper.startsWith("LOOK ")) {', 'LOOK parser'),
    (b'Kora3D::centeredGaze = false;', 'free gaze'),
    (b'faceTarget.lookX = x;', 'LOOK X'),
    (b'faceTarget.lookY = y;', 'LOOK Y'),
]:
    if data.count(needle) != 1:
        raise RuntimeError(f'{label}: expected once, found {data.count(needle)}')

INO.write_bytes(data)
print('SOURCE OK before', source_sha, 'after', hashlib.sha256(data).hexdigest(), 'delta', len(data) - 1707577)

# Rebuild public ZIP by replacing exactly the Animal INO member and preserving every other member.
tmp = ZIP.with_name(ZIP.name + '.tmp')
replaced = 0
with zipfile.ZipFile(ZIP, 'r') as src, zipfile.ZipFile(tmp, 'w') as dst:
    for info in src.infolist():
        payload = src.read(info.filename)
        if PurePosixPath(info.filename).name == INO.name:
            payload = INO.read_bytes()
            replaced += 1
        dst.writestr(info, payload)
if replaced != 1:
    tmp.unlink(missing_ok=True)
    raise RuntimeError(f'ZIP expected one matching INO, got {replaced}')
shutil.move(tmp, ZIP)
with zipfile.ZipFile(ZIP, 'r') as check:
    bad = check.testzip()
    if bad is not None:
        raise RuntimeError(f'ZIP CRC failure in {bad}')
    matches = [check.read(n) for n in check.namelist() if PurePosixPath(n).name == INO.name]
    if len(matches) != 1 or matches[0] != INO.read_bytes():
        raise RuntimeError('ZIP embedded INO does not match source')
print('ZIP OK', hashlib.sha256(ZIP.read_bytes()).hexdigest())

# Sanity-check the same progressive curve used by the V18 trio.
def target(p):
    if p <= 65: return 0.0
    if p <= 80: return 0.25*(p-65)/15
    if p <= 92: return 0.25+0.35*(p-80)/12
    return 0.60+0.30*(p-92)/8
for p in (0, 50, 65, 75, 80, 86, 92, 96, 100):
    q = target(p)
    print(f'light={p:3d}% squint={q:.3f} opening={1-q:.3f}')
if not (abs(target(65)-0.0)<1e-9 and abs(target(80)-0.25)<1e-9 and abs(target(92)-0.60)<1e-9 and abs(target(100)-0.90)<1e-9):
    raise RuntimeError('squint curve sanity failed')
print('ALL ANIMAL CHECKS PASSED')
