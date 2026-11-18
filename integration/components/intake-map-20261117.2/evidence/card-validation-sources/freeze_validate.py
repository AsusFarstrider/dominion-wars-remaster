"""Freeze native opening cards on the tested HD parent and run relevant checks."""
import hashlib,json,shutil,subprocess,sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent
base=Path('E:/Codex/research/dominion-rendering');parent=base/'hd-video-source-v1';work=base/'opening-cards-work-01';out=base/'opening-cards-source-v1'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists(),'Use a new frozen tree'
assert sha(parent/'qfr-source.json')=='f49c0d39864bd461bfa43b459c3201ba68f855d1f08804b2e5d8957cae0bf1e3'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files']
assert all(sha(parent/r)==h for r,h in pins.items())
changed=sorted(r for r,h in pins.items() if sha(work/r)!=h)
assert changed==['DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp'],changed
added=['DDrawCompat/Research/OpeningFlow/CardImage.h']
log=work/'build-cards-02.log';assert all(x in log.read_text(encoding='utf-8-sig') for x in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
files=sorted(set(pins)|set(added))
for r in files:
 d=out/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,d)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-hd-video-source.json')
contracts=out/'card-contract-snapshots';contracts.mkdir()
paths=[Path('E:/Codex/research/quantum-engine-kb/docs/OPENING-CARD-REPLACEMENT-CONTRACT-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/validation/opening-card-contract-01-manifest.json'),
 here.parents[1]/'cinematic-extraction/remaining-opening-v1/inventory.json',base/'opening-card-assets-v1/assets.json']
rows=[]
for p in paths:shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
tests=out/'card-validation-sources';tests.mkdir()
for p in here.iterdir():
 if p.is_file() and p.suffix in ('.h','.cpp','.vcxproj','.py','.ps1'):shutil.copyfile(p,tests/p.name)
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
entries={r['path']:r for r in m['changes']}
for r in changed+added:
 if r not in entries:entries[r]=dict(path=r,before=pins.get(r))
 entries[r]['after']=sha(out/r)
m['changes']=[entries[r] for r in sorted(entries)]
m['opening_flow']['source_files']={r:sha(out/r) for r in files}
m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['opening_cards']=dict(schema='quantum-opening-cards-source-1',parent_manifest_sha256=sha(parent/'qfr-source.json'),
 opt_in='QFR_OPENING_CARDS=1 plus unchanged HD/movie/UI/HUD parent opt-ins',
 profiles=[dict(state=5,resource='101',name='publisher'),dict(state=6,resource='103',name='title')],
 expected_controller_assertions=293,contracts=rows,source_changes=changed+added,
 test_sources={p.name:sha(p) for p in tests.iterdir()},game_launched=False,
 limitations=['Only two verified card owners/resources; private supplied artwork is not source-bundled.',
 'Live card appearance/timing/input/transitions remain unverified.',
 'Pure/mock controller tests do not establish real presentation or hardware compatibility.',
 'Unchanged HD movie history/provider is inherited from the tested parent.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n');identity=sha(out/'qfr-source.json')
jobs=[]
for old in load(parent/'validation/results.json')['tests']:
 if old['name'] in ('OpeningControllerTests','HudAssemblyTests'):continue
 binary=Path(old['binary']);assert sha(binary)==old['binary_sha256'];jobs.append((old['name'],binary,old['expected_assertions']))
jobs += [('OpeningControllerTests',base/'opening-cards-tests-01/bin/OpeningControllerTests.exe',293),
         ('HudAssemblyTests',base/'hud-assembly-tests-01/bin/HudAssemblyTests.exe',864)]
def run(item):
 name,binary,count=item;r=subprocess.run([str(binary)],capture_output=True,text=True,timeout=90)
 return dict(name=name,binary=str(binary),binary_sha256=sha(binary),expected_assertions=count,exit_code=r.returncode,
             stdout=r.stdout,stderr=r.stderr,passed=r.returncode==0 and r.stdout.startswith('PASS '+str(count)+' '))
with ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(run,jobs))
v=out/'validation';v.mkdir()
for r in results:
 d=v/r['name']/'bin';d.mkdir(parents=True);shutil.copyfile(r['binary'],d/Path(r['binary']).name)
result=dict(schema='quantum-opening-cards-preflight-1',source_manifest_sha256=identity,source_files_verified=len(files),
 tests=results,retained_test_policy='Retained menu/HUD binaries rerun only where production dependencies are unchanged; modified OpeningFlow compiled into the new controller fixture.',
 recorder='Unchanged pinned recorder implementation; prior normal/abrupt fixture evidence retained via parent manifest, not rerun here.',
 game_launched=False,passed=all(r['passed'] for r in results))
(v/'results.json').write_text(json.dumps(result,indent=2)+'\n')
def exports(path):return [(e.ordinal,e.name) for e in pefile.PE(str(path)).DIRECTORY_ENTRY_EXPORT.symbols]
checks=dict(schema='quantum-opening-cards-build-checks-1',source_manifest_sha256=identity,wrapper_sha256=sha(out/'Release/ddraw.dll'),
 build_log_sha256=sha(log),clean_build=True,exports_match_parent=exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll'),
 results_sha256=sha(v/'results.json'),source_files_verified=len(files),changed=changed,added=added,game_launched=False,
 passed=result['passed'] and exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll'))
(v/'opening-build-checks.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=identity,wrapper_sha256=checks['wrapper_sha256'],passed=checks['passed'],tests=[dict(name=r['name'],stdout=r['stdout'],passed=r['passed']) for r in results]),indent=2))
assert checks['passed']
