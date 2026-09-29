from pathlib import Path
import hashlib

p = Path('eyes/KoraAnimalEyesV14_S3_PUBLIC/KoraAnimalEyesV14_S3_PUBLIC.ino')
data = p.read_bytes()
text = data.decode('utf-8')
lines = text.splitlines()
print('ANIMAL bytes', len(data), 'lines', len(lines), 'sha256', hashlib.sha256(data).hexdigest())

needles = [
    'PIN_LIGHT_L', 'PIN_LIGHT_R', 'updateLightPupil', 'lightPupilOffset',
    'pupilL', 'pupilR', 'sqL', 'sqR',
    'FSRSTATUS', 'lastInteractionMs', 'processSerial', 'handleCommand',
    'LOOK ', 'IRIS ', 'void loop()', 'void setup()'
]

seen = set()
for needle in needles:
    hits = [i for i,l in enumerate(lines) if needle in l]
    print('\n===', needle, 'hits', len(hits), '===')
    for i in hits[:12]:
        a=max(0,i-8); b=min(len(lines),i+12)
        key=(a,b)
        if key in seen: continue
        seen.add(key)
        for j in range(a,b):
            print(f'{j+1:05d}: {lines[j]}')
        print('---')
print('AUDIT COMPLETE')
