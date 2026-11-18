"""Dependency-free validation of reference-free DWSSEL2 asset contracts."""
import argparse,hashlib,json,struct
from pathlib import Path
DESCRIPTOR=struct.Struct('<IIhhHHII')
def validate(data):
    if not 12<=len(data)<=32*1024*1024 or data[:8]!=b'DWSSEL2\0':raise ValueError('magic/size')
    count,=struct.unpack_from('<I',data,8)
    if count!=114:raise ValueError('count')
    pos=12;keys=set();records=[]
    for _ in range(count):
        start=pos
        if pos+88>len(data):raise ValueError('truncated record header')
        kind,key,ox,oy,w,h,pw,ph=DESCRIPTOR.unpack_from(data,pos);pos+=24
        digest=data[pos:pos+32];classes=data[pos+32:pos+64];pos+=64
        if (kind,key) in keys or not (0<w<=800 and 0<h<=600) or not any(digest) or classes[0]&1:raise ValueError('identity/geometry')
        keys.add((kind,key))
        if kind==3:
            if (key,ox,oy,w,h,pw,ph)!=(0x801,0,0,800,600,800,600):raise ValueError('map shape')
        elif (pw,ph)!=(w*4,h*4):raise ValueError('density')
        if kind==1:
            width=570 if key>=0x802 else 140 if key<0x15 else 180
            if key not in [0x802,0x803,0x804,*range(0x12,0x18)] or (ox,oy,h,w)!=(0,0,32,width):raise ValueError('body shape')
        elif kind==2:
            if not (32<=key<=127 and w<=17 and h<=19 and 0<=ox<=17 and 0<=oy<=24):raise ValueError('glyph shape')
        elif kind==4:
            widths=[82,82,101,101,153,153,95,95]
            if not 0x809<=key<=0x810 or (ox,oy,w,h)!=(0,0,widths[key-0x809],21 if key<0x80B else 22):raise ValueError('faction shape')
        elif kind!=3:raise ValueError('kind')
        if kind!=2 and any(classes):raise ValueError('unexpected classes')
        size=0 if kind==3 else pw*ph*(2 if kind==2 else 4)
        if pos+size>len(data):raise ValueError('truncated payload')
        payload=memoryview(data)[pos:pos+size]
        if kind==2 and any(alpha and (index==0 or not classes[index//8]&(1<<(index%8))) for index,alpha in zip(payload[::2],payload[1::2])):raise ValueError('covered glyph index outside original class set')
        records.append(dict(offset=start,kind=kind,key=key,logical_origin=[ox,oy],logical_size=[w,h],replacement_size=[pw,ph],
            identity_sha256=digest.hex(),glyph_classes_hex=classes.hex(),payload_offset=pos,payload_bytes=size,payload_sha256=hashlib.sha256(payload).hexdigest()))
        pos+=size
    if pos!=len(data):raise ValueError('trailing data')
    return dict(format='DWSSEL2',record_count=count,bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),
        contains_original_identity_pixel_field=False,records=records)
def main():
    p=argparse.ArgumentParser();p.add_argument('catalog',type=Path);p.add_argument('--receipt',type=Path);args=p.parse_args()
    r=validate(args.catalog.read_bytes())
    if args.receipt:
        if args.receipt.exists():raise ValueError('Use a new receipt path')
        args.receipt.write_text(json.dumps(r,indent=2,sort_keys=True)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in r.items() if k!='records'},indent=2))
if __name__=='__main__':main()
