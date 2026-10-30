"""Read-only native-release audit; writes a new PRIVATE report, never publishes.

Checks the actual package, all reachable source blobs, and audio COPY provenance
independently of the integration delta builder. Pattern checks are not legal or
security clearance. Original media and generated movies never enter the report.
"""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import zipfile


def sha(data):
    return hashlib.sha256(data).hexdigest()


def frames(data):
    h = struct.unpack_from('<11I', data)
    assert data[:4] == b'BIKi' and h[1] + 8 == len(data) and h[10] == 1
    offsets = [x & ~1 for x in struct.unpack_from('<' + str(h[2]+1) + 'I', data, 56)]
    assert offsets[0] == 56 + 4*(h[2]+1) and offsets[-1] == len(data)
    result = []
    for start, end in zip(offsets, offsets[1:]):
        audio_end = start + 4 + struct.unpack_from('<I', data, start)[0]
        assert start < audio_end <= end <= len(data)
        result.append((start, audio_end, end))
    return h, result


def audio_audit(before, delta):
    assert sha(before) == delta['input_sha256'] and len(before) == delta['input_size']
    out = bytearray()
    spans = []
    for op in delta['operations']:
        start = len(out)
        if op['kind'] == 'copy':
            offset, length = op['offset'], op['length']
            assert offset >= 0 and length > 0 and offset+length <= len(before)
            out.extend(before[offset:offset+length])
            spans.append((start, len(out), offset))
        else:
            assert op['kind'] == 'bytes'
            out.extend(base64.b64decode(op['data'], validate=True))
            spans.append((start, len(out), None))
    assert len(out) == delta['output_size'] and sha(out) == delta['output_sha256']
    old, sources = frames(before)
    new, targets = frames(out)
    assert old[2] == new[2] and old[7:9] == new[7:9]

    def copied(target_start, target_end, source_start):
        assert out[target_start:target_end] == before[source_start:source_start+target_end-target_start]
        cursor = target_start
        for start, end, source in spans:
            if end <= cursor or start >= target_end:
                continue
            assert start <= cursor and source is not None
            assert source + cursor-start == source_start + cursor-target_start
            cursor = min(end, target_end)
        assert cursor == target_end

    copied(44, 56, 44)
    audio_bytes = 12
    for (start, end, _), (target, target_end, _) in zip(sources, targets):
        assert end-start == target_end-target
        copied(target, target_end, start)
        audio_bytes += end-start
    return dict(frames=new[2], width=new[5], height=new[6], fps_numerator=new[7],
                fps_denominator=new[8], input_sha256=sha(before), output_sha256=sha(out),
                audio_header_bytes=12, copied_audio_bytes=audio_bytes,
                audio_packets_source_copy_only=True, literal_audio_bytes=0,
                total_literal_bytes=sum(end-start for start,end,source in spans if source is None))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo', type=Path, required=True)
    p.add_argument('--revision', required=True)
    p.add_argument('--package-source-revision', help='Original revision when source is privacy-mapped')
    p.add_argument('--zip', type=Path, required=True)
    p.add_argument('--sha256', required=True)
    p.add_argument('--original-movies-root', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    assert not a.report.exists(), 'Preserve previous report'
    git = lambda *args: subprocess.check_output(['C:/Program Files/Git/cmd/git.exe', '-C', str(a.repo), *args])
    secret = re.compile(rb'gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----|AKIA[A-Z0-9]{16}')
    restricted = re.compile(r'(?i)\.(?:blt|bik|sav|iso|cue|bin|obj|mp4|dwcard|dwbank|pdb)$|(?:^|/)ds9dw[^/]*\.exe$')
    commits = git('rev-list', a.revision).decode().splitlines()
    seen = set()
    privacy, secrets, restricted_paths, binaries = [], [], [], []
    blobs = []
    for commit in commits:
        for row in git('ls-tree', '-r', commit).splitlines():
            meta, name = row.split(b'\t', 1)
            _, kind, oid = meta.split()
            name = name.decode()
            if kind != b'blob' or oid in seen:
                continue
            seen.add(oid)
            blobs.append((commit, name, oid))
    batch = subprocess.run(['C:/Program Files/Git/cmd/git.exe','-C',str(a.repo),'cat-file','--batch'],
        input=b'\n'.join(oid for _,_,oid in blobs)+b'\n', stdout=subprocess.PIPE, check=True).stdout
    cursor = 0
    for commit, name, oid in blobs:
        end = batch.index(b'\n', cursor)
        header = batch[cursor:end].split()
        assert header[:2] == [oid,b'blob']
        size = int(header[2])
        data = batch[end+1:end+1+size]
        cursor = end+size+2
        identity = dict(revision=commit, path=name, sha256=sha(data), bytes=len(data))
        if restricted.search(name):
            restricted_paths.append(identity)
        if secret.search(data):
            secrets.append(identity)
        # Decode text escaped JSON paths without printing the private values.
        normalized = data.replace(b'\\\\', b'\\')
        if re.search(rb'(?i)[A-Z]:[\\/]Users[\\/](?!Public(?:[\\/]|\b)|Example(?:[\\/]|\b))[^\\/\s"<>]+', normalized):
            privacy.append(identity)
        if data[:2] == b'MZ':
            binaries.append(identity)
    zipped = a.zip.read_bytes()
    assert sha(zipped) == a.sha256
    with zipfile.ZipFile(a.zip) as z:
        assert z.testzip() is None
        names = z.namelist()
        assert len(names) == len(set(names))
        assert not any(n.startswith(('/', '\\')) or ':' in n or '..' in n.replace('\\','/').split('/') for n in names)
        allowed_visuals = {x['source'] for x in json.loads(z.read('manifest.json'))['runtime_files']
                           if x['target'].endswith(('.dwcard','.dwbank'))}
        assert not any(restricted.search(n) and n not in allowed_visuals for n in names)
        checksums = json.loads(z.read('CHECKSUMS.json'))
        for name, digest in checksums.items():
            assert sha(z.read(name)) == digest, name
        mdata = z.read('manifest.json')
        manifest = json.loads(mdata)
        package_revision = a.package_source_revision or a.revision
        assert manifest['source_revision'] == package_revision and not manifest['uncommitted_source']
        for item in manifest['files']:
            data = z.read(item['path'])
            assert sha(data) == item['sha256'] and len(data) == item['size'], item['path']
        identity_data = z.read('runtime/version.json')
        assert sha(identity_data) == manifest['version_identity']['sha256']
        identity = json.loads(identity_data)
        assert identity['source_revision'] == package_revision
        movies = []
        for item in manifest['asset_patches']:
            before = (a.original_movies_root / item['target']).read_bytes()
            audit = audio_audit(before, json.loads(z.read(item['path'])))
            assert audit['input_sha256'] == item['input_sha256']
            assert audit['output_sha256'] == item['output_sha256']
            movies.append(dict(target=item['target'], **audit))
        art = [x for x in manifest['runtime_files'] if x['target'].endswith(('.dwcard','.dwbank'))]
        assert len(art) == 9 and len(movies) == 3
        native = [dict(path=n,sha256=sha(z.read(n)),bytes=z.getinfo(n).file_size)
                  for n in names if n.endswith(('.dll','.exe'))]
        notices = [n for n in names if 'license' in n.lower() or 'notice' in n.lower()]
        assert 'runtime/DDrawCompat-LICENSE.txt' in notices
        assert 'foundation/payload/tools/I6COMP-LICENSE.txt' in notices
        package = dict(sha256=sha(zipped), bytes=len(zipped), members=len(names),
            manifest_sha256=sha(mdata), identity_sha256=sha(identity_data), identity=identity,
            checksummed_members=len(checksums), manifest_files=len(manifest['files']),
            movies=movies, authorized_visual_payloads=art, native_binaries=native, notices=notices,
            historical_embedded_status=manifest['status'], crc_pass=True)
    report = dict(revision=a.revision, history_commits=commits, unique_blobs_scanned=len(seen),
        credential_pattern_matches=secrets, personal_path_matches=privacy,
        restricted_history_paths=restricted_paths, native_source_binaries=binaries,
        package=package, limits=['Pattern scanning is not legal/security clearance.',
        'Original media remain private; no originals or decoded audio enter this report.',
        'Embedded historical approval status requires separate exact-package acceptance.'])
    a.report.parent.mkdir(parents=True, exist_ok=True)
    with a.report.open('x', encoding='utf-8') as f:
        json.dump(report, f, indent=2)
    print(json.dumps(dict(report=str(a.report), unique_blobs=len(seen),
        credential_matches=len(secrets), personal_paths=len(privacy), restricted_paths=restricted_paths,
        package_sha256=package['sha256'], movies=movies), indent=2))


if __name__ == '__main__':
    main()
