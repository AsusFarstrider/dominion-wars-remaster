import hashlib,json,struct
from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/esc-menu-state-01/events.ndjson')
rows=[json.loads(s) for s in p.read_text().splitlines()]
snapshots=[r for r in rows if r['kind']=='snapshot'];main=[r for r in snapshots if r['values']['state']==7]
assert [r['sequence'] for r in snapshots]==list(range(1,len(snapshots)+1))
assert rows[-1]['kind']=='sampler_end' and rows[-1]['changes_after_limit']==0
assert len(main)>0
for r in main:
 v=r['values'];root=r['root'];g=r['global_node'];native=bytes.fromhex(r['target']['native_hex']);outer=bytes.fromhex(r['target']['outer_hex'])
 assert [v[k] for k in ('state','pending','previous_state','input','secondary','draw','scene','pre','post','mode','mission_mode','escape')]==[7,7,2,0x42A210,0x42A230,0x42A290,0,0x47DDC0,0,16,0,0]
 assert root['address']==0x4E47C0 and root['id']==0x405 and root['flags']==0x84 and root['xywh']==[0,0,800,600]
 assert root['draw']==0x46ADD0 and not root['input'] and root['parent']==0x5F7ED8 and not root['next'] and not root['previous'] and root['child']==0x4E4B20 and root['data']==0x201
 assert g['flags']==4 and g['child']==root['address'] and not g['parent'] and not g['next'] and not g['draw'] and g['xywh']==[0,0,2560,1440]
 assert struct.unpack_from('<HH',outer,0x10)==struct.unpack_from('<HH',native,10)==(2560,1440)
 for i,b in enumerate(r['buttons']):
  assert b['address']==0x4E4850+i*0x90 and b['parent']==root['address'] and b['flags']==1 and b['input']==0x46AFA0 and b['draw']==0x46B440 and b['xywh'][2:]==[180,100] and not b['child'] and b['counter']==0
summary=dict(schema='quantum-esc-reentry-readonly-state-1',capture=str(p),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),snapshots=len(snapshots),main_snapshots=len(main),first_main_sequence=main[0]['sequence'],first_main_elapsed=main[0]['elapsed'],last_main_sequence=main[-1]['sequence'],pre_callback='0x47DDC0',post_callback=0,observed_rejection='CanvasRuntime::screen requires pre==0; captured re-entry pre==47DDC0',sampler_footer=rows[-1],exit_event=next(r for r in rows if r['kind']=='process_exit'),exit_interpretation='Rendering force-closed only the stuck isolated copy after read-only capture; not a normal-quit or spontaneous-crash result',limitations=['Sampler reads external process memory; not render publication, per-call execution, pixel capture or exhaustive event trace.','Snapshot sequence is consecutive and cap suppressed no changes; 200ms sampling may miss intermediate game transitions.','No internal publication flags or original button image/event-mask fields were collected; do not claim those memory values were observed.'])
(p.parent/'audit.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
