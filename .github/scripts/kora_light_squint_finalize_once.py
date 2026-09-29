from pathlib import Path
import hashlib

FILES = [
    Path("eyes/KoraHumanMetalV18_S3_PUBLIC/KoraHumanMetalV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMetalEyesV18_S3_PUBLIC/KoraMetalEyesV18_S3_PUBLIC.ino"),
    Path("eyes/KoraMonsterEyesV18_S3_PUBLIC/KoraMonsterEyesV18_S3_PUBLIC.ino"),
]
TOKENS = [
    b"static constexpr int PIN_LIGHT_L=3, PIN_LIGHT_R=4;",
    b"static constexpr int PIN_LIGHT_L=-1, PIN_LIGHT_R=-1;",
    b"Mode mode=AUTO",
    b"Mode mode=ANIMATED",
    b'upper.startsWith("LOOK ")',
    b"faceTarget.lookX",
    b"faceTarget.lookY",
    b"analogRead(PIN_LIGHT_L)",
    b"analogRead(PIN_LIGHT_R)",
    b"lightSquint",
    b"lightLidResponse",
]
for p in FILES:
    data = p.read_bytes()
    norm = data.replace(b"\r\n", b"\n")
    print("===", p, "===")
    print("bytes", len(data), "normalized_sha256", hashlib.sha256(norm).hexdigest(), "CRLF", data.count(b"\r\n"), "LF", data.count(b"\n"))
    for t in TOKENS:
        print(t.decode("utf-8", "replace"), "count=", data.count(t))
    text = norm.decode("utf-8", "replace").splitlines()
    needles = ("PIN_LIGHT", "Mode mode=", "LOOK ", "analogRead(PIN_LIGHT", "lightSquint", "lightLidResponse", "control.tick(now,fresh", "oL *= (1.0f - 0.80f * sqL)")
    for i,line in enumerate(text,1):
        if any(n in line for n in needles):
            print(f"L{i}: {line[:240]}")
print("AUDIT COMPLETE")
