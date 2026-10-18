"""Exact single-instruction clear-extent test on the HUD-aligned v2 image."""
import hashlib, json
from pathlib import Path
import sys
sys.path.insert(0, 'E:/Codex/tools/pydeps')
import pefile
ROOT = Path(__file__).resolve().parent
BASE = '8a5e8d87f8cd907d9f4e99304794fc5d6f2d85b4d374cc6617b892313aaa75da'

def build(raw):
    if hashlib.sha256(raw).hexdigest() != BASE:
        raise ValueError('Exact HUD-aligned v2 image required')
    p = pefile.PE(data=raw)
    pos = p.get_offset_from_rva(0x23a9)
    before = bytes.fromhex('a1d0f65f00')
    after = bytes.fromhex('b800000000')
    if raw[pos:pos+5] != before:
        raise ValueError('Unexpected clear-rectangle instruction')
    out = raw[:pos] + after + raw[pos+5:]
    return out, dict(base_sha256=BASE, candidate_sha256=hashlib.sha256(out).hexdigest(),
        va='0x4023a9', file_offset=pos, before=before.hex(), after=after.hex(),
        purpose='Full-width bottom-strip target clear; no widget or input changes',
        runtime_validated=False)

if __name__ == '__main__':
    out, report = build((ROOT/'game-ui-center-v2/ds9dw.exe').read_bytes())
    exe = ROOT/'strip-clear-probe-v1.exe'
    manifest = ROOT/'strip-clear-patch-v1.json'
    if exe.exists() or manifest.exists():
        raise FileExistsError('Preserve prior artifacts')
    with exe.open('xb') as f: f.write(out)
    with manifest.open('x') as f: json.dump(report, f, indent=2)
    print(report['candidate_sha256'])
