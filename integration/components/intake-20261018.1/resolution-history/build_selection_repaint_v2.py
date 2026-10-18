"""Corrected surface argument; menu repaint only, no loading changes."""
import json
from build_selection_loading_v1 import ROOT,build
if __name__=='__main__':
    data,r=build((ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes(),True,False)
    if (ROOT/'selection-repaint-probe-v2.exe').exists() or (ROOT/'selection-repaint-patch-v2.json').exists():raise FileExistsError('Preserve artifacts')
    with (ROOT/'selection-repaint-probe-v2.exe').open('xb') as f:f.write(data)
    with (ROOT/'selection-repaint-patch-v2.json').open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
