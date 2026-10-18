"""Deduplicated rectangle-source observer; preserves the unlaunched v3 artifact."""
import json
from build_layout_trace_v3 import ROOT, build

if __name__ == '__main__':
    candidate, report = build((ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes(), unique=True)
    if (ROOT/'layout-trace-probe-v4.exe').exists() or (ROOT/'layout-trace-patch-v4.json').exists():
        raise FileExistsError('Preserve previous artifacts')
    with (ROOT/'layout-trace-probe-v4.exe').open('xb') as f: f.write(candidate)
    with (ROOT/'layout-trace-patch-v4.json').open('x') as f: json.dump(report,f,indent=2)
    print(report['candidate_sha256'])
