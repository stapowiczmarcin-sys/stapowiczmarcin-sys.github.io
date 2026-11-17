"""KORA Pi5 + Hailo-10H enclosure V3: rear LED ring, five-part, three-colour 3MF.
Source dimensions provided by the owner; mounting/fan/LED positions require physical fitting.
"""
from pathlib import Path
import io
import math
import zipfile
import numpy as np
import cadquery as cq
import trimesh
from lxml import etree

OUT = Path(__file__).resolve().parent
L, W, H = 117.9, 71.0, 98.0
PLATEAU, OPEN_FRONT, SLOPE_DROP = 74.0, 33.0, 16.0
X_BREAK = L - OPEN_FRONT
WALL, FLOOR, WALL_TOP = 2.8, 3.2, 95.0
FAN_X, FAN_Y = 37.0, 35.5
LED_Y, LED_Z, LED_BOLT_RAD = 35.5, 60.0, 28.5

def box(x,y,z,l,w,h):
    return cq.Workplane('XY').box(l,w,h,centered=(False,False,False)).translate((x,y,z))

def profile(points,y0,width):
    return cq.Workplane('XZ').polyline(points).close().extrude(width).translate((0,y0+width,0))

def hole_z(x,y,z,r,length):
    return cq.Workplane('XY').center(x,y).circle(r).extrude(length).translate((0,0,z))

def hole_x(x,y,z,r,length):
    return cq.Workplane(obj=cq.Solid.makeCylinder(r,length,cq.Vector(x,y,z),cq.Vector(1,0,0)))

# Silver shell: 117.9 x 71 x 95; white cap brings overall case to 98.
body = box(0,0,0,L,W,FLOOR)
side = [(0,0),(L,0),(L,19),(X_BREAK,19),(X_BREAK,WALL_TOP-SLOPE_DROP),
        (PLATEAU,WALL_TOP),(0,WALL_TOP)]
body = body.union(profile(side,0,WALL)).union(profile(side,W-WALL,WALL))
body = body.union(box(0,0,0,WALL,W,WALL_TOP))
# Rear ventilation slots are below the LED recess (not hidden by the light ring).
for y in (13.,31.,49.):
    body=body.cut(box(-.5,y,19,4.,10.,4.3))
for x in (14.,30.,46.,62.):
    for z in (15.,23.):
        body=body.cut(box(x,-.5,z,10.,3.8,4.))
        body=body.cut(box(x,W-WALL-.5,z,10.,4.1,4.))
for x in (23.,42.,61.):
    for y in (28.,44.):
        body=body.cut(box(x,y,-.5,11.,3.,FLOOR+1))
# Pi5 mounting: 58 x 49 mm hole-centre rectangle, M2.5 clearance.
mounts=[(13.,11.),(71.,11.),(13.,60.),(71.,60.)]
for x,y in mounts:
    body=body.union(hole_z(x,y,FLOOR,5.1,7.4))
    body=body.cut(hole_z(x,y,-.2,1.4,11.2))
    body=body.cut(hole_z(x,y,-.2,2.6,1.8))
lid_screws=[(7,6),(67,6),(7,65),(67,65)]
for x,y in lid_screws:
    body=body.union(box(x-4,.8 if y<30 else W-9,86.4,8,8.2,8.6))
    body=body.cut(hole_z(x,y,86.2,1.15,10.))
# Rear-facing LED ring on the short end wall (x=0), centered across the 71 mm width.
# PCB diameter 47 mm, optical diffuser wall 0.6 mm = three 0.2 mm print layers.
# The red retainer mounts on the back; its outer diameter 62.8 mm fits in 71 mm.
body=body.cut(hole_x(-1,LED_Y,LED_Z,24.1,WALL+2))
body=body.cut(hole_x(-.05,LED_Y,LED_Z,26.2,.70))
for y,z in [(LED_Y+LED_BOLT_RAD,LED_Z),(LED_Y-LED_BOLT_RAD,LED_Z),
            (LED_Y,LED_Z+LED_BOLT_RAD),(LED_Y,LED_Z-LED_BOLT_RAD)]:
    body=body.cut(hole_x(-.5,y,z,1.42,WALL+1))
# Geometry guards: physical LED board and screw positions must fit rear panel.
assert LED_Y-31.4>0 and LED_Y+31.4<W
assert LED_Z-31.4>0 and LED_Z+31.4<WALL_TOP
assert 23.3 < LED_Z - 24.1, 'Rear vents must not overlap the LED aperture.'

# White roof, flat rear 74 mm, 16 mm descent until x=84.9, no roof for last 33 mm.
roof=profile([(0,95),(PLATEAU,95),(X_BREAK,79),(X_BREAK,82),(PLATEAU,98),(0,98)],0,W)
clearance=[(0,0),(X_BREAK,0),(X_BREAK,79.35),(PLATEAU,95.35),(0,95.35)]
roof=roof.cut(profile(clearance,-.01,WALL+.35))
roof=roof.cut(profile(clearance,W-WALL-.34,WALL+.35))
roof=roof.cut(hole_z(FAN_X,FAN_Y,93,11.,7))
# Four approximate fixing slots; check real 26 x 26 mm fan screw pitch!
for sx in (-1,1):
    for sy in (-1,1):
        slit=(cq.Workplane('XY').center(FAN_X+sx*10,FAN_Y+sy*10)
              .slot2D(4.2,2.4,0).extrude(7).translate((0,0,93)))
        roof=roof.cut(slit)
