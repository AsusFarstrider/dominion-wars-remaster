"""Combine validated repaint fix with isolated loading-origin corrections."""
import json
from build_selection_loading_v1 import ROOT,build
if __name__=='__main__':
    data,r=build((ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes(),True,True)
    if (ROOT/'selection-loading-probe-v3.exe').exists() or (ROOT/'selection-loading-patch-v3.json').exists():raise FileExistsError('Preserve artifacts')
    with (ROOT/'selection-loading-probe-v3.exe').open('xb') as f:f.write(data)
    with (ROOT/'selection-loading-patch-v3.json').open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
