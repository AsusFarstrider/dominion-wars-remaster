"""Record real-session metadata and an explicitly supplied human test result."""
import argparse,collections,hashlib,json,shutil
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--stamp',required=True);p.add_argument('--width',type=int,required=True);p.add_argument('--height',type=int,required=True);p.add_argument('--human',required=True);a=p.parse_args()
game=Path('E:/Codex/remaster-build-tests/adaptive-output-20261030-01');meta=game/'.dw-remaster';here=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
source=meta/('output-evidence-'+a.stamp);events=[json.loads(s) for s in (source/'events.ndjson').read_text().splitlines()]
start=events[0];pid=start['pid'];snapshots=[e for e in events if e['kind']=='snapshot'];end=events[-1]
assert start['kind']=='start' and end['kind']=='sampler_end'
assert events[-2]==dict(kind='process_exit',exit_code=0)
assert end['changes_after_limit']==0 and end['snapshots']==len(snapshots)
assert not [e for e in events if e['kind'] in ('sampler_error','time_limit')]
matching=[]
for run in (meta/'runs').iterdir():
 r=run/'result.json'
 if r.exists():
  result=load(r);selection=result.get('output_selection',{})
  if (selection.get('width'),selection.get('height'))==(a.width,a.height):matching.append(run)
assert len(matching)==1,matching
run=matching[0];result=load(run/'result.json');assert result['launcher_exit']==0 and result['managed_files_verified']
ledger=list(Path('C:/Users/Example/AppData/Local/Dominion Wars Remastered/Rendering').glob('rendering-'+str(pid)+'-*.ndjson'));assert len(ledger)==1
notes=[json.loads(s) for s in ledger[0].read_text().splitlines()]
assert any(n['event']=='native_output_bound' and f'width={a.width}; height={a.height};' in n['detail'] for n in notes)
assert any(n['event']=='functional_ready' for n in notes)
faults=[n for n in notes if any(x in n['event'] for x in ('refused','rejected','error','failure'))]
sizes=lambda key:sorted({tuple(e['values']['dimensions'][key]) for e in snapshots if e['values'].get('dimensions') and e['values']['dimensions'].get(key)})
assert [a.width,a.height] in [list(s) for s in sizes('client_wh')]
assert sizes('native_image_wh')==[(a.width,a.height)]
out=here/('live-'+str(a.width)+'x'+str(a.height)+'-01');assert not out.exists();out.mkdir()
files=[(source/'events.ndjson','events.ndjson'),(ledger[0],'rendering.ndjson')]
for name in ('result.json','output-selection.json','focus.log','helper-error.log','launcher-output.log','launcher-error.log'):
 f=run/name
 if f.exists():files.append((f,name))
for suffix,name in (('-sampler.log','sampler.log'),('-sampler-error.log','sampler-error.log')):
 f=meta/('output-evidence-'+a.stamp+suffix)
 if f.exists():files.append((f,name))
copied={}
for f,name in files:shutil.copyfile(f,out/name);copied[name]=dict(source=str(f),sha256=sha(out/name))
fingerprint={name:sha(game/name) for name in ('ds9dw.exe','ddraw.dll','Remaster.Rendering.json','version.json','Launch-Remaster.ps1','Remaster.Core.psm1','Watch-RemasterFocus.ps1','DDrawCompat-ds9dw.ini')}
assert fingerprint['ds9dw.exe']==start['executable_sha256'] and fingerprint['ddraw.dll']==start['wrapper_sha256']
record=dict(schema='quantum-adaptive-live-acceptance-1',client_date='2026-10-30',game=str(game),pid=pid,run=str(run),selected_physical_output=result['output_selection'],human_result=a.human,normal_process_exit=0,normal_managed_launcher_exit=0,managed_files_verified=True,snapshots=len(snapshots),changes_omitted_after_capacity=end['changes_after_limit'],observed_client_sizes=sizes('client_wh'),observed_screen_sizes=sizes('screen_wh'),observed_native_image_sizes=sizes('native_image_wh'),raw_state_counts=dict(collections.Counter(str(e['values']['state']) for e in snapshots)),selected_ledger_events=len(notes),selected_ledger_failures=faults,fingerprint=fingerprint,evidence=copied,limitations=['200ms read-only state sampling can miss transitions between polls; zero capacity loss is not exhaustive event capture.','Selected safety ledger is bounded to128 events and is not the comprehensive flight recorder.','No GPU pixel capture, screenshot analysis, per-draw audit or comprehensive hardware proof.','Human result applies to the requested playtest categories, not every menu/state/gameplay feature.'])
(out/'acceptance.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps(dict(acceptance=str(out/'acceptance.json'),sha256=sha(out/'acceptance.json'),snapshots=len(snapshots),selected_ledger_failures=faults),indent=2))
