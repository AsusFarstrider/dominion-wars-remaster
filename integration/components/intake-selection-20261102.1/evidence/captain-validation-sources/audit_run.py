"""Retain bounded captain-trial receipts alongside the general recorder audit."""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import struct


def audit(run):
    counts = Counter()
    samples = defaultdict(list)
    hits = Counter()
    shapes = Counter()
    owners = Counter()
    boundaries = Counter()
    published_owners = Counter()
    end = None
    errors = []
    commits_by_epoch = Counter()
    with (run / 'events.ndjson').open('rb') as stream:
        for line in stream:
            row = json.loads(line)
            kind = row.get('event', '')
            if kind == 'native_transition_prepaint':
                if row.get('current') in (1, 2, 3):
                    owners[(row.get('current'), row.get('draw'), row.get('scene'))] += 1
            if kind.startswith('native_hud_captain_'):
                counts[kind] += 1
                if len(samples[kind]) < 16:
                    samples[kind].append(row)
                if kind.endswith('_input') and row.get('hit'):
                    hits[(row['node'], row['buttons'])] += 1
                if kind.endswith('_commit'):
                    commits_by_epoch[row['hud_epoch']] += 1
                    shapes[(row['hardware_entries'], row['gpu_packets'], row['draws'])] += 1
                    if 'state' in row:
                        published_owners[(row['state'], row['draw_owner'])] += 1
                if kind.endswith('_boundary'):
                    boundaries[row['reason']] += 1
                if kind.endswith('_end'):
                    end = row
                if kind.endswith('_error'):
                    errors.append(row)
    gpu_receipts = []
    for row in samples['native_hud_captain_gpu']:
        values = {key: struct.unpack('<f', struct.pack('<I', row[key + '_bits']))[0]
                  for key in ('source_x', 'source_y', 'sent_x', 'sent_y')}
        gpu_receipts.append(dict(seq=row['seq'], node=hex(row['node']), **values))
    result = dict(schema='quantum-captain-trial-audit-1',
        counts=dict(counts), end=end, errors=errors,
        input_hits=[dict(node=hex(node), buttons=buttons, sampled_hits=n)
                    for (node, buttons), n in sorted(hits.items())],
        commit_shapes=[dict(hardware_entries=a, gpu_packets=b, draws=c, groups=n)
                       for (a, b, c), n in sorted(shapes.items())],
        commits_by_epoch=dict(commits_by_epoch), boundaries=dict(boundaries),
        mission_prepaints=[dict(state=s, draw=hex(d), scene=hex(g), frames=n)
                           for (s, d, g), n in sorted(owners.items())],
        published_owners=[dict(state=s, draw=hex(d), groups=n)
                          for (s, d), n in sorted(published_owners.items())],
        gpu_xy_receipts=gpu_receipts, samples=dict(samples),
        limitation='Receipts are intentionally sampled. Visual defects and acceptance come from the user; no pixel capture was enabled.')
    output = run / 'captain-audit-01.json'
    if output.exists():
        raise FileExistsError(output)
    output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(counts=dict(counts), end=end, errors=errors,
                          input_hits=result['input_hits'], commit_shapes=result['commit_shapes'],
                          boundaries=dict(boundaries), audit_sha256=hashlib.sha256(output.read_bytes()).hexdigest()), indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('run', type=Path)
    audit(parser.parse_args().run)
