"""Check exact-byte application in an owned disposable source tree."""
import difflib,hashlib,json,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2];out=root/'handoffs/master/rendering-adaptive-output-20261030'
r=json.loads((out/'component.json').read_text());source=Path(r['frozen_source']);before=Path('E:/Codex/research/dominion-rendering/production-runtime-source-v2')
check=Path('E:/Codex/research/dominion-rendering/dynamic-output-patch-check-02');assert not check.exists();check.mkdir()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
lines=lambda p:p.read_bytes().decode().splitlines(keepends=True)
def apply(name):
 for mode in ('--check','--whitespace=nowarn'):
  subprocess.run(['git','-c','core.autocrlf=false','-c','core.whitespace=cr-at-eol','apply',mode,str(out/name)],cwd=check,check=True)
diff=[]
for n in r['provider_changed_from_baseline']:
 if (before/n).exists():
  (check/n).parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(before/n,check/n)
 diff.extend(difflib.unified_diff(lines(before/n) if (before/n).exists() else [],lines(source/n),fromfile='a/'+n if (before/n).exists() else '/dev/null',tofile='b/'+n))
(out/'provider-source.patch').write_bytes(''.join(diff).encode());apply('provider-source.patch')
assert all(sha(check/n)==r['provider_production_pins'][n] for n in r['provider_changed_from_baseline'])
print('Provider patch:13 exact target hashes',flush=True)
current=Path('E:/Codex/remaster-builds/playable-int-20261029.1');diff=[];(check/'runtime').mkdir()
for n,h in r['launcher_runtime'].items():
 shutil.copyfile(current/n,check/'runtime'/n)
 diff.extend(difflib.unified_diff(lines(current/n),lines(out/'launcher-source/runtime'/n),fromfile='a/runtime/'+n,tofile='b/runtime/'+n))
(out/'launcher-source.patch').write_bytes(''.join(diff).encode());apply('launcher-source.patch')
assert all(sha(check/'runtime'/n)==h for n,h in r['launcher_runtime'].items())
print('Launcher patch:3 exact target hashes',flush=True)
here=Path(__file__).resolve().parent
shutil.copyfile(here/'HANDOFF.md',out/'README.md')
shutil.copyfile(root/'rendering-research/DYNAMIC-OUTPUT-PROTOTYPE-01.md',out/'evidence/investigation.md')
for n in ('package_handoff.py','validate_handoff.py'):shutil.copyfile(here/n,out/'evidence'/n)
(out/'patch-validation.json').write_text(json.dumps(dict(provider_exact_hashes=13,launcher_exact_hashes=3,disposable_apply_check=str(check),git_options=['-c','core.autocrlf=false','-c','core.whitespace=cr-at-eol'],note='Disable Windows auto-CRLF for exact frozen source byte hashes; no source logic changes.',no_current_writes=True),indent=2)+'\n')
checks={p.relative_to(out).as_posix():sha(p) for p in out.rglob('*') if p.is_file() and p.name!='CHECKSUMS.json'}
(out/'CHECKSUMS.json').write_text(json.dumps(checks,indent=2)+'\n')
assert all(sha(out/n)==h for n,h in checks.items())
print('CHECKSUMS SHA256 '+sha(out/'CHECKSUMS.json'),flush=True)
