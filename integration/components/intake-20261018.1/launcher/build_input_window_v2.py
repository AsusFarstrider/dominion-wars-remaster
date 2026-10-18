import json
from build_input_window_v1 import ROOT,SOURCE,build
if __name__=='__main__':
 out,report=build(SOURCE.read_bytes(),hwnd_address=0xa387a4)
 paths=(ROOT/'input-window-probe-v2.exe',ROOT/'input-window-patch-v2.json')
 if any(p.exists() for p in paths):raise FileExistsError('Preserve artifacts')
 with paths[0].open('xb') as f:f.write(out)
 with paths[1].open('x') as f:json.dump(report,f,indent=2)
 print(report['candidate_sha256'])
