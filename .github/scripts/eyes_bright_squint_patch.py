from pathlib import Path, PurePosixPath
import hashlib
import re
import shutil
import zipfile

V18 = [
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino'),
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino'),
]
ANIMAL = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino')


def replace_once(data: bytes, old: bytes, new: bytes, label: str) -> bytes:
    count = data.count(old)
    if count != 1:
        raise SystemExit(f'{label}: expected exactly one anchor, found {count}')
    return data.replace(old, new, 1)


def nl_for(data: bytes) -> bytes:
    return b'\r\n' if data.count(b'\r\n') > data.count(b'\n') // 2 else b'\n'


def patch_v18(path: Path):
    data = path.read_bytes()
    if b'lightLidSquint' in data or b'lightReflexFactor' in data:
        raise SystemExit(f'{path}: reflex already present; refusing duplicate patch')
    nl = nl_for(data)

    anchor = b'static const char* lightSource="none";' + nl
    extra = (
        b'static const char* lightSource="none";' + nl +
        b'// Bright-light eyelid reflex / odruch powiek na bardzo mocne swiatlo.' + nl +
        b'// It is applied only at final render, so moods, blinks, FSR and sleep stay untouched.' + nl +
        b'static float lightLidSquint=1.0f,lightReflexFactor=1.0f;' + nl +
        b'static uint32_t lightReflexAt=0,lightReflexLastAt=0;' + nl
    )
    data = replace_once(data, anchor, extra, f'{path} globals')

    tick = b'  KoraPupil::control.tick(now,fresh,remoteFresh?lightRemote:lightLocal);' + nl
    reflex = (
        tick +
        b'  // Natural bright-light response: gradual squint above 78%, brief close/reopen above 98%.' + nl +
        b'  const float ambient=KoraPupil::control.hasLight?KoraPupil::control.filtered:0.0f;' + nl +
        b'  float squintTarget=1.0f;' + nl +
        b'  if(KoraPupil::control.hasLight&&ambient>78.0f){' + nl +
        b'    float t=(ambient-78.0f)/17.0f;if(t<0.0f)t=0.0f;if(t>1.0f)t=1.0f;' + nl +
        b'    t=t*t*(3.0f-2.0f*t);' + nl +
        b'    squintTarget=1.0f-0.40f*t;' + nl +
        b'  }' + nl +
        b'  lightLidSquint+=(squintTarget-lightLidSquint)*0.14f;' + nl +
        b'  if(KoraPupil::control.hasLight&&ambient>=98.0f&&lightReflexAt==0&&' + nl +
        b'     (lightReflexLastAt==0||uint32_t(now-lightReflexLastAt)>=2500u)){' + nl +
        b'    lightReflexAt=now;lightReflexLastAt=now;' + nl +
        b'  }' + nl +
        b'  lightReflexFactor=1.0f;' + nl +
        b'  if(lightReflexAt){' + nl +
        b'    const uint32_t e=uint32_t(now-lightReflexAt);' + nl +
        b'    if(e<80u)lightReflexFactor=1.0f-float(e)/80.0f;' + nl +
        b'    else if(e<140u)lightReflexFactor=0.0f;' + nl +
        b'    else if(e<300u)lightReflexFactor=float(e-140u)/160.0f;' + nl +
        b'    else {lightReflexAt=0;lightReflexFactor=1.0f;}' + nl +
        b'  }' + nl
    )
    data = replace_once(data, tick, reflex, f'{path} light update')

    lid = b'  oL *= displayPower.lid; oR *= displayPower.lid;' + nl
    lid_new = (
        lid +
        b'  // Preserve every existing lid source; light only modulates the final visible opening.' + nl +
        b'  if(crtState!=2){const float lightLid=lightLidSquint*lightReflexFactor;oL*=lightLid;oR*=lightLid;}' + nl
    )
    data = replace_once(data, lid, lid_new, f'{path} final lid')

    for needle in [
        b'PIN_LIGHT_L=3, PIN_LIGHT_R=4', b'analogRead(PIN_LIGHT_L)', b'analogRead(PIN_LIGHT_R)',
        b'lightLidSquint', b'lightReflexFactor', b'ambient>=98.0f', b'if(crtState!=2)'
    ]:
        if needle not in data:
            raise SystemExit(f'{path}: missing validation needle {needle!r}')
    path.write_bytes(data)
    print('PATCHED V18', path)


