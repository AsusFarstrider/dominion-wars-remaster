"""Summarize an immutable submenu run; retain gaps and failed publication evidence."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

def audit(run,observed_result=None):
    counts=Counter();commits=Counter();errors=[];receipts=[];last=None;issues=[];footer=None
    digest=hashlib.sha256();entries={};states=set();checkpoints=0
    with (run/'events.ndjson').open('rb') as f:
        for line_number,line in enumerate(f,1):
            digest.update(line)
            try:r=json.loads(line)
            except (ValueError,UnicodeError):issues.append(dict(line=line_number,reason='unparseable_record'));continue
            kind=r.get('event','');counts[kind]+=1
            if 'seq' in r:
                seq=r['seq']
                if last is not None and seq!=last+1:issues.append(dict(line=line_number,previous=last,next=seq))
                last=seq
            if kind=='capture_end':footer=r
            if kind=='native_recording_checkpoint':checkpoints+=1
            if kind in ('native_opening_error','native_canvas_reject') and len(errors)<32:errors.append(r)
            if kind=='native_opening_screen_commit':commits[f"state{r.get('state')}/root{r.get('root'):X}"]+=1
            if kind=='native_ui_traversal':
                key=(r.get('thread'),r.get('ui_owner_frame'))
                if r.get('phase')=='entry' and r.get('depth')==1:entries[key]=r
                if r.get('phase')=='return' and r.get('depth')==0:
                    entry=entries.pop(key,None)
                    if entry and len(receipts)<12:receipts.append(dict(frame=key[1],entry_esp=entry['stack_va'],return_esp=r['stack_va'],saved_bytes=entry['stack_va']-r['stack_va'],first_node=entry.get('first_node')))
            if kind.startswith('native_transition_') and 'current' in r:states.add(r['current'])
    if footer is None:issues.append(dict(reason='no_capture_end'))
    elif footer.get('last_sequence')!=last:issues.append(dict(reason='footer_sequence_mismatch'))
    result=dict(schema='quantum-submenu-run-audit-1',run=str(run),events_sha256=digest.hexdigest(),
        events_bytes=(run/'events.ndjson').stat().st_size,records=sum(counts.values()),event_counts=dict(counts),
        commits=dict(commits),first_errors=errors,outer_traversal_receipts=receipts,states=sorted(states),
        durable_checkpoints=checkpoints,exception_bytes=(run/'exceptions.bin').stat().st_size,
        capture_footer=footer,integrity_issues=issues,
        observed_result=observed_result or 'User reported remaining issues; detailed description pending.',
        diagnosis='No diagnosis is assigned by this generic audit; correlate captured events with the reported symptoms.',
        limitation='Metadata does not establish visual correctness or gameplay acceptance.')
    (run/'audit-01.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('records','events_sha256','commits','states','capture_footer','integrity_issues','outer_traversal_receipts')},indent=2))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);p.add_argument('--observed-result');a=p.parse_args();audit(a.run,a.observed_result)
