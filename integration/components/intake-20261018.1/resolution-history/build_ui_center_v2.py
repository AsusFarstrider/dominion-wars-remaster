"""Build a new isolated HUD-local-origin candidate; preserve v1 artifacts."""
import json
from build_ui_center_probe import build, ROOT

if __name__ == '__main__':
    out, report = build((ROOT/'game-wide960-v1/ds9dw.exe').read_bytes(), True)
    exe = ROOT/'ui-center-probe-v2.exe'
    manifest = ROOT/'ui-center-patch-v2.json'
    if exe.exists() or manifest.exists():
        raise FileExistsError('Preserve previous experiment')
    with exe.open('xb') as f:
        f.write(out)
    with manifest.open('x') as f:
        json.dump(report, f, indent=2)
    print(report['candidate_sha256'])
