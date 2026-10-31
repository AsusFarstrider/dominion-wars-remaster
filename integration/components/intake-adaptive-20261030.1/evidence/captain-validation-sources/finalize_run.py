"""Add the process-only HUD opt-in to an independently prepared opening runner."""
import argparse,hashlib,json,shutil,subprocess,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);run=p.parse_args().run
    subprocess.run([sys.executable,str(HERE.parent/'opening-flow/finalize_submenu_run.py'),'--run',str(run)],check=True)
    inputs=json.loads((run/'inputs.json').read_text());m=json.loads((run/'source-manifest.json').read_text())
    assert m['captain_trial']['schema']=='quantum-captain-trial-source-1'
    shutil.copyfile(HERE/'Launch-HudCaptains.ps1',run/'Launch-HudCaptains.ps1')
    shutil.copyfile(HERE/'finalize_run.py',run/'finalize_captain_run.py')
    inputs['captain_trial']=dict(schema='quantum-captain-trial-run-1',launcher_sha256=sha(run/'Launch-HudCaptains.ps1'),
        parent_manifest_sha256=m['captain_trial']['parent_manifest_sha256'],world_scene_changed=False,current_modified=False,
        test='Enter a mission with two ships/captains if available; inspect portraits/name/status, click and double-click slots, exercise auxiliary icons if available, move camera, pause/Escape, paused Alt-Tab, normal quit',
        temporary_layout='Captain group is enlarged beside existing small HUD in unused reserved-strip space; auxiliary icons remain overlays above the row.')
    (run/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    print(json.dumps(dict(run=str(run),game=inputs['game'],source_manifest_sha256=inputs['source_manifest_sha256'],launcher_sha256=inputs['captain_trial']['launcher_sha256'],game_launched=False)))
