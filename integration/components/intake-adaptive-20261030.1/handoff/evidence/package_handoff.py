"""Private integration intake; neither publish nor update Current."""
import difflib,hashlib,json,shutil
from pathlib import Path
here=Path(__file__).resolve().parent;root=here.parents[1]
base=Path('E:/Codex/research/dominion-rendering');source=base/'dynamic-output-source-v1';approved=base/'production-runtime-source-v2'
out=root/'handoffs/master/rendering-adaptive-output-20261030';assert not out.exists()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert sha(source/'qfr-source.json')=='9dd1b04e08ebaffa6fbb2e39bbe22ddd0eed7866612c8a68795c01ef537f7af9'
assert sha(approved/'qfr-source.json')=='c992873dcabf1047b196d4fbf708f3bcae046506c7413318ed9fea12c7944b84'
m=load(source/'qfr-source.json');pins=m['opening_flow']['source_files'];old=load(approved/'qfr-source.json')['opening_flow']['source_files']
assert all(sha(source/r)==h for r,h in pins.items());assert all(sha(approved/r)==h for r,h in old.items())
a=load(here/'live-2560x1440-01/acceptance.json');b=load(here/'live-1920x1080-01/acceptance.json')
assert a['fingerprint']==b['fingerprint'];assert not a['selected_ledger_failures'] and not b['selected_ledger_failures']
out.mkdir(parents=True);changed=[r for r,h in pins.items() if r not in old or h!=old[r]];assert len(changed)==13,changed
patch=[]
for r in changed:
 before=(approved/r).read_bytes().decode().splitlines(keepends=True) if r in old else []
 patch.extend(difflib.unified_diff(before,(source/r).read_bytes().decode().splitlines(keepends=True),fromfile='a/'+r if r in old else '/dev/null',tofile='b/'+r))
(out/'provider-source.patch').write_bytes(''.join(patch).encode())
launcher=here/'launcher-v2-snapshot';runtime=load(launcher/'source-receipt.json')['runtime'];patch=[]
for n,h in runtime.items():
 assert sha(launcher/'runtime'/n)==h
 before=Path('E:/Codex/remaster-builds/playable-int-20261029.1')/n
 patch.extend(difflib.unified_diff(before.read_bytes().decode().splitlines(keepends=True),(launcher/'runtime'/n).read_bytes().decode().splitlines(keepends=True),fromfile='a/runtime/'+n,tofile='b/runtime/'+n))
(out/'launcher-source.patch').write_bytes(''.join(patch).encode())
shutil.copytree(launcher,out/'launcher-source');shutil.copyfile(here/'HANDOFF.md',out/'README.md')
evidence=out/'evidence';evidence.mkdir()
for n in ('live-2560x1440-01','live-1920x1080-01','failed-preflight-01'):shutil.copytree(here/n,evidence/n)
for p in here.glob('*.py'):shutil.copyfile(p,evidence/p.name)
for p in here.glob('*.ps1'):shutil.copyfile(p,evidence/p.name)
for n in ('sampler-provenance.json','fixture-provenance.json'):shutil.copyfile(here/n,evidence/n)
shutil.copyfile(root/'rendering-research/DYNAMIC-OUTPUT-PROTOTYPE-01.md',evidence/'investigation.md')
engine={}
for r in ('rendering-research/native-resolution/build_engine.py','integration/components/intake-20261018.1/launcher/build_input_window_v1.py','integration/components/intake-20261018.1/launcher/input-window-patch-v2.json'):
 p=out/'engine-build-provenance'/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(root/r,p);engine[r]=sha(p)
shutil.copyfile(source/'engine-recipe/engine.json',out/'engine-recipe.json')
record=dict(schema='quantum-adaptive-output-intake-1',status='two-primary-resolution prototype human accepted; Master review pending',baseline='int-20261029.1',baseline_runtime_revision='f025d0cfaacaa1c28964792424ee7b77f37ace97',baseline_provider_manifest_sha256=sha(approved/'qfr-source.json'),frozen_source=str(source),frozen_source_manifest_sha256=sha(source/'qfr-source.json'),provider_production_pins=pins,provider_changed_from_baseline=changed,wrapper_sha256=sha(source/'Release/ddraw.dll'),engine_sha256=a['fingerprint']['ds9dw.exe'],engine_reproduction_sources=engine,launcher_runtime=runtime,launcher_receipt_sha256=sha(out/'launcher-source/source-receipt.json'),same_fingerprint_both_live_runs=a['fingerprint'],live_acceptance={n:sha(evidence/n/'acceptance.json') for n in ('live-2560x1440-01','live-1920x1080-01')},dependencies=['Includes accepted ESC return and Preferences performance fixes; supersedes their provider source delta when applied against approved production-runtime-source-v2.','Apply all three adaptive Launcher runtime files with engine/profile/provider atomically; all installed inventory and version/config hashes must be regenerated.','Native-display-v1 must omit fixed manifest W/H, require exact E81, and retain unchanged required asset identities.'],limitations=['Actual hardware tests are two physical monitors on one computer, both primary-origin0/DPI96.','Other aspect ratios, 4K, high DPI, arbitrary secondary origins and live resizing/hotplug are not live accepted.','Metadata samples and selected ledger do not prove every draw/pixel; full recorder was OFF.'],current_modified=False,release_authorized=False)
(out/'component.json').write_text(json.dumps(record,indent=2)+'\n')
checks={p.relative_to(out).as_posix():sha(p) for p in out.rglob('*') if p.is_file()}
(out/'CHECKSUMS.json').write_text(json.dumps(checks,indent=2)+'\n')
assert all(sha(out/n)==h for n,h in checks.items())
print(json.dumps(dict(handoff=str(out),component_sha256=sha(out/'component.json'),checksums_sha256=sha(out/'CHECKSUMS.json'),files=len(checks),provider_delta_files=len(changed)),indent=2))
