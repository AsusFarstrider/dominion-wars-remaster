"""Deterministic COPY/literal deltas; input/output SHA256 are mandatory.

No copyrighted base files are packaged. These deltas are not, by themselves,
redistribution clearance for modified art or executable snippets.
"""
import base64
import hashlib
import struct


def sha(data):
    return hashlib.sha256(data).hexdigest()


def add_copy(ops, offset, length):
    if not length:
        return
    if ops and ops[-1]['kind'] == 'copy' and ops[-1]['offset'] + ops[-1]['length'] == offset:
        ops[-1]['length'] += length
    else:
        ops.append(dict(kind='copy', offset=offset, length=length))


def add_bytes(ops, data):
    if data:
        ops.append(dict(kind='bytes', data=base64.b64encode(data).decode('ascii')))


def same_position(ops, before, after, source_offset=0):
    """Describe equal-length blocks without including their unchanged bytes."""
    if len(before) != len(after):
        raise ValueError('Equal block sizes required')
    cursor = 0
    while cursor < len(after):
        equal = before[cursor] == after[cursor]
        end = cursor + 1
        while end < len(after) and (before[end] == after[end]) == equal:
            end += 1
        if equal:
            add_copy(ops, source_offset + cursor, end-cursor)
        else:
            add_bytes(ops, after[cursor:end])
        cursor = end


def make_delta(before, after):
    ops = []
    common = min(len(before), len(after))
    same_position(ops, before[:common], after[:common])
    add_bytes(ops, after[common:])
    return finish(before, after, ops)


def bolt_blocks(data):
    """Read only outer BOLT block boundaries; no codec or assets are decoded."""
    if data[:4] != b'BOLT' or len(data) < 16:
        raise ValueError('BOLT header required')
    count = data[11] or 256
    header_end = 16+count*16
    roots = []
    expected = header_end
    for i in range(count):
        flags, _, _, number, size, start, token = struct.unpack_from('<BBBBIII', data, 16+i*16)
        number = number or 256
        end = start+number*16+size
        if flags or token or start != expected or end > len(data):
            raise ValueError('Unsupported BOLT root layout')
        entries = []
        for j in range(number):
            pos = start+j*16
            offset = struct.unpack_from('<I', data, pos+8)[0]
            if not start+number*16 <= offset < end:
                raise ValueError('BOLT payload outside its block')
            entries.append((j, offset))
        entries.sort(key=lambda row: row[1])
        if len(set(offset for _, offset in entries)) != len(entries):
            raise ValueError('Aliased BOLT payloads unsupported')
        roots.append((start, start+number*16, end, entries))
        expected = end
    if expected != len(data):
        raise ValueError('Trailing BOLT bytes unsupported')
    return header_end, roots


def make_bolt_delta(before, after):
    """Preserve relocated, unchanged compressed streams by source COPY offsets."""
    old_header, old_roots = bolt_blocks(before)
    new_header, new_roots = bolt_blocks(after)
    if old_header != new_header or len(old_roots) != len(new_roots):
        raise ValueError('Different BOLT directories unsupported')
    ops = []
    same_position(ops, before[:old_header], after[:new_header])
    for old, new in zip(old_roots, new_roots):
        os, ot, oe, rows = old
        ns, nt, ne, targets = new
        if ot-os != nt-ns or [i for i, _ in rows] != [i for i, _ in targets]:
            raise ValueError('BOLT leaf ordering changed')
        same_position(ops, before[os:ot], after[ns:nt], os)
        first_old = rows[0][1]
        first_new = targets[0][1]
        if first_old-ot != first_new-nt:
            raise ValueError('BOLT table gap changed')
        same_position(ops, before[ot:first_old], after[nt:first_new], ot)
        for idx, ((_, offset), (_, target)) in enumerate(zip(rows, targets)):
            old_end = rows[idx+1][1] if idx+1 < len(rows) else oe
            new_end = targets[idx+1][1] if idx+1 < len(targets) else ne
            source, dest = before[offset:old_end], after[target:new_end]
            if source == dest:
                add_copy(ops, offset, len(source))
            else:
                add_bytes(ops, dest)
    return finish(before, after, ops)


def finish(before, after, ops):
    delta = dict(schema=1, input_sha256=sha(before), input_size=len(before),
                 output_sha256=sha(after), output_size=len(after), operations=ops)
    if apply_delta(before, delta) != after:
        raise ValueError('Delta round-trip mismatch')
    return delta


def apply_delta(before, delta):
    if delta.get('schema') != 1 or len(before) != delta['input_size'] or sha(before) != delta['input_sha256']:
        raise ValueError('Unsupported input')
    if not 0 < delta['output_size'] <= 128*1024*1024:
        raise ValueError('Output size outside package limit')
    result = bytearray()
    for op in delta['operations']:
        if op['kind'] == 'copy':
            offset, length = op['offset'], op['length']
            if offset < 0 or length <= 0 or offset+length > len(before):
                raise ValueError('Invalid copy extent')
            result.extend(before[offset:offset+length])
        elif op['kind'] == 'bytes':
            result.extend(base64.b64decode(op['data'], validate=True))
        else:
            raise ValueError('Unknown delta operation')
        if len(result) > delta['output_size']:
            raise ValueError('Output overrun')
    if len(result) != delta['output_size'] or sha(result) != delta['output_sha256']:
        raise ValueError('Output identity mismatch')
    return bytes(result)
