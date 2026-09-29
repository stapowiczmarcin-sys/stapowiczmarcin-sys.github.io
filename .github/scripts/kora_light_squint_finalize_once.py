from pathlib import Path, PurePosixPath
from textwrap import dedent
import io, zipfile, hashlib, json

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

def nl_for(data: bytes) -> bytes:
    return b"\r\n" if b"\r\n" in data else b"\n"

def block_bytes(text: str, nl: bytes) -> bytes:
    return dedent(text).strip("\n").encode("utf-8").replace(b"\n", nl)

def replace_once(data: bytes, old: bytes, new: bytes, label: str) -> bytes:
    n=data.count(old)
    if n!=1: raise RuntimeError(f"{label}: expected exactly one anchor, found {n}")
    return data.replace(old,new,1)

def assert_once(data: bytes, needle: bytes, label: str):
    n=data.count(needle)
    if n!=1: raise RuntimeError(f"{label}: expected once, found {n}")

def patch_source(path: Path):
    before=path.read_bytes()
    data=before
    nl=nl_for(data)

    # Current main was audited first. Refuse to patch any unexpected state.
    assert_once(data,b"static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;",f"{path}: GPIO3/4")
    if b"PIN_LIGHT_L=-1" in data or b"PIN_LIGHT_R=-1" in data:
        raise RuntimeError(f"{path}: disabled light pins still present")
    assert_once(data,b"analogRead(PIN_LIGHT_L)",f"{path}: left ADC")
    assert_once(data,b"analogRead(PIN_LIGHT_R)",f"{path}: right ADC")
    if b"lightSquint" in data or b"lightLidResponse" in data:
        raise RuntimeError(f"{path}: a light-lid patch already exists; refusing duplicate")
    if b'upper.startsWith("LOOK ")' in data:
        raise RuntimeError(f"{path}: LOOK already exists; refusing duplicate")

    data=replace_once(data,b"  Mode mode=ANIMATED;",b"  Mode mode=AUTO; // light sensors control pupil size by default",f"{path}: pupil AUTO")

    source_anchor=b'static const char* lightSource="none";'
    data=replace_once(data,source_anchor,source_anchor+nl+block_bytes(LIGHT_STATE,nl),f"{path}: squint state")

    tick_anchor=b"  KoraPupil::control.tick(now,fresh,remoteFresh?lightRemote:lightLocal);"
    data=replace_once(data,tick_anchor,tick_anchor+nl+nl+block_bytes(LIGHT_UPDATE,nl),f"{path}: squint update")

    render_anchor=b"  oL *= (1.0f - 0.80f * sqL);"+nl+b"  oR *= (1.0f - 0.80f * sqR);"+nl+nl+b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    render_new=b"  oL *= (1.0f - 0.80f * sqL);"+nl+b"  oR *= (1.0f - 0.80f * sqR);"+nl+nl+block_bytes(LIGHT_RENDER,nl)+nl+nl+b"  oL *= displayPower.lid; oR *= displayPower.lid;"
    data=replace_once(data,render_anchor,render_new,f"{path}: lid render")

    fsr_anchor=b'  if (upper == "FSRSTATUS") {'
    assert_once(data,fsr_anchor,f"{path}: FSRSTATUS")
    start=data.index(fsr_anchor)
    marker=nl+b"  lastInteractionMs = millis();"
    pos=data.find(marker,start)
    if pos<0: raise RuntimeError(f"{path}: LOOK insertion point not found")
    data=data[:pos]+nl+block_bytes(LOOK_BLOCK,nl)+nl+data[pos:]

    for needle,label in [
        (b"Mode mode=AUTO; // light sensors control pupil size by default","AUTO mode"),
        (b"static float lightSquintTarget(float pct){","squint curve"),
        (b"float tau=(target>lightSquint)?0.16f:0.70f;","squint smoothing"),
        (b"oL *= (1.0f - lightSquint);","left lid"),
        (b"oR *= (1.0f - lightSquint);","right lid"),
        (b'if (upper.startsWith("LOOK ")) {',"LOOK parser"),
        (b"faceTarget.lookX = x;","LOOK X"),
        (b"faceTarget.lookY = y;","LOOK Y"),
    ]: assert_once(data,needle,f"{path}: {label}")

    # Byte-preservation guard: insertions are balanced and line-ending style cannot change.
    if nl_for(data)!=nl_for(before): raise RuntimeError(f"{path}: line ending style changed")
    for a,b,name in [(b"{",b"}","braces"),(b"(",b")","parentheses"),(b"[",b"]","brackets")]:
        if (data.count(a)-data.count(b)) != (before.count(a)-before.count(b)):
            raise RuntimeError(f"{path}: {name} balance changed")
    delta=len(data)-len(before)
    if not (1200 < delta < 5000): raise RuntimeError(f"{path}: unexpected byte delta {delta}")

    path.write_bytes(data)
    print("SOURCE PATCHED:",path,"before",hashlib.sha256(before).hexdigest(),"after",hashlib.sha256(data).hexdigest(),"delta",delta)

def rebuild_zip(zpath: Path, ino: Path):
    original=zpath.read_bytes()
    src_bio=io.BytesIO(original); out_bio=io.BytesIO(); replaced=0
    with zipfile.ZipFile(src_bio,"r") as src, zipfile.ZipFile(out_bio,"w") as dst:
        for info in src.infolist():
            payload=src.read(info.filename)
            if PurePosixPath(info.filename).name==ino.name:
                payload=ino.read_bytes(); replaced+=1
            dst.writestr(info,payload)
    if replaced!=1: raise RuntimeError(f"{zpath}: expected one INO replacement, got {replaced}")
    zpath.write_bytes(out_bio.getvalue())
    with zipfile.ZipFile(zpath,"r") as check:
        bad=check.testzip()
        if bad is not None: raise RuntimeError(f"{zpath}: CRC failure in {bad}")
        matches=[check.read(n) for n in check.namelist() if PurePosixPath(n).name==ino.name]
        if len(matches)!=1 or matches[0]!=ino.read_bytes(): raise RuntimeError(f"{zpath}: embedded INO mismatch")
    print("ZIP OK:",zpath,hashlib.sha256(zpath.read_bytes()).hexdigest())

def update_manifest():
    manifest=json.loads(MANIFEST.read_text(encoding="utf-8-sig"))
    for zpath in ZIP_MAP:
        manifest[zpath.name]=hashlib.sha256(zpath.read_bytes()).hexdigest()
    MANIFEST.write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+"\n",encoding="utf-8",newline="\n")
    print("MANIFEST OK")

for p in V18: patch_source(p)
for z,p in ZIP_MAP.items(): rebuild_zip(z,p)
update_manifest()

print("--- SQUINT CURVE ---")
def target(p):
    if p<=65:return 0.0
    if p<=80:return 0.25*(p-65)/15
    if p<=92:return 0.25+0.35*(p-80)/12
    return 0.60+0.30*(p-92)/8
for p in (0,50,65,75,80,86,92,96,100):
    q=target(p); print(f"light={p:3d}% squint={q:.3f} opening={1-q:.3f}")
if not(target(65)==0 and target(80)==0.25 and target(92)==0.60 and abs(target(100)-0.90)<1e-9):
    raise RuntimeError("squint curve failed")
print("ALL PATCH CHECKS PASSED")
