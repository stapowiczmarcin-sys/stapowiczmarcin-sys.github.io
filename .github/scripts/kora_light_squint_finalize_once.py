from pathlib import Path, PurePosixPath
from textwrap import dedent
import subprocess, io, zipfile, hashlib, json

BASE = "30db11179c660ea46acd371936a95bb2afc1a1d3"

V18 = [
    Path("eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino"),
]
ANIMAL = Path("eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino")
ZIP_MAP = {
    Path("eyes/KoraHumanMetalV18_S3_PUBLIC.zip"): V18[0],
    Path("eyes/KoraMetalEyesV18_S3_PUBLIC.zip"): V18[1],
    Path("eyes/KoraMonsterEyesV18_S3_PUBLIC.zip"): V18[2],
    Path("eyes/KoraAnimalEyesV14_S3_PUBLIC.zip"): ANIMAL,
}
MANIFEST = Path("eyes/SHA256_V18.json")

def git_show_bytes(path: Path) -> bytes:
    return subprocess.check_output(["git", "show", f"{BASE}:{path.as_posix()}"])

def nl_for(data: bytes) -> bytes:
    return b"\r\n" if b"\r\n" in data else b"\n"

def block_bytes(text: str, nl: bytes) -> bytes:
    text = dedent(text).strip("\n") + "\n"
    return text.encode("utf-8").replace(b"\n", nl)

def replace_once(data: bytes, old: bytes, new: bytes, label: str) -> bytes:
    n = data.count(old)
    if n != 1:
        raise RuntimeError(f"{label}: expected exactly one anchor, found {n}")
    return data.replace(old, new, 1)

def assert_once(data: bytes, needle: bytes, label: str):
    n = data.count(needle)
    if n != 1:
        raise RuntimeError(f"{label}: expected once, found {n}")

GLARE_V18 = r"""
static uint32_t lightGlareBlinkAt=0;
static bool lightGlareLatched=false;

// Natural eyelid reflex from ambient light:
// <=75% no squint, 75..100% progressive squint,
// >=96% one brief full glare blink; re-arms below 88%.
static float lightLidResponse(uint32_t now){
  if(!KoraPupil::control.hasLight)return 1.0f;
  const float ambient=KoraPupil::control.filtered;

  if(ambient<88.0f)lightGlareLatched=false;
  if(ambient>=96.0f&&!lightGlareLatched){
    lightGlareLatched=true;
    lightGlareBlinkAt=now;
  }

  float base=1.0f;
  if(ambient>75.0f){
    float t=(ambient-75.0f)/25.0f;
    if(t>1.0f)t=1.0f;
    base=1.0f-0.38f*t;
  }

  if(lightGlareBlinkAt){
    const uint32_t dt=uint32_t(now-lightGlareBlinkAt);
    if(dt<320u){
      float blink=1.0f;
      if(dt<90u)blink=1.0f-float(dt)/90.0f;
      else if(dt<150u)blink=0.0f;
      else blink=float(dt-150u)/170.0f;
      if(blink<base)return blink;
    }
  }
  return base;
}
"""

