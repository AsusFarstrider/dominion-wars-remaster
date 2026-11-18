"""Freeze the adaptive research component without touching any installation."""
import hashlib,json,shutil,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent
base=Path('E:/Codex/research/dominion-rendering');parent=base/'esc-menu-perf-source-v1';work=base/'dynamic-output-work-01';out=base/'dynamic-output-source-v1'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists()
assert sha(parent/'qfr-source.json')=='e0933cd5271c5f74963c65771e7ad9abf3866a1a0aa112889e48c5f53f5e20d4'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files'];assert len(pins)==464
assert all(sha(parent/r)==h for r,h in pins.items())
new=['DDrawCompat/Remaster/'+n for n in ('OutputMode.h','OutputSites.h','OutputMode.cpp')]
changed=[r for r,h in pins.items() if sha(work/r)!=h]
assert len(changed)==8,changed
validation=load(work/'adaptive-validation/results.json')
assert validation['wrapper_sha256']==sha(work/'Release/ddraw.dll')
log=(work/'adaptive-validation/wrapper-clean-build.log').read_text()
assert 'error ' not in log.lower() and 'warning ' not in log.lower()
assert any(r['name']=='wrapper-clean-build' and r['exit_code']==0 for r in validation['results'])
exports=lambda p:[(x.ordinal,x.name) for x in pefile.PE(str(p)).DIRECTORY_ENTRY_EXPORT.symbols]
assert exports(work/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll')
for r in [*pins,*new]:
 p=out/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,p)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-esc-menu-perf-source-v1.json')
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll')
shutil.copytree(work/'adaptive-validation',out/'adaptive-validation')
shutil.copytree(here,out/'adaptive-validation-sources',ignore=shutil.ignore_patterns('__pycache__'))
launcher=Path('startup-research/adaptive-display-v1-source');receipt=load(launcher/'source-receipt.json')
for r in receipt['source_files']:assert sha(launcher/r['path'])==r['sha256'],r
shutil.copytree(launcher,out/'launcher-source',ignore=shutil.ignore_patterns('__pycache__'))
contracts=out/'adaptive-contract-snapshots';contracts.mkdir()
for p in [Path('E:/Codex/research/quantum-engine-kb/docs')/n for n in ('LAUNCH-DIMENSION-BINDING-REVIEW-01.md','DYNAMIC-OUTPUT-SOURCE-REVIEW-01.md')]+[Path('E:/Codex/research/dominion-resolution/RUNTIME-OUTPUT-GENERALIZATION-10.md')]:shutil.copyfile(p,contracts/p.name)
engine=base/'native-engine-800x600-01';assert sha(engine/'ds9dw.exe')=='e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023'
(out/'engine-recipe').mkdir();shutil.copyfile(engine/'engine.json',out/'engine-recipe/engine.json')
m['opening_flow']['source_files']={r:sha(out/r) for r in [*pins,*new]}
m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll')
m['opening_flow']['work_source']=str(work)
m['adaptive_output']=dict(schema='quantum-adaptive-output-source-1',status='offline validated; real cold startup and alternate display acceptance pending',baseline='int-20261029.1',parent_manifest_sha256=sha(parent/'qfr-source.json'),changed=changed,added=new,wrapper_sha256=sha(out/'Release/ddraw.dll'),engine_sha256=sha(engine/'ds9dw.exe'),launcher_receipt_sha256=sha(out/'launcher-source/source-receipt.json'),validation_sha256=sha(out/'adaptive-validation/results.json'),exports_match_parent=True,scope='One launch-selected physical primary mode at origin0, no disk engine rewrite, no live resize or secondary routing; retain accepted ESC/Preferences fixes. Source-sized HD history fitted into native target.',limitations=['Simulated targets and relocated startup fixture are not physical hardware acceptance.','Source2560x1440 HD movie identities remain asset requirements, not output constants.','Prior source manifest sections are historical evidence; this adaptive_output section defines the current delta.','Source snapshot is private research, not a public package or integration acceptance.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
record=dict(source=str(out),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=sha(out/'Release/ddraw.dll'),engine_sha256=sha(engine/'ds9dw.exe'),changed=changed,added=new)
(work/'frozen-component.json').write_text(json.dumps(record,indent=2)+'\n');print(json.dumps(record,indent=2))
