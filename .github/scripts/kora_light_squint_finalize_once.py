from pathlib import Path, PurePosixPath
from textwrap import dedent
import subprocess, io, zipfile, hashlib, json

BASE = "30db11179c660ea46acd371936a95bb2afc1a1d3"
V18 = [
    Path("eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino"),
]
ZIP_MAP = {
    Path("eyes/KoraHumanMetalV18_S3_PUBLIC.zip"): V18[0],
    Path("eyes/KoraMetalEyesV18_S3_PUBLIC.zip"): V18[1],
    Path("eyes/KoraMonsterEyesV18_S3_PUBLIC.zip"): V18[2],
}
MANIFEST = Path("eyes/SHA256_V18.json")
EXPECTED_NORMALIZED_SHA256 = {
    V18[0]: "096081f9a0bba6171838cbcd3021d87ebca45d25c3e59cecc139d306113502b2",
    V18[1]: "fb9c982241c17d4c0b1deea8a32acd47cbb6460e2f2420e2bfd7c9ba8f85e2b7",
    V18[2]: "b3da9f81ec8a56b4de740dcd9dc59a479e1537c694799b7346c95ca6cc80ffa5",
}

def git_show_bytes(path: Path) -> bytes:
    return subprocess.check_output(["git", "show", f"{BASE}:{path.as_posix()}"])

def nl_for(data: bytes) -> bytes:
    return b"\r\n" if b"\r\n" in data else b"\n"

def block_bytes(text: str, nl: bytes) -> bytes:
    text = dedent(text).strip("\n")
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

LIGHT_STATE = r'''// Bright-light reflex: above 65% ambient the lids progressively squint.
// 65-80% = slight squint, 80-92% = clear squint, 92-100% = nearly closed.
static float lightSquint=0.0f;
static uint32_t lightSquintAt=0;
static float lightSquintTarget(float pct){
  pct=clampF(pct,0.0f,100.0f);
  if(pct<=65.0f)return 0.0f;
  if(pct<=80.0f)return 0.25f*(pct-65.0f)/15.0f;
  if(pct<=92.0f)return 0.25f+0.35f*(pct-80.0f)/12.0f;
  return 0.60f+0.30f*(pct-92.0f)/8.0f;
}'''

LIGHT_UPDATE = r'''  // Natural photophobia reflex. Close faster in glare, reopen more gently.
  float target=fresh?lightSquintTarget(KoraPupil::control.filtered):0.0f;
  float dt=lightSquintAt?clampF(float(uint32_t(now-lightSquintAt))*.001f,0.0f,0.10f):0.0f;
  lightSquintAt=now;
  float tau=(target>lightSquint)?0.16f:0.70f;
  if(dt>0.0f)lightSquint+=(target-lightSquint)*(dt/(tau+dt));
  else lightSquint=target;'''

LIGHT_RENDER = r'''  // Ambient-light squint is independent from the FSR squash reflex.
  // At maximum glare only ~10% eyelid opening remains.
  if(!quietMode && crtState != 2){
    oL *= (1.0f - lightSquint);
    oR *= (1.0f - lightSquint);
  }'''

LOOK_BLOCK = r'''  if (upper.startsWith("LOOK ")) {
    String arg = line.substring(5);
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
    faceTarget.lookX = x;
    faceTarget.lookY = y;
    Serial.println("OK LOOK " + String(x, 2) + " " + String(y, 2));
    return;
  }'''