ANIMAL_LIGHT = r"""
// ---------- ambient light sensors / czujniki swiatla ----------
// Public wiring: LEFT = GPIO3, RIGHT = GPIO4.
// Both ADC readings are averaged so both pupils and lids react together.
static constexpr int PIN_LIGHT_L = 3;
static constexpr int PIN_LIGHT_R = 4;
static constexpr int LIGHT_DARK_L = 0, LIGHT_BRIGHT_L = 4095;
static constexpr int LIGHT_DARK_R = 0, LIGHT_BRIGHT_R = 4095;
static bool lightLocalReady = false;
static bool lightFilterReady = false;
static float lightFiltered01 = 0.5f;
static float lightPupilOffset = 0.0f;
static uint32_t lightSampleAt = 0;
static uint32_t lightGlareBlinkAt = 0;
static bool lightGlareLatched = false;

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
  // This is only an offset; Animal's original mood/breathing remains intact.
  lightPupilOffset = (0.5f - lightFiltered01) * 0.70f;
}

// Natural eyelid reflex from ambient light:
// <=75% no squint, 75..100% progressive squint,
// >=96% one brief full glare blink; re-arms below 88%.
static float lightLidResponse(uint32_t now) {
  if (!lightLocalReady || !lightFilterReady) return 1.0f;
  const float ambient = lightFiltered01 * 100.0f;

  if (ambient < 88.0f) lightGlareLatched = false;
  if (ambient >= 96.0f && !lightGlareLatched) {
    lightGlareLatched = true;
    lightGlareBlinkAt = now;
  }

  float base = 1.0f;
  if (ambient > 75.0f) {
    float t = (ambient - 75.0f) / 25.0f;
    if (t > 1.0f) t = 1.0f;
    base = 1.0f - 0.38f * t;
  }

  if (lightGlareBlinkAt) {
    const uint32_t dt = uint32_t(now - lightGlareBlinkAt);
    if (dt < 320u) {
      float blink = 1.0f;
      if (dt < 90u) blink = 1.0f - float(dt) / 90.0f;
      else if (dt < 150u) blink = 0.0f;
      else blink = float(dt - 150u) / 170.0f;
      if (blink < base) return blink;
    }
  }
  return base;
}
"""

def build_v18(path: Path):
    data = git_show_bytes(path)
    nl = nl_for(data)
    old_pin = b"static constexpr int PIN_LIGHT_L=-1, PIN_LIGHT_R=-1;"
    new_pin = b"static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;"
    data = replace_once(data, old_pin, new_pin, f"{path}: light pins")
    data = replace_once(
        data,
        b"// Disabled until the actual sensor GPIOs and divider calibration are supplied.",
        b"// Ambient light sensors / czujniki swiatla: LEFT GPIO3, RIGHT GPIO4.",
        f"{path}: light comment",
    )
    handler = b"static bool handleIrisPupil(const String& upper){"
    data = replace_once(data, handler, block_bytes(GLARE_V18, nl) + nl + handler, f"{path}: glare function insertion")
    lid_anchor = b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    lid_new = lid_anchor + nl + b"  const float lightLid=lightLidResponse(now);" + nl + b"  oL*=lightLid; oR*=lightLid;"
    data = replace_once(data, lid_anchor, lid_new, f"{path}: lid response")

    assert_once(data, new_pin, f"{path}: final pins")
    assert_once(data, b"analogRead(PIN_LIGHT_L)", f"{path}: left ADC")
    assert_once(data, b"analogRead(PIN_LIGHT_R)", f"{path}: right ADC")
    assert_once(data, b"static float lightLidResponse(uint32_t now){", f"{path}: glare fn")
    assert_once(data, b"const float lightLid=lightLidResponse(now);", f"{path}: glare use")
    if old_pin in data:
        raise RuntimeError(f"{path}: disabled pins still present")
    path.write_bytes(data)
    print("V18 SOURCE OK:", path)

def build_animal():
    path = ANIMAL
    data = git_show_bytes(path)
    nl = nl_for(data)

    pin_anchor = b"static constexpr int PIN_FSR_R = 2;    // ADC1_CH1 - right / prawa"
    data = replace_once(data, pin_anchor, pin_anchor + nl + block_bytes(ANIMAL_LIGHT, nl), f"{path}: light block")

    setup_anchor = b"  analogSetPinAttenuation(PIN_FSR_R, ADC_11db);"
    data = replace_once(data, setup_anchor, setup_anchor + nl + b"  setupLightSensors();", f"{path}: setup")

    loop_anchor = b"void loop() {"
    data = replace_once(data, loop_anchor, loop_anchor + nl + b"  updateLightPupil();", f"{path}: loop")

    pupil_old = b"  float pupilL = clampF(faceNow.pupil + pupilBreath - sqL * 0.30f, 0.05f, 1.0f);"
    pupil_new = b"  float pupilL = clampF(faceNow.pupil + pupilBreath + lightPupilOffset - sqL * 0.30f, 0.05f, 1.0f);"
    data = replace_once(data, pupil_old, pupil_new, f"{path}: pupil")

    lid_anchor = b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    lid_new = lid_anchor + nl + b"  const float lightLid = lightLidResponse(now);" + nl + b"  oL *= lightLid; oR *= lightLid;"
    data = replace_once(data, lid_anchor, lid_new, f"{path}: lid response")

    for needle, label in [
        (b"static constexpr int PIN_LIGHT_L = 3;", "left pin"),
        (b"static constexpr int PIN_LIGHT_R = 4;", "right pin"),
        (b"analogRead(PIN_LIGHT_L)", "left ADC"),
        (b"analogRead(PIN_LIGHT_R)", "right ADC"),
        (b"static float lightLidResponse(uint32_t now)", "glare fn"),
        (b"const float lightLid = lightLidResponse(now);", "glare use"),
        (b"+ lightPupilOffset - sqL * 0.30f", "pupil offset"),
    ]:
        assert_once(data, needle, f"{path}: {label}")
    path.write_bytes(data)
    print("ANIMAL SOURCE OK:", path)

