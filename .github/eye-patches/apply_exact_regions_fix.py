from pathlib import Path
import base64
import re

source = Path('.github/eye-patches/apply_exact_regions.py').read_text(encoding='utf-8')


def payload(name):
    m = re.search(rf"^{name} = base64\.b64decode\('([^']+)'\)", source, re.M)
    if not m:
        raise SystemExit(f'missing embedded payload {name}')
    return base64.b64decode(m.group(1))


human_light = payload('HUMAN_LIGHT')
metal_light = payload('METAL_LIGHT')
draw = payload('DRAW')
serial = payload('SERIAL')

targets = {
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino'): human_light,
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino'): metal_light,
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino'): metal_light,
}


def require_index(data, anchor, start=0, label='anchor'):
    pos = data.find(anchor, start)
    if pos < 0:
        raise SystemExit(f'missing {label}: {anchor!r}')
    return pos


for path, light in targets.items():
    data = path.read_bytes()

    # Light block: locate the stable PIN declaration, then rewind to the blank
    # line before its old comments. Replace through handleIrisPupil.
    pin = require_index(data, b'static constexpr int PIN_LIGHT_L=', label=f'{path} PIN_LIGHT_L')
    blank = data.rfind(b'\n\n', 0, pin)
    if blank < 0:
        raise SystemExit(f'{path}: no blank line before PIN_LIGHT_L')
    start = blank + 2
    end = require_index(data, b'static bool handleIrisPupil', pin, f'{path} handleIrisPupil')
    data = data[:start] + light + data[end:]

    # Rendering squint block.
    start = require_index(data, b'  float sqL = ', label=f'{path} sqL')
    end = require_index(data, b'  oL *= displayPower.lid;', start, f'{path} displayPower')
    data = data[:start] + draw + data[end:]

    # Serial LIST -> LOOK block, through triggerEarPerk line.
    start = require_index(data, b'  if (upper == "LIST") { printBehaviorList(); return; }', label=f'{path} LIST')
    trig = require_index(data, b'  triggerEarPerk();', start, f'{path} triggerEarPerk')
    nl = data.find(b'\n', trig)
    end = len(data) if nl < 0 else nl + 1
    data = data[:start] + serial + data[end:]

    path.write_bytes(data)

for rej in Path('eyes').rglob('*.rej'):
    rej.unlink()
