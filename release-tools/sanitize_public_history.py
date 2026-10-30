"""Create a mapped, privacy-redacted public branch in an ISOLATED clone only.

Master must authorize the exact source tip and a verified complete private bundle
must already exist. Never run this against the active integration repository.
Existing published ancestry is retained. Generated report is private until reviewed.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--clone', type=Path, required=True)
    p.add_argument('--tip', required=True)
    p.add_argument('--published-anchor', required=True)
    p.add_argument('--report', type=Path, required=True)
    p.add_argument('--profile-name', required=True, help='Private profile component to replace; not stored in report')
    a = p.parse_args()
    assert not a.report.exists()
    assert a.clone.name == 'public-source-sanitized', 'Use the designated isolated clone'
    def git(*args, data=None):
        return subprocess.check_output(['C:/Program Files/Git/cmd/git.exe','-C',str(a.clone),*args],input=data)
    def blob(data):
        return git('hash-object','-w','--stdin',data=data).decode().strip()
    sha = lambda b: hashlib.sha256(b).hexdigest()
    commits = git('rev-list','--reverse',a.tip).decode().splitlines()
    assert a.published_anchor in commits
    published = set(git('rev-list',a.published_anchor).decode().splitlines())
    mappings, changes = {}, []
    for commit in commits:
        raw = git('cat-file','commit',commit)
        if commit in published:
            mappings[commit] = commit
            continue
        entries = {}
        for row in git('ls-tree','-rz',commit).split(b'\0'):
            if not row:
                continue
            meta, path = row.split(b'\t',1)
            mode, kind, oid = meta.decode().split()
            assert kind == 'blob'
            entries[path.decode()] = [mode, oid]
        modified = {}
        for name, (mode, oid) in entries.items():
            if not name.endswith(('/qfr-source.json','/build-production-06.log',
                                  '/gizmo-v5-asset-report.json','/gizmo-v5-preparation.json')):
                continue
            data = git('cat-file','blob',oid)
            if b'\0' in data:
                continue
            # Keep suffixes, tool names and findings. Redact only the profile
            # identity; ordinary E:/Codex and Program Files paths are retained.
            after = re.sub(rb'(?i)C:([\\/]+)Users([\\/]+)'+re.escape(a.profile_name.encode())+rb'(?=[\\/])',
                lambda m: b'C:'+m[1]+b'Users'+m[2]+b'Example',data)
            if after != data:
                modified[name] = after
                changes.append(dict(original_commit=commit,path=name,
                    original_sha256=sha(data),public_sha256=sha(after),change='personal-profile-prefix-only'))
        # Refresh internal source-lock hash/size relationships, not runtime bytes.
        for name in entries:
            if not re.fullmatch(r'integration/components/intake-native-[^/]+/source-lock.json',name):
                continue
            data = git('cat-file','blob',entries[name][1])
            lock = json.loads(data)
            root = name.rsplit('/',1)[0]+'/'
            changed = False
            for item in lock['files']:
                path = root+item['path']
                if path in modified:
                    item['sha256'] = sha(modified[path])
                    item['size'] = len(modified[path])
                    changed = True
            if root+'qfr-source.json' in modified:
                lock['source_manifest_sha256'] = sha(modified[root+'qfr-source.json'])
                changed = True
            if changed:
                after = (json.dumps(lock,indent=2,sort_keys=True)+'\n').encode()
                modified[name] = after
                changes.append(dict(original_commit=commit,path=name,
                    original_sha256=sha(data),public_sha256=sha(after),change='public-only-lock-rehash'))
        for name, data in modified.items():
            entries[name][1] = blob(data)
        # Build nested trees with Git plumbing; no working files are overwritten.
        tree = {}
        for name, (mode, oid) in entries.items():
            node = tree
            pieces = name.split('/')
            for part in pieces[:-1]:
                node = node.setdefault(part,{})
            node[pieces[-1]] = (mode,oid)
        def make_tree(node):
            lines = []
            for name, value in node.items():
                if isinstance(value,dict):
                    mode, kind, oid = '040000','tree',make_tree(value)
                else:
                    mode, oid = value
                    kind = 'blob'
                lines.append(f'{mode} {kind} {oid}\t{name}'.encode()+b'\0')
            return git('mktree','-z',data=b''.join(lines)).decode().strip()
        new_tree = make_tree(tree)
        header, message = raw.split(b'\n\n',1)
        lines = []
        for line in header.split(b'\n'):
            if line.startswith(b'tree '):
                line = b'tree '+new_tree.encode()
            elif line.startswith(b'parent '):
                line = b'parent '+mappings[line[7:].decode()].encode()
            assert not line.startswith(b'gpgsig '), 'Signed commits require separate handling'
            lines.append(line)
        mapped = git('hash-object','-t','commit','-w','--stdin',data=b'\n'.join(lines)+b'\n\n'+message).decode().strip()
        mappings[commit] = mapped
    git('update-ref','refs/heads/master',mappings[a.tip])
    report = dict(schema=1,original_tip=a.tip,public_tip=mappings[a.tip],
        published_anchor=a.published_anchor,existing_published_ancestry_unchanged=True,
        commit_mapping=mappings,changed_blobs=changes,
        original_master_and_worktrees_unchanged=True,
        tested_zip_and_runtime_unchanged=True,
        note='Redacted source can rebuild runtime; original exact ZIP retains private source/lock identity. No bit-identical ZIP claim from mapped HEAD.')
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x',encoding='utf-8') as f:
        json.dump(report,f,indent=2)
    print(json.dumps(dict(public_tip=mappings[a.tip],changed_blobs=len(changes),report=str(a.report))))


if __name__ == '__main__':
    main()
