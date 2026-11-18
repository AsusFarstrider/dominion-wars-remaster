"""Inspect clean or abruptly terminated QFR files without changing the evidence.

A valid prefix and durable checkpoint are useful evidence, never a complete
capture assertion. Missing queue tails and exceptions bypassing VEH stay unknown.
"""
import argparse
from collections import Counter,deque
import hashlib
import json
from pathlib import Path
import struct

def inspect(run):
    path=run/'events.ndjson';digest=hashlib.sha256();counts=Counter();recent=deque(maxlen=80)
    transitions=deque(maxlen=24);failures=deque(maxlen=100);last=0;footer=None;issues=[];mode=None
    valid_bytes=0;trailing_bytes=0
    if path.exists():
        total=path.stat().st_size
        with path.open('rb') as stream:
            for line in stream:
                digest.update(line)
                try:e=json.loads(line)
                except (ValueError,UnicodeError):
                    trailing_bytes=total-valid_bytes;issues.append('unparsed trailing bytes; only preceding prefix inspected')
                    for block in iter(lambda:stream.read(1024*1024),b''):digest.update(block)
                    break
                kind=e.get('event');counts[kind]+=1;valid_bytes+=len(line)
                if footer:issues.append('event after footer')
                if kind=='capture_end':footer=e;continue
                if e.get('seq')!=last+1:issues.append('noncontiguous sequence near '+str(e.get('seq')))
                last=e.get('seq',last)
                if kind=='native_recording_durability':mode=e
                if kind in ('native_transition_install_entry','native_transition_precleanup'):transitions.append(e)
                if kind in ('native_opening_error','native_api_failure','native_ui_gap','native_transition_gap','native_gap'):failures.append(e)
                if kind.startswith(('native_opening_','native_canvas_','native_transition_')):recent.append(e)
    else:issues.append('no event stream')
    checkpoints=[];journal_tail=False
    if (run/'durable.ndjson').exists():
        for line in (run/'durable.ndjson').read_bytes().splitlines():
            try:checkpoints.append(json.loads(line))
            except (ValueError,UnicodeError):journal_tail=True;break
    for e in checkpoints:
        if e['through_sequence']>last:issues.append('durable checkpoint exceeds inspected sequence')
    exceptions=[];partial=0
    if (run/'exceptions.bin').exists():
        raw=(run/'exceptions.bin').read_bytes();partial=len(raw)%92
        for offset in range(0,len(raw)-partial,92):
            fields=struct.unpack_from('<21IQ',raw,offset)
            if fields[:3]!=(0x31455851,92,1):issues.append('invalid exception record');break
            exceptions.append(dict(code=hex(fields[3]),flags=fields[4],thread=fields[5],address=hex(fields[6]),
                eip=hex(fields[7]),esp=hex(fields[8]),registers=dict(zip(('eax','ebx','ecx','edx','esi','edi','ebp'),fields[9:16])),
                parameter_count=fields[16],parameters=list(fields[17:21]),uptime_ms=fields[21],first_chance=True,
                fatality='not established by this observer'))
    if footer and footer.get('last_sequence')!=last:issues.append('footer sequence mismatch')
    complete=bool(footer and footer.get('complete') and footer.get('lost')==0 and not issues and not trailing_bytes)
    return dict(schema='quantum-recording-recovery-1',run=str(run.resolve()),events_sha256=digest.hexdigest() if path.exists() else None,
        valid_prefix_bytes=valid_bytes,trailing_unparsed_bytes=trailing_bytes,last_inspected_sequence=last,
        counts=dict(counts),footer=footer,capture_complete=complete,issues=issues,durability_mode=mode,
        durable_checkpoints=len(checkpoints),last_durable_checkpoint=checkpoints[-1] if checkpoints else None,
        journal_has_unparsed_tail=journal_tail,exception_records=exceptions,exception_trailing_bytes=partial,
        recent_context=list(recent),recent_transitions=list(transitions),recent_recorded_failures=list(failures),
        lost_events=footer.get('lost') if footer else 'unknown: queued or buffered tail after last checkpoint may be absent',
        limitations=['Prefix recovery is not a full QFR registry/blob integrity import.',
            'Checkpoint interval does not bound backlog latency; only confirmed through_sequence is durable.',
            'First-chance exceptions can be handled; external kill/fast-fail can bypass the observer.',
            'No log before recorder initialization; no pixel or physical scanout reconstruction.',
            'No footer means incomplete even if every retained JSON record parses.'])

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args()
    if a.output.exists():raise ValueError('Use a NEW output')
    result=inspect(a.run);result['analyzer_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    a.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('capture_complete','last_inspected_sequence','durable_checkpoints','issues','lost_events')}))
