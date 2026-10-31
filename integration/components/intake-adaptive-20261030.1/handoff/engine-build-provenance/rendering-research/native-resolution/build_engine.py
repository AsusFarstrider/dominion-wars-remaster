"""Build a disposable native-mode engine from exact original NA1.04.

Reuses the frozen Launcher native recenter emitter and Resolution's seven-site
recipe, extended with inspected height and transition operands. No game install
is edited; every output must be new. No original artwork is generated or copied.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
ORIGINAL='03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a'
APPROVED='77a99e3bf0a2d8704f4060e5ce584514daa03bb44891fd3336a7d66460efe179'
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile

# Full original instructions, immediate offset, width/height, recovered role.
SITES=[
 (0x428c84,'6858020000',1,'height','initial application layout'),
 (0x428c89,'6820030000',1,'width','initial application layout'),
 (0x42b5b5,'c705d037710020030000',6,'width','mission default writer'),
 (0x42b5bf,'c705d437710058020000',6,'height','mission default writer'),
 (0x4a6480,'c7410820030000',3,'width','screen-object default'),
 (0x4a6487,'c7410c58020000',3,'height','screen-object default'),
 (0x433498,'6858020000',1,'height','initial screen request'),
 (0x43349d,'6820030000',1,'width','initial screen request'),
 (0x4ad865,'6858020000',1,'height','generic application reset request'),
 (0x4ad86a,'6820030000',1,'width','generic application reset request'),
 (0x429883,'6858020000',1,'height','initial input SetRect bottom'),
 (0x429888,'6820030000',1,'width','initial input SetRect right'),
 (0x4292d9,'6858020000',1,'height','application transition mode request'),
 (0x4292de,'6820030000',1,'width','application transition mode request'),
 (0x40117a,'c70508034f0000000040',6,'projection','mission projection m11'),
]

def sha(data):return hashlib.sha256(data).hexdigest()
def align(v,a):return (v+a-1)//a*a

def input_control(raw):
    if sha(raw)!=ORIGINAL:raise ValueError('Exact original NA1.04 required')
    image=pefile.PE(data=raw)
    if image.FILE_HEADER.Characteristics&0x20:raise ValueError('Unexpected LAA flag')
    last=image.sections[-1];header=image.sections[0].get_file_offset()+40*len(image.sections)
    if any(raw[header:header+40]) or header+40>image.OPTIONAL_HEADER.SizeOfHeaders:raise ValueError('No empty section header')
    raw_at=align(len(raw),image.OPTIONAL_HEADER.FileAlignment)
    va=align(last.VirtualAddress+max(last.Misc_VirtualSize,last.SizeOfRawData),image.OPTIONAL_HEADER.SectionAlignment)
    size=align(4096,image.OPTIONAL_HEADER.FileAlignment)
    out=bytearray(raw);out.extend(bytes(raw_at+size-len(out)))
    struct.pack_into('<8sIIIIIIHHI',out,header,b'.dwui\0\0\0',0,va,size,raw_at,0,0,0,0,0xE0000020)
    struct.pack_into('<H',out,image.FILE_HEADER.get_field_absolute_offset('NumberOfSections'),len(image.sections)+1)
    struct.pack_into('<I',out,image.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfImage'),align(va+size,image.OPTIONAL_HEADER.SectionAlignment))
    struct.pack_into('<I',out,image.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfCode'),image.OPTIONAL_HEADER.SizeOfCode+size)
    emitter=ROOT/'integration/components/intake-20261018.1/launcher/build_input_window_v1.py'
    spec=importlib.util.spec_from_file_location('native_probe_frozen_input',emitter)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    module.BASE=sha(out)
    result,report=module.build(bytes(out),hwnd_address=0xa387a4)
    passed=json.loads((emitter.parent/'input-window-patch-v2.json').read_text())
    if bytes.fromhex(report['stub_bytes'])[:-5]!=bytes.fromhex(passed['stub_bytes'])[:-5]:raise ValueError('Input body changed')
    return result,dict(report,emitter_sha256=sha(emitter.read_bytes()),native_body_matches_frozen_v2=True,
        reason='Original layout control plus accepted no-startup-switch recenter behavior; no960-specific UI adapters')

def build(raw,width,height):
    if type(width) is not int or type(height) is not int or not 640<=width<=8192 or not 480<=height<=8192:
        raise ValueError('Bounded integer launch dimensions required')
    control,input_report=input_control(raw);image=pefile.PE(data=raw);out=bytearray(control);rows=[]
    m11=2*(800/width)*((height-154)/446)
    for va,expected,offset,axis,role in SITES:
        at=image.get_offset_from_rva(va-image.OPTIONAL_HEADER.ImageBase);expected=bytes.fromhex(expected)
        if raw[at:at+len(expected)]!=expected:raise ValueError(f'Original instruction mismatch: {va:x}')
        value=struct.pack('<f',m11) if axis=='projection' else struct.pack('<I',width if axis=='width' else height)
        out[at+offset:at+offset+4]=value
        rows.append(dict(va=hex(va),file_offset=at+offset,role=role,axis=axis,original_instruction=expected.hex(),
            before=raw[at+offset:at+offset+4].hex(),after=value.hex()))
    allowed={i for r in rows for i in range(r['file_offset'],r['file_offset']+4)}
    changed={i for i,(a,b) in enumerate(zip(control,out)) if a!=b}
    if len(control)!=len(out) or not changed<=allowed:raise ValueError('Unexpected mode patch delta')
    return bytes(out),dict(schema='quantum-native-engine-1',original_sha256=ORIGINAL,
        approved_comparison_sha256=APPROVED,output_sha256=sha(out),control_with_input_sha256=sha(control),
        width=width,height=height,pixel_bits_policy='original initialization retained; selected actual format must be observed',
        scene=[0,0,width,height-154],projection_m11=struct.unpack('<f',struct.pack('<f',m11))[0],projection_m22=3.5,
        projection_policy='Retain original vertical calibration; scale original m11 by original/new scene aspect',
        input=input_report,sites=rows,mode_changed_bytes=len(changed),runtime_validated=False,
        scope='launch-selected scene experiment; no independent UI scaling, live resize, NPOT, LAA, texture/model/pool changes')

def main(original,output,width,height):
    if output.exists():raise ValueError('Use a NEW output directory')
    data,report=build(original.read_bytes(),width,height)
    output.mkdir(parents=True);(output/'ds9dw.exe').write_bytes(data)
    report['builder_sha256']=sha(Path(__file__).read_bytes())
    (output/'engine.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ('output_sha256','width','height','projection_m11','mode_changed_bytes')}))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--width',type=int,required=True);parser.add_argument('--height',type=int,required=True)
    args=parser.parse_args();main(args.original,args.output,args.width,args.height)