def patch_animal(path: Path):
    data = path.read_bytes()
    if b'lightLidSquint' in data or b'lightReflexFactor' in data:
        raise SystemExit(f'{path}: reflex already present; refusing duplicate patch')
    nl = nl_for(data)

    anchor = b'static float lightPupilOffset = 0.0f;' + nl
    extra = (
        anchor +
        b'// Bright-light eyelid reflex / odruch powiek na bardzo mocne swiatlo.' + nl +
        b'// Applied only at final render so Animal mood, blink, FSR and sleep logic remain unchanged.' + nl +
        b'static float lightLidSquint = 1.0f;' + nl +
        b'static float lightReflexFactor = 1.0f;' + nl +
        b'static uint32_t lightReflexAt = 0, lightReflexLastAt = 0;' + nl
    )
    data = replace_once(data, anchor, extra, f'{path} globals')

    offset = b'  lightPupilOffset = (0.5f - lightFiltered01) * 0.70f;' + nl
    reflex = (
        offset +
        b'' + nl +
        b'  // Same thresholds as V18: soft squint above 78%, short protective close above 98%.' + nl +
        b'  float squintTarget = 1.0f;' + nl +
        b'  if (lightFiltered01 > 0.78f) {' + nl +
        b'    float t = (lightFiltered01 - 0.78f) / 0.17f;' + nl +
        b'    if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;' + nl +
        b'    t = t * t * (3.0f - 2.0f * t);' + nl +
        b'    squintTarget = 1.0f - 0.40f * t;' + nl +
        b'  }' + nl +
        b'  lightLidSquint += (squintTarget - lightLidSquint) * 0.14f;' + nl +
        b'  if (lightFiltered01 >= 0.98f && lightReflexAt == 0 &&' + nl +
        b'      (lightReflexLastAt == 0 || uint32_t(now - lightReflexLastAt) >= 2500u)) {' + nl +
        b'    lightReflexAt = now; lightReflexLastAt = now;' + nl +
        b'  }' + nl +
        b'  lightReflexFactor = 1.0f;' + nl +
        b'  if (lightReflexAt) {' + nl +
        b'    const uint32_t e = uint32_t(now - lightReflexAt);' + nl +
        b'    if (e < 80u) lightReflexFactor = 1.0f - float(e) / 80.0f;' + nl +
        b'    else if (e < 140u) lightReflexFactor = 0.0f;' + nl +
        b'    else if (e < 300u) lightReflexFactor = float(e - 140u) / 160.0f;' + nl +
        b'    else { lightReflexAt = 0; lightReflexFactor = 1.0f; }' + nl +
        b'  }' + nl
    )
    data = replace_once(data, offset, reflex, f'{path} light update')

    lid = b'  oL *= displayPower.lid; oR *= displayPower.lid;' + nl
    lid_new = (
        lid +
        b'  // Preserve every existing lid source; light only modulates the final visible opening.' + nl +
        b'  if (crtState != 2) { const float lightLid = lightLidSquint * lightReflexFactor; oL *= lightLid; oR *= lightLid; }' + nl
    )
    data = replace_once(data, lid, lid_new, f'{path} final lid')

    for needle in [
        b'PIN_LIGHT_L = 3', b'PIN_LIGHT_R = 4', b'analogRead(PIN_LIGHT_L)', b'analogRead(PIN_LIGHT_R)',
        b'lightLidSquint', b'lightReflexFactor', b'lightFiltered01 >= 0.98f', b'if (crtState != 2)'
    ]:
        if needle not in data:
            raise SystemExit(f'{path}: missing validation needle {needle!r}')
    path.write_bytes(data)
    print('PATCHED ANIMAL', path)


def refresh_zip(zpath: Path, ino: Path):
    tmp = zpath.with_name(zpath.name + '.tmp')
    replaced = 0
    with zipfile.ZipFile(zpath, 'r') as src, zipfile.ZipFile(tmp, 'w') as dst:
        for info in src.infolist():
            payload = src.read(info.filename)
            if PurePosixPath(info.filename).name == ino.name:
                payload = ino.read_bytes()
                replaced += 1
            dst.writestr(info, payload)
    if replaced != 1:
        tmp.unlink(missing_ok=True)
        raise SystemExit(f'{zpath}: expected one INO entry, replaced {replaced}')
    shutil.move(tmp, zpath)
    with zipfile.ZipFile(zpath, 'r') as z:
        bad = z.testzip()
        if bad is not None:
            raise SystemExit(f'{zpath}: CRC failed at {bad}')
    print('ZIP OK', zpath)


for p in V18:
    patch_v18(p)
patch_animal(ANIMAL)

zip_map = {
    Path('eyes/KoraHumanMetalV18_S3_PUBLIC.zip'): V18[0],
    Path('eyes/KoraMetalEyesV18_S3_PUBLIC.zip'): V18[1],
    Path('eyes/KoraMonsterEyesV18_S3_PUBLIC.zip'): V18[2],
    Path('eyes/KoraAnimalEyesV14_S3_PUBLIC.zip'): ANIMAL,
}
for z, ino in zip_map.items():
    refresh_zip(z, ino)

manifest = Path('eyes/SHA256_V18.json')
text = manifest.read_text(encoding='utf-8')
for z in list(zip_map.keys())[:3]:
    digest = hashlib.sha256(z.read_bytes()).hexdigest()
    pattern = rf'("{re.escape(z.name)}"\s*:\s*")[0-9a-f]{{64}}(")'
    text, n = re.subn(pattern, rf'\g<1>{digest}\g<2>', text, count=1)
    if n != 1:
        raise SystemExit(f'{manifest}: could not update hash for {z.name}')
    print('SHA256', z.name, digest)
manifest.write_text(text, encoding='utf-8', newline='')

print('BRIGHT-LIGHT REFLEX PATCH COMPLETE')
