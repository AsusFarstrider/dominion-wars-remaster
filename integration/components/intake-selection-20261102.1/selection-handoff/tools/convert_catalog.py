"""Convert private DWSSEL1 to DWSSEL2 without retaining reference image bytes.

Only standard-library code. The legacy input is local owner material, never an
output/dependency for the runtime package. Replacement payloads are byte-copied.
"""
import argparse,hashlib,json,struct
from pathlib import Path

HEADER=struct.Struct('<IIhhHHII')
def convert(data):
    if not 12<=len(data)<=32*1024*1024 or data[:8]!=b'DWSSEL1\0':raise ValueError('Unsupported legacy catalog')
    count,=struct.unpack_from('<I',data,8)
    if count!=114:raise ValueError('Record count')
    output=bytearray(b'DWSSEL2\0'+struct.pack('<I',count));pos=12;records=[];keys=set()
    for _ in range(count):
        if pos+HEADER.size>len(data):raise ValueError('Truncated descriptor')
        descriptor=data[pos:pos+HEADER.size];kind,key,ox,oy,w,h,pw,ph=HEADER.unpack(descriptor);pos+=HEADER.size
        if (kind,key) in keys or kind not in (1,2,3,4) or not (0<w<=800 and 0<h<=600):raise ValueError('Descriptor')
        keys.add((kind,key));n=w*h
        if pos+n>len(data):raise ValueError('Truncated original identity')
        original=data[pos:pos+n];pos+=n
        digest=hashlib.sha256(original).digest();classes=bytearray(32)
        if kind==2:
            for index in set(original)-{0}:classes[index//8]|=1<<(index%8)
        payload_size=0 if kind==3 else pw*ph*(2 if kind==2 else 4)
        if pw>3200 or ph>2400 or pos+payload_size>len(data):raise ValueError('Replacement payload')
        payload=data[pos:pos+payload_size];pos+=payload_size
        if kind==2 and any(alpha and (index==0 or not classes[index//8]&(1<<(index%8))) for index,alpha in zip(payload[::2],payload[1::2])):raise ValueError('Glyph class')
        output+=descriptor+digest+classes+payload
        records.append(dict(kind=kind,key=key,logical_origin=[ox,oy],logical_size=[w,h],replacement_size=[pw,ph],
            identity_bytes=n,identity_sha256=digest.hex(),nonzero_glyph_classes=[i for i in range(1,256) if classes[i//8]&(1<<(i%8))],
            replacement_bytes=payload_size,replacement_sha256=hashlib.sha256(payload).hexdigest()))
    if pos!=len(data):raise ValueError('Trailing bytes')
    return bytes(output),dict(schema='selection-reference-free-conversion-1',input_sha256=hashlib.sha256(data).hexdigest(),
        output_sha256=hashlib.sha256(output).hexdigest(),original_reference_pixels_written=False,
        source_bytes=len(data),output_bytes=len(output),reference_bytes_removed=sum(r['identity_bytes'] for r in records),
        identity_bytes_added=count*64,records=records)

def main():
    p=argparse.ArgumentParser();p.add_argument('--legacy-catalog',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    if args.output.exists():raise ValueError('Use a new output directory')
    out,receipt=convert(args.legacy_catalog.read_bytes());args.output.mkdir(parents=True)
    (args.output/'selection.dwselect').write_bytes(out)
    receipt['converter_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    (args.output/'conversion.json').write_text(json.dumps(receipt,indent=2,sort_keys=True)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in receipt.items() if k!='records'},indent=2))
if __name__=='__main__':main()
