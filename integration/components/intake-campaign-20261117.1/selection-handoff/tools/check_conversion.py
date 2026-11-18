import hashlib,json,struct
from pathlib import Path
from convert_catalog import convert
from validate_catalog import validate
BASE=Path('E:/Codex/research/dominion-rendering')
old=(BASE/'mission-select-assets-20261101-04/selection.dwselect').read_bytes()
new=(BASE/'selection-reference-free-assets-20261102-01/selection.dwselect').read_bytes()
again,receipt=convert(old);assert again==new
r=validate(new);assert len(r['records'])==114
count=3;pos=12
for entry in r['records']:
    header=old[pos:pos+24];kind,key,ox,oy,w,h,pw,ph=struct.unpack('<IIhhHHII',header);pos+=24
    reference=old[pos:pos+w*h];pos+=w*h
    assert entry['identity_sha256']==hashlib.sha256(reference).hexdigest()
    size=entry['payload_bytes'];old_payload=old[pos:pos+size];pos+=size
    assert new[entry['offset']:entry['offset']+24]==header
    assert new[entry['payload_offset']:entry['payload_offset']+size]==old_payload
    count+=3
assert pos==len(old);count+=1
def rejected(data):
    global count
    try:validate(data)
    except (ValueError,struct.error):count+=1;return
    raise AssertionError('malformed catalog accepted')
rejected(b'');rejected(old);rejected(new+b'\0')
for cut in [8,11,12,35,36,67,68,99,len(new)-1]:rejected(new[:cut])
for address,value in [(8,0),(12,99),(36,0)]:
    b=bytearray(new)
    if address==36:b[36:68]=bytes(32)
    else:b[address]=value
    rejected(b)
b=bytearray(new);b[68]=1;rejected(b)
b=bytearray(new);b[69]=2;rejected(b)
glyph=next(e for e in r['records'] if e['kind']==2 and e['payload_bytes']>32 and any(bytes.fromhex(e['glyph_classes_hex'])))
b=bytearray(new);b[glyph['offset']+56:glyph['offset']+88]=bytes(32);rejected(b)
b=bytearray(new);b[glyph['payload_offset']]=0;b[glyph['payload_offset']+1]=255;rejected(b)
# Malformed legacy conversion never emits an asset directory or image bytes.
for bad in [b'',old[:12],old[:-1],old+b'\0']:
    try:convert(bad)
    except (ValueError,struct.error):count+=1
    else:raise AssertionError('malformed legacy accepted')
out=BASE/'selection-reference-free-candidate-20261102-01/conversion-validation.json'
assert not out.exists()
out.write_text(json.dumps(dict(assertions=count,format=r['format'],input_sha256=hashlib.sha256(old).hexdigest(),
    output_sha256=r['sha256'],records=114,replacement_payloads_byte_equal=True,reference_pixel_fields_removed=True,
    reference_bytes_removed=receipt['reference_bytes_removed'],runtime_artwork_rights_reviewed=False),indent=2)+'\n',encoding='utf-8')
print(f'PASS {count} deterministic conversion/structural identity/payload/malformed checks')
