"""Pin a prepared submenu run and its efficient existing-helper launcher."""
import hashlib
import json
from pathlib import Path
import shutil
import argparse

BASE=Path('E:/Codex/research/dominion-rendering')
RUN=BASE/'runs/opening-submenus-1440-01'
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--run',type=Path,default=RUN);RUN=parser.parse_args().run
    inputs=load(RUN/'inputs.json')
    if 'submenus' in inputs:raise ValueError('Already finalized')
    source=load(RUN/'source-manifest.json');assert source['submenu_followup']['schema']=='quantum-opening-submenus-source-1'
    game=Path(inputs['game']);m=load(game/'.dw-remaster/manifest.json');state=load(game/'.dw-remaster/installed.json')
    expected={r['path']:m['engine']['output_sha256'] if r['path']=='ds9dw.exe' else r['sha256'] for r in m['original_files']}
    expected.update({r['target']:r['sha256'] for r in m['runtime_files']})
    expected['DDrawCompat-ds9dw.ini']=next(p['sha256'] for p in m['profiles'] if p['name']==state['profile'])
    pins=[dict(path=rel,sha256=sha(game/rel),expected=digest) for rel,digest in sorted(expected.items())]
    assert all(p['sha256']==p['expected'] for p in pins)
    (RUN/'verified-candidate-files.json').write_text(json.dumps(dict(files=pins,passed=True),indent=2)+'\n')
    shutil.copyfile(HERE/'Launch-Submenus.ps1',RUN/'Launch-Submenus.ps1')
    shutil.copyfile(HERE/'finalize_submenu_run.py',RUN/'finalize_submenu_run.py')
    inputs['submenus']=dict(schema='quantum-opening-submenus-run-1',launcher_sha256=sha(RUN/'Launch-Submenus.ps1'),
        verified_candidate_sha256=sha(RUN/'verified-candidate-files.json'),
        source_parent_manifest_sha256=source['submenu_followup']['parent_manifest_sha256'],
        current_untouched=True,game_launched=False,recording='compact nonblocking opening profile; GPU readback disabled',
        test='Preferences slider drag/restore/cancel -> Load view/select/scroll/cancel -> Credits exit -> reachable local Multiplayer UI/back -> New Game/briefing/fleet -> loading panel -> responsive mission -> normal quit')
    inputs['opening_flow']['known_original_sized']=['In-game HUD/gameplay interface','Unrelated confirmation/tooltip routes','Multiplayer states23+']
    (RUN/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    print(json.dumps(dict(run=str(RUN),game=str(game),runtime_files_verified=len(pins),launcher_sha256=inputs['submenus']['launcher_sha256'],ready=True,game_launched=False)))