for x,y in lid_screws:
    roof=roof.cut(hole_z(x,y,93,1.65,7))
    roof=roof.cut(hole_z(x,y,97,2.5,2))
for x in (12.,21.,53.,62.):
    for y in (23.,38.,53.):
        if abs(x-FAN_X)<21 and abs(y-FAN_Y)<21:
            continue
        roof=roof.cut(box(x,y,94,3.2,7.5,5.5))

# Red badge with embossed KORA lettering; separate object for multi-colour slicing.
logo=box(0,0,0,44,12,1)
try:
    t=cq.Workplane('XY').text('KORA',7.5,.75,font='DejaVu Sans',kind='bold',combine=True)
    bb=t.val().BoundingBox()
    t=t.translate((22-(bb.xmin+bb.xmax)/2,6-(bb.ymin+bb.ymax)/2,.95))
    logo=logo.union(t)
except Exception as ex:
    print('Logo font fallback:',ex)

# White diffuser. Outer optical disk 0.6mm thick = 3 layers at 0.2mm.
diffuser=hole_z(0,0,0,26,.6)
diffuser=diffuser.union(hole_z(0,0,.6,23.85,1.2).cut(hole_z(0,0,.5,22.8,1.6)))
retainer=hole_z(0,0,0,31.4,2.4).cut(hole_z(0,0,-.2,21.5,3))
for x,y in [(LED_BOLT_RAD,0),(-LED_BOLT_RAD,0),(0,LED_BOLT_RAD),(0,-LED_BOLT_RAD)]:
    retainer=retainer.cut(hole_z(x,y,-.2,1.08,3))

overlap=body.val().intersect(roof.val()).Volume()
print('Roof/body overlap mm3:', overlap)
assert overlap<.05, 'Case body and roof interfere.'

parts={
    '01_SILVER_body':body,
    '02_WHITE_top_and_slope':roof,
    '03_WHITE_diffuser_0p6mm':diffuser,
    '04_RED_KORA_logo':logo,
    '05_RED_LED_retainer':retainer
}
def meshify(part):
    vertices,faces=part.val().tessellate(.18,.11)
    mesh=trimesh.Trimesh(
        vertices=np.array([[v.x,v.y,v.z] for v in vertices]),
        faces=np.array(faces),process=True)
    mesh.fix_normals()
    assert mesh.is_watertight and mesh.volume>0
    return mesh

meshes={name:meshify(part) for name,part in parts.items()}
scene=trimesh.Scene()
A=np.eye(4);A[:3,3]=[5,5,0]
scene.add_geometry(meshes['01_SILVER_body'],geom_name='01_SILVER_body',transform=A)
B=trimesh.transformations.rotation_matrix(math.pi,[0,1,0])
B[:3,3]=[130+X_BREAK,5,H]
scene.add_geometry(meshes['02_WHITE_top_and_slope'],geom_name='02_WHITE_top_and_slope',transform=B)
for name,pos in [
    ('03_WHITE_diffuser_0p6mm',(154,112,0)),
    ('04_RED_KORA_logo',(134,164,0)),
    ('05_RED_LED_retainer',(215,169,0))
]:
    M=np.eye(4);M[:3,3]=pos
    scene.add_geometry(meshes[name],geom_name=name,transform=M)

raw=trimesh.exchange.threemf.export_3MF(scene)
NS='http://schemas.microsoft.com/3dmanufacturing/core/2015/02'
with zipfile.ZipFile(io.BytesIO(raw)) as z:
    members={name:z.read(name) for name in z.namelist()}
xml=etree.fromstring(members['3D/3dmodel.model'])
resources=xml.find('{%s}resources'%NS)
materials=etree.Element('{%s}basematerials'%NS,id='9000')
for color,code in [('SILVER','#B2B5B9FF'),('WHITE','#F5F3EEFF'),('RED','#D72E35FF')]:
    etree.SubElement(materials,'{%s}base'%NS,name=color,displaycolor=code)
resources.insert(0,materials)
for obj in resources.findall('{%s}object'%NS):
    name=obj.get('name','')
    obj.set('pid','9000')
    obj.set('pindex',str(0 if 'SILVER' in name else 1 if 'WHITE' in name else 2))
members['3D/3dmodel.model']=etree.tostring(xml,encoding='utf-8',xml_declaration=True)
target=OUT/'KORA_Pi5_Hailo10H_V3_REAR_LED.3mf'
with zipfile.ZipFile(target,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for name,data in members.items():
        z.writestr(name,data)

reloaded=trimesh.load(str(target),force='scene')
assert len(reloaded.geometry)==5
assert reloaded.bounds[0][0]>=0 and reloaded.bounds[0][1]>=0
assert reloaded.bounds[1][0]<=256 and reloaded.bounds[1][1]<=256
print('SUCCESS',target, target.stat().st_size, 'bytes')
