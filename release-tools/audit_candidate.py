"""Read-only source/package audit. Writes a NEW private report, never publishes."""
import hashlib
import io
import json
from pathlib import Path
import re
import subprocess
import tarfile
import zipfile

ROOT=Path(__file__).resolve().parents[1]
GIT='C:/Program Files/Git/cmd/git.exe'
REV='921826392c5b6f5acdc523f4db2429cbad234782'
ZIP=Path('E:/Codex/remaster-builds/int-20261018.1-candidate-original/dominion-wars-remaster-int-20261018.1.zip')
EXPECTED='7633f657e8c5845d083c2d6cf6a871b8360155b7f8d7bc1733e839134392c864'
REPORT=ROOT/'handoffs/release/int-20261018.1/publication-audit.json'

def sha(b):return hashlib.sha256(b).hexdigest()
def git(*args):return subprocess.check_output([GIT,'-C',str(ROOT),*args])

def main():
    if REPORT.exists():raise ValueError('Preserve previous audit; use a new report location')
    commits=git('rev-list',REV).decode().splitlines()
    source=[];path_files=[];secret_files=[];restricted=[];native=[]
    secret=re.compile(r'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----|AKIA[A-Z0-9]{16})')
    forbidden=re.compile(r'(?i)(?:\.(?:blt|bik|sav|iso|cue|obj|png|jpg|jpeg|pdb)$|(?:^|/)ds9dw[^/]*\.exe$|(?:^|/)(?:\.env|id_rsa|credentials)$)')
    for revision in commits:
        tar=git('archive','--format=tar',revision)
        with tarfile.open(fileobj=io.BytesIO(tar)) as tree:
            for member in tree.getmembers():
                if not member.isfile():continue
                data=tree.extractfile(member).read();name=member.name
                source.append(dict(revision=revision,path=name,bytes=len(data),sha256=sha(data)))
                if forbidden.search(name):restricted.append(dict(revision=revision,path=name))
                if data.startswith(b'MZ'):native.append(dict(path=name,sha256=sha(data),bytes=len(data)))
                if b'\0' not in data:
                    text=data.decode('utf-8',errors='replace')
                    if re.search(r'(?i)[A-Z]:[\\/](?:Users|Codex)[\\/]',text):path_files.append(dict(revision=revision,path=name))
                    if secret.search(text):secret_files.append(dict(revision=revision,path=name))
    zipped=ZIP.read_bytes()
    if sha(zipped)!=EXPECTED:raise ValueError('Approved ZIP identity mismatch')
    with zipfile.ZipFile(io.BytesIO(zipped)) as archive:
        if archive.testzip():raise ValueError('ZIP CRC failure')
        names=archive.namelist()
        if len(names)!=len(set(names)):raise ValueError('Duplicate ZIP members')
        if any(name.startswith(('/', '\\')) or '..' in name.replace('\\','/').split('/') for name in names):raise ValueError('Unsafe member path')
        manifest_data=archive.read('manifest.json');manifest=json.loads(manifest_data)
        if sha(manifest_data)!='ee9c6cff67be3ccf7c607cddb41fccb398aaff20ee945c4ab359a896640adcb4':raise ValueError('Manifest identity mismatch')
        checksums=json.loads(archive.read('CHECKSUMS.json'))
        for name,expected in checksums.items():
            if sha(archive.read(name))!=expected:raise ValueError('Member checksum mismatch: '+name)
        for record in manifest['files']:
            data=archive.read(record['path'])
            if len(data)!=record['size'] or sha(data)!=record['sha256']:raise ValueError('Manifest member mismatch')
        forbidden_members=[n for n in names if forbidden.search(n) or 'saber' in n.lower()]
        if forbidden_members:raise ValueError('Restricted package members: '+str(forbidden_members))
        required=['foundation/LICENSE.txt','foundation/payload/DDrawCompat-LICENSE.txt','foundation/payload/tools/I6COMP-LICENSE.txt','third_party/DDrawCompat-source/LICENSE.txt']
        if any(n not in names for n in required):raise ValueError('Missing carried-through notice')
        report=dict(source_revision=REV,history_commits=commits,source_files=source,
            restricted_history_paths=restricted,credential_pattern_files=secret_files,
            historical_absolute_path_files=path_files,native_binaries=native,
            package=dict(path=str(ZIP),sha256=EXPECTED,bytes=len(zipped),members=len(names),
                manifest_sha256=sha(manifest_data),checksummed_members=len(checksums),
                manifest_files=len(manifest['files']),crc_pass=True,asset=manifest['asset'],
                historical_approvals=manifest['approvals'],required_notices_present=required),
            unresolved=['Project-authored integration-source license decision',
                'Review executable-delta distribution scope; delta format alone is not clearance',
                'Decide treatment of committed historical local paths before public history push',
                'Supplement frozen historical approval fields with current package-bound release record'],
            limits=['Pattern scanning is not proof that no credential or protected material exists.',
                    'Binary/third-party provenance requires separate review; not all executable files are game files.',
                    'No license grant, repository creation, push, upload or runtime modification performed.'])
    REPORT.parent.mkdir(parents=True,exist_ok=True)
    with REPORT.open('x',encoding='utf-8') as out:json.dump(report,out,indent=2)
    print(json.dumps(dict(report=str(REPORT),commits=len(commits),source_files=len(source),
        restricted_history_paths=restricted,credential_pattern_files=secret_files,
        historical_absolute_path_files=len(path_files),native_binaries=native,package=report['package'],
        unresolved=report['unresolved']),indent=2))

if __name__=='__main__':main()