def build_v18(path: Path):
    data = git_show_bytes(path)
    nl = nl_for(data)

    data = replace_once(data, b"static constexpr int PIN_LIGHT_L=-1, PIN_LIGHT_R=-1;", b"static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;", f"{path}: light pins")
    data = replace_once(data, b"// Disabled until the actual sensor GPIOs and divider calibration are supplied.", b"// Ambient-light sensors: GPIO3 left, GPIO4 right (ADC1).", f"{path}: light comment")

    animated = b"  Mode mode=ANIMATED;"
    auto = b"  Mode mode=AUTO; // light sensors control pupil size by default"
    if animated in data:
        data = replace_once(data, animated, auto, f"{path}: default pupil mode")
    elif auto not in data:
        raise RuntimeError(f"{path}: pupil mode anchor not found")

    source_anchor = b'static const char* lightSource="none";'
    data = replace_once(data, source_anchor, source_anchor + nl + block_bytes(LIGHT_STATE, nl), f"{path}: light squint state")
    tick_anchor = b"  KoraPupil::control.tick(now,fresh,remoteFresh?lightRemote:lightLocal);"
    data = replace_once(data, tick_anchor, tick_anchor + nl + nl + block_bytes(LIGHT_UPDATE, nl), f"{path}: light squint update")
    render_anchor = b"  oL *= (1.0f - 0.80f * sqL);" + nl + b"  oR *= (1.0f - 0.80f * sqR);" + nl + nl + b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    render_new = b"  oL *= (1.0f - 0.80f * sqL);" + nl + b"  oR *= (1.0f - 0.80f * sqR);" + nl + nl + block_bytes(LIGHT_RENDER, nl) + nl + nl + b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    data = replace_once(data, render_anchor, render_new, f"{path}: render squint")

    if b'upper.startsWith("LOOK ")' not in data:
        fsr_anchor = b'  if (upper == "FSRSTATUS") {'
        assert_once(data, fsr_anchor, f"{path}: FSRSTATUS")
        start = data.index(fsr_anchor)
        marker = nl + b"  lastInteractionMs = millis();"
        pos = data.find(marker, start)
        if pos < 0:
            raise RuntimeError(f"{path}: LOOK insertion point not found")
        data = data[:pos] + nl + block_bytes(LOOK_BLOCK, nl) + nl + data[pos:]

    for needle, label in [
        (b"static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;", "GPIO3/4"),
        (b"Mode mode=AUTO; // light sensors control pupil size by default", "AUTO pupil"),
        (b"analogRead(PIN_LIGHT_L)", "left ADC"),
        (b"analogRead(PIN_LIGHT_R)", "right ADC"),
        (b"static float lightSquintTarget(float pct){", "squint curve"),
        (b"oL *= (1.0f - lightSquint);", "left lid squint"),
        (b"oR *= (1.0f - lightSquint);", "right lid squint"),
        (b'if (upper.startsWith("LOOK ")) {', "LOOK command"),
        (b"faceTarget.lookX = x;", "LOOK X"),
        (b"faceTarget.lookY = y;", "LOOK Y"),
    ]:
        assert_once(data, needle, f"{path}: {label}")

    normalized = data.replace(b"\r\n", b"\n")
    digest = hashlib.sha256(normalized).hexdigest()
    expected = EXPECTED_NORMALIZED_SHA256[path]
    if digest != expected:
        raise RuntimeError(f"{path}: generated source differs from locally audited file: {digest} != {expected}")

    path.write_bytes(data)
    print("SOURCE OK:", path, digest)

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
        if check.testzip() is not None:
            raise RuntimeError(f"{zpath}: ZIP CRC failure")
        matches = [check.read(n) for n in check.namelist() if PurePosixPath(n).name == ino.name]
        if len(matches) != 1 or matches[0] != ino.read_bytes():
            raise RuntimeError(f"{zpath}: embedded INO does not match source")
    print("ZIP OK:", zpath)

def update_manifest():
    manifest = json.loads(git_show_bytes(MANIFEST).decode("utf-8"))
    for zpath in ZIP_MAP:
        digest = hashlib.sha256(zpath.read_bytes()).hexdigest()
        manifest[zpath.name] = digest
        print("SHA256", zpath.name, digest)
    MANIFEST.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")

def validate_diff_size():
    cmd = ["git", "diff", "--numstat", BASE, "--"] + [p.as_posix() for p in V18]
    out = subprocess.check_output(cmd, text=True)
    print("--- SOURCE NUMSTAT VS CLEAN BASE ---")
    print(out, end="")
    for line in out.splitlines():
        a, d, name = line.split("\t", 2)
        if a == "-" or d == "-":
            raise RuntimeError(f"{name}: source unexpectedly treated as binary")
        if int(a) + int(d) > 160:
            raise RuntimeError(f"{name}: source diff too large ({int(a)+int(d)} changed lines)")

for p in V18:
    build_v18(p)
for z, ino in ZIP_MAP.items():
    rebuild_zip(z, ino)
update_manifest()
validate_diff_size()

print("--- LIGHT SQUINT SANITY ---")
def target(p):
    if p <= 65: return 0.0
    if p <= 80: return 0.25*(p-65)/15
    if p <= 92: return 0.25+0.35*(p-80)/12
    return 0.60+0.30*(p-92)/8
for p in (0, 50, 65, 75, 80, 86, 92, 96, 100):
    q=target(p)
    print(f"light={p:3d}% squint={q:.3f} opening={1-q:.3f}")
if not (target(65)==0.0 and target(80)==0.25 and target(92)==0.60 and abs(target(100)-0.90)<1e-9):
    raise RuntimeError("light squint curve sanity failed")
print("ALL CHECKS PASSED")
