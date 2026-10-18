import json
from build_banner_trace_v1 import build,ROOT
if __name__=='__main__':
    out,r=build((ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes(),True)
    exe=ROOT/'banner-trace-probe-v2.exe';manifest=ROOT/'banner-trace-patch-v2.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
