"""Extend a NEW cards/HD candidate with the exact Gizmo movie and Main underlay."""
import argparse,hashlib,json,shutil,struct,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent;root=here.parents[1]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
p=argparse.ArgumentParser();p.add_argument('run',type=Path);run=p.parse_args().run
i=load(run/'inputs.json');source=load(run/'source-manifest.json');game=Path(i['game'])
assert source['gizmo_main']['schema']=='quantum-gizmo-main-source-1'
assert game.parent==Path('E:/Codex/remaster-build-tests') and game.name.startswith('opening-cards-gizmo-main-')
assert game.resolve()!=Path(i['baseline']['game_root']).resolve() and not (run/'launch.json').exists()
subprocess.run([sys.executable,str(here.parent/'opening-cards/finalize_run.py'),str(run)],check=True)
managed=game/'.dw-remaster/manifest.json';manifest=load(managed)
gizmo=root/'cinematic-extraction/original-gizmo-hd-v1/GizmoLogo-1440p-original-audio.bik'
gizmo_sha='c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851'
original_sha='c80976eacec03c9dbe032b46524aaa99f2686d155bb846b34711e1a67d003e7e'
assert sha(gizmo)==gizmo_sha and gizmo.stat().st_size==76069912
with gizmo.open('rb') as f:header=f.read(56)
assert header[:4]==b'BIKi' and struct.unpack_from('<I',header,8)[0]==601 and struct.unpack_from('<IIII',header,20)==(2560,1440,2997,100)
receipt=gizmo.parent/'history-receipt.json';r=load(receipt);assert r['exit']==0 and r['asset_sha256']==gizmo_sha and 'compared=601' in r['stdout'] and 'guard_ok=1 equal=1' in r['stdout']
dest=game/'rs/Video/GizmoLogo.bik';assert sha(dest)==original_sha
rows=[x for x in manifest['original_files'] if x['path'].lower()=='rs/video/gizmologo.bik'];assert len(rows)==1 and rows[0]['sha256']==original_sha
backup=run/'original-assets/GizmoLogo.bik';shutil.copyfile(dest,backup);shutil.copyfile(gizmo,dest);rows[0].update(sha256=gizmo_sha,size=dest.stat().st_size)
movie=dict(path='rs/Video/GizmoLogo.bik',resource='5C6',sha256=gizmo_sha,original_sha256=original_sha,source=str(gizmo),backup=str(backup),
 source_profile=dict(width=2560,height=1440,frames=601,rate=2997,divisor=100,original_xy=[0,0]),
 decoder_receipt_sha256=sha(receipt),asset_report_sha256=sha(gizmo.parent/'asset-report.json'),runtime_validated=False)
manifest.setdefault('research_asset_overrides',{})['rs/Video/GizmoLogo.bik']=movie
bg=load(Path('E:/Codex/research/dominion-rendering/main-background-assets-v1/assets.json'));src=Path(bg['payload'])
assert sha(src)==bg['payload_sha256']=='94a131d5b71bec90e3d747a6475c18ece4ad6efdc98b5310b9daa74c7d3c0bcd'
assert sha(Path(bg['source']))==bg['source_sha256'];rel='rs/Remaster/OpeningCards/main.dwcard';target=game/rel
assert not target.exists();shutil.copyfile(src,target);bg['target']=rel
manifest['runtime_files'].append(dict(source=str(src),target=rel,sha256=sha(target),size=target.stat().st_size))
manifest['research_main_background']=bg;managed.write_text(json.dumps(manifest,indent=2)+'\n')
state=load(game/'.dw-remaster/installed.json')
expected={x['path']:manifest['engine']['output_sha256'] if x['path']=='ds9dw.exe' else x['sha256'] for x in manifest['original_files']}
expected.update({x['target']:x['sha256'] for x in manifest['runtime_files']})
expected['DDrawCompat-ds9dw.ini']=next(x['sha256'] for x in manifest['profiles'] if x['name']==state['profile'])
pins=[dict(path=rel,sha256=sha(game/rel),expected=h) for rel,h in sorted(expected.items())];assert all(x['sha256']==x['expected'] for x in pins)
(run/'verified-candidate-files.json').write_text(json.dumps(dict(files=pins,passed=True),indent=2)+'\n')
(run/'gizmo-registration.json').write_text(json.dumps(movie,indent=2)+'\n');(run/'main-background-registration.json').write_text(json.dumps(bg,indent=2)+'\n')
shutil.copyfile(here/'Launch-GizmoMain.ps1',run/'Launch-GizmoMain.ps1');shutil.copyfile(Path(__file__),run/'finalize_gizmo_main.py')
i=load(run/'inputs.json');i['submenus']['verified_candidate_sha256']=sha(run/'verified-candidate-files.json')
i['gizmo_main']=dict(schema='quantum-gizmo-main-run-1',launcher_sha256=sha(run/'Launch-GizmoMain.ps1'),gizmo=movie,main_background=bg,
 test='Natural Gizmo soundtrack/end; Main idle/hover/activation/version; Preferences slider/restore/cancel; Load cancel; NewGame/Previous; normal quit.',
 current_modified=False,game_launched=False,files_verified=len(pins))
(run/'inputs.json').write_text(json.dumps(i,indent=2)+'\n')
print(json.dumps(dict(game=str(game),run=str(run),files_verified=len(pins),wrapper_sha256=i['recorder_dll_sha256'],source_manifest_sha256=i['source_manifest_sha256'],ready=True)))