def rebuild_zip(zpath: Path, ino: Path):
    base_zip = git_show_bytes(zpath)
    src_bio = io.BytesIO(base_zip)
    out_bio = io.BytesIO()
    replaced = 0
    with zipfile.ZipFile(src_bio, "r") as src, zipfile.ZipFile(out_bio, "w") as dst:
        for info in src.infolist():
            payload = src.read(info.filename)
            if PurePosixPath(info.filename).name == ino.name:
                payload = ino.read_bytes()
                replaced += 1
            dst.writestr(info, payload)
    if replaced != 1:
        raise RuntimeError(f"{zpath}: expected one INO replacement, got {replaced}")
    zpath.write_bytes(out_bio.getvalue())
    with zipfile.ZipFile(zpath, "r") as check:
        bad = check.testzip()
        if bad is not None:
            raise RuntimeError(f"{zpath}: CRC failure in {bad}")
        matches = [check.read(n) for n in check.namelist() if PurePosixPath(n).name == ino.name]
        if len(matches) != 1 or matches[0] != ino.read_bytes():
            raise RuntimeError(f"{zpath}: embedded INO does not match source")
    print("ZIP OK:", zpath)

def update_manifest():
    manifest = json.loads(git_show_bytes(MANIFEST).decode("utf-8"))
    for zpath in list(ZIP_MAP.keys())[:3]:
        digest = hashlib.sha256(zpath.read_bytes()).hexdigest()
        manifest[zpath.name] = digest
        print("SHA256", zpath.name, digest)
    MANIFEST.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")

def validate_diff_size():
    cmd = ["git", "diff", "--numstat", BASE, "--"] + [p.as_posix() for p in V18 + [ANIMAL]]
    out = subprocess.check_output(cmd, text=True)
    print("--- SOURCE NUMSTAT VS CLEAN BASE ---")
    print(out, end="")
    for line in out.splitlines():
        a, d, name = line.split("\t", 2)
        if a == "-" or d == "-":
            raise RuntimeError(f"{name}: source unexpectedly treated as binary")
        changes = int(a) + int(d)
        if changes > 180:
            raise RuntimeError(f"{name}: source diff too large ({changes} changed lines)")
    for p in V18 + [ANIMAL]:
        before = git_show_bytes(p)
        after = p.read_bytes()
        if b"\r\n" in before and after.count(b"\r\n") < max(1, before.count(b"\r\n") - 200):
            raise RuntimeError(f"{p}: CRLF preservation check failed")

build_v18(V18[0])
build_v18(V18[1])
build_v18(V18[2])
build_animal()

for zpath, ino in ZIP_MAP.items():
    rebuild_zip(zpath, ino)

update_manifest()
validate_diff_size()

def base_lid(ambient):
    if ambient <= 75:
        return 1.0
    t = min(1.0, (ambient - 75.0) / 25.0)
    return 1.0 - 0.38 * t

print("--- LIGHT LID SANITY ---")
for a in (0, 50, 75, 80, 88, 95, 96, 100):
    print(f"ambient={a:3d}% base_lid={base_lid(a):.3f}")
if not (base_lid(75) == 1.0 and 0.61 < base_lid(100) < 0.63):
    raise RuntimeError("light lid arithmetic sanity failed")

print("ALL FOUR SOURCES + ZIPS + DIFF VALIDATION PASSED")
