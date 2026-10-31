"""Audit completed combined captures without conflating video lifetimes or pixel stages."""
import argparse,hashlib,json
from collections import Counter
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('run',type=Path);a=p.parse_args();run=a.run
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not (run/'gizmo-main-audit-01.json').exists(),'Preserve completed audits'
g=load(run/'audit-01.json');i=load(run/'inputs.json')
rows=[json.loads(s) for s in (run/'events.ndjson').read_text().splitlines()]
select=lambda k:[r for r in rows if r['event']==k]
issues=[]
def require(ok,why):
 if not ok:issues.append(why)
footer=g['capture_footer']
require(footer and footer['complete'] and footer['lost']==0 and footer['stalls']==0 and not g['integrity_issues'] and g['exception_bytes']==0,'Recorder integrity failure')
require(load(run/'exit.json')['launcher_exit']==0,'Nonzero launcher exit')
require(bool(select('process_exit')) and select('process_exit')[-1]['code']==0,'No normal game exit')
require(sha(run/'source-manifest.json')==i['source_manifest_sha256'],'Source manifest changed')
require(sha(Path(i['game'])/'ddraw.dll')==i['recorder_dll_sha256'],'Installed wrapper changed')
movies=[]
for resource,name,frames,rate,divisor,xy in ((0x5C5,'Paramount',225,15,1,[80,60]),(0x5C6,'Gizmo/Mass Media',601,2997,100,[0,0])):
 copies=[r for r in select('native_hd_copy') if r['external_resource']==resource]
 require(bool(copies),'No native video evidence for '+name)
 lifetimes=sorted({r['lifetime'] for r in copies})
 for life in lifetimes:
  part=[r for r in copies if r['lifetime']==life];pixels=[r for r in select('native_hd_pixels') if r['lifetime']==life]
  for r in part:
   require((r['declared_frames'],r['rate'],r['rate_divisor'])==(frames,rate,divisor),'Bad identified movie profile')
   require([r['original_x'],r['original_y']]==xy,'Unexpected original movie coordinates')
   require([r['source_width'],r['source_height'],r['internal_width'],r['internal_height'],r['source_pitch'],r['source_x'],r['source_y']]==[2560,1440,2560,1440,5120,0,0],'Invalid persistent source geometry')
   require(r['guards_intact'] and r['result'] in (0,1),'Invalid codec result or damaged guards')
   require(r['identity_written'] and r['history_valid'] and not r['native_writes_withheld'],'Movie native write withheld')
   require((r['target_width'],r['target_height'])==(2560,1440) and r['target_pitch']>=5120,'Invalid native output geometry')
  require(bool(pixels) and all(r['equal'] and r['source_sha256']==r['target_sha256'] for r in pixels),'Movie native pixel identity failure')
  require(part[0]['copy_flags']==0x8000000A,'First movie copy not full initialization')
  require(all(r['copy_flags']==10 for r in part[1:]),'Unexpected later movie flags')
  unlocks=[r for r in select('native_hd_unlock') if r['lifetime']==life]
  require(len(unlocks)==len(part) and all(r['published'] and r['hr']>=0 for r in unlocks),'Movie Unlock publication mismatch')
  require(any(r['lifetime']==life for r in select('native_hd_close')),'Movie lifetime not closed')
  movies.append(dict(name=name,resource=resource,lifetime=life,copies=len(part),first_frame=part[0]['frame'],last_frame=part[-1]['frame'],
   complete_frame_range=[r['frame'] for r in part]==list(range(1,frames+1)),results=dict(Counter(r['result'] for r in part)),pixel_receipts=pixels,unlocks=len(unlocks)))
composed=select('native_main_background_composed');pixels=select('native_main_background_pixels')
require(bool(composed),'No native Main background composed')
asset=i['gizmo_main']['main_background'];path=Path(i['game'])/asset['target']
require(sha(path)==asset['payload_sha256'],'Installed Main asset changed')
source=hashlib.sha256(path.read_bytes()[16:]).hexdigest()
require(len(pixels)==1 and pixels[0]['equal'] and pixels[0]['source_sha256']==pixels[0]['target_sha256']==source,'Native Main underlay identity missing/mismatched')
commit_kinds=('native_canvas_commit','native_opening_screen_commit')
commits_by_scope={}
for r in rows:
 if r['event'] in commit_kinds and r.get('background_scope'):
  commits_by_scope.setdefault(r['background_scope'],[]).append(r)
scopes={};duplicates=[];unpublished=[]
for r in composed:
 scope=r['scope'];scopes[scope]=scopes.get(scope,0)+1
 require((r['source_width'],r['source_height'],r['target_width'],r['target_height'])==(2560,1440,2560,1440) and r['identity_background'],'Invalid Main source/native geometry')
 commits=commits_by_scope.get(scope,[])
 if len(commits)!=1:unpublished.append(dict(scope=scope,commit_count=len(commits)))
duplicates=[k for k,v in scopes.items() if v!=1]
require(not duplicates and not unpublished,'Missing/duplicate Main composition/publication')
rejects=select('native_main_background_reject');fallback=select('native_main_background_legacy_fallback')
require(not rejects and not fallback,'Native background rejected/fell back; inspect captured primitive')
errors=select('native_opening_error')+select('native_canvas_coverage_lost')
require(not errors,'Rendering error or lost drawing coverage')
result=dict(schema='quantum-gizmo-main-audit-1',source_manifest_sha256=i['source_manifest_sha256'],wrapper_sha256=i['recorder_dll_sha256'],
 run=str(run),events_sha256=g['events_sha256'],records=g['records'],footer=footer,human_observation=g['observed_result'],
 movies=movies,main=dict(composed=len(composed),native_underlay_pixel_receipts=pixels,duplicate_scopes=duplicates,unpublished_scopes=unpublished,
 rejects=rejects[:4],reject_count=len(rejects),fallback_count=len(fallback)),errors=errors,issues=sorted(set(issues)),passed=not issues,
 limitations=['Pixel hashes compare private RGB565 source to original-format locked native surface before foreground; not GPU scanout.',
 'Per-profile audio, natural ending and skips require human observation; complete frame range is reported independently.',
 'Raw foreground coverage and original color conversion are verified in code/fixtures; no live per-pixel foreground reference.',
 'One display/system only; unchanged gameplay was not exercised in this menu/video run.'])
(run/'gizmo-main-audit-01.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(dict(passed=result['passed'],issues=result['issues'],movies=[{k:v for k,v in m.items() if k!='pixel_receipts'} for m in movies],main={k:v for k,v in result['main'].items() if k not in ('native_underlay_pixel_receipts','rejects')},footer=footer),indent=2))
