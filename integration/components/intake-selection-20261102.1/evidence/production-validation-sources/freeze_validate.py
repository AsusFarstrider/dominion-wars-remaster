"""Freeze an offline integration component; no package identity/game/Current edit."""
import hashlib,json,shutil,subprocess,sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent;workspace=here.parents[1];base=Path('E:/Codex/research/dominion-rendering')
parent=base/'gizmo-main-source-v5';work=base/'production-runtime-work-01';out=base/'production-runtime-source-v1'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists();assert sha(parent/'qfr-source.json')=='5f375031db2b6c190c9f4bb4485997bbcb2bf90bc1a379582c29b3adf4ba7c5f'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files'];assert all(sha(parent/r)==h for r,h in pins.items())
added=sorted('DDrawCompat/Remaster/'+p.name for p in (work/'DDrawCompat/Remaster').iterdir() if p.suffix in ('.h','.cpp'))
assert len(added)==8
changed=sorted(r for r,h in pins.items() if sha(work/r)!=h);assert len(changed)==12,changed
log=work/'build-production-06.log';assert all(x in log.read_text(encoding='utf-8-sig') for x in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
files=sorted(set(pins)|set(added))
for r in files:d=out/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,d)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-gizmo-main-source.json')
tests=out/'production-validation-sources';tests.mkdir()
for p in here.rglob('*'):
 if p.is_file() and p.suffix in ('.h','.cpp','.vcxproj','.py','.md'):
  dest=tests/p.relative_to(here);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dest)
for name in ('CanvasRuntimeTests.cpp','CanvasRuntimeTests.vcxproj','RuntimeTestMemory.h'):
 shutil.copyfile(here/'canvas-tests'/name,out/'validation-sources'/name)
contracts=out/'production-contract-snapshots';contracts.mkdir()
paths=[Path('E:/Codex/research/quantum-engine-kb/docs/MAIN-BUTTONS-AND-SSLOGO-CONTRACT-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/PRODUCTION-LABEL-STARTUP-REVIEW-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/PRODUCTION-ORDERED-MAIN-REVIEW-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/PRODUCTION-ORDERED-MAIN-REVIEW-02.md'),
 workspace/'versioning/identity.schema.json',base/'main-button-assets-v7/assets.json',
 workspace/'cinematic-extraction/original-ss-hd-v1/asset-report.json',workspace/'cinematic-extraction/original-ss-hd-v1/history-receipt.json']
for p in paths:shutil.copyfile(p,contracts/p.name)
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
entries={r['path']:r for r in m['changes']}
for r in changed+added:
 if r not in entries:entries[r]=dict(path=r,before=pins.get(r))
 entries[r]['after']=sha(out/r)
m['changes']=[entries[r] for r in sorted(entries)];m['opening_flow']['source_files']={r:sha(out/r) for r in files}
m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['ui_canvas']['sources']={n:sha(out/'DDrawCompat/Research/MenuCanvas'/n) for n in m['ui_canvas']['sources']}
m['ui_canvas']['validation_sources']={n:sha(out/'validation-sources'/n) for n in m['ui_canvas']['validation_sources']}
m['production_runtime']=dict(schema='quantum-production-runtime-source-1',status='offline-validated integration intake; live/package-bound acceptance pending',
 parent_manifest_sha256=sha(parent/'qfr-source.json'),source_changes=changed+added,source_files=len(files),
 runtime_config='Remaster.Rendering.json;12exactkeys;profile native-1440-v1;version_sha256 binds root version.json;recording off default',
 required_functional_mask=63,recording_independent=True,game_launched=False,current_modified=False,real_package_identity_tested=False,
 inherited_sections_policy='Predecessor component evidence retained; current full production pins are opening_flow.source_files and current validation is production_runtime/validation/results.json.',
 ss_asset_sha256='910eb6d55755c72c67a557138634fc344fff25560cba7e267899ed9a9904bc50',
 buttons_manifest_sha256=sha(base/'main-button-assets-v7/assets.json'),buttons_cached_bytes=241920960,
 contract_sources=[dict(path=str(p),sha256=sha(p)) for p in paths],
 test_sources={p.relative_to(tests).as_posix():sha(p) for p in tests.rglob('*') if p.is_file()},
 limitations=['No new game/codec/audio/presentation or hardware pass; final version identity belongs to Master package.',
 'Only exact native1440engine/profile/threeHDmovies/Main/cards/v7buttons; original assets core and explicit safe fallbacks.',
 'Offline fixtures use synthetic identity, mock original blits/COM and real converted button-bank data; not package acceptance.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n');identity=sha(out/'qfr-source.json')
msbuild='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe'
specs=[('RuntimePolicyTests','RuntimePolicyTests.vcxproj',51),('TransitionRuntimeTests','TransitionRuntimeTests.vcxproj',7),
 ('OrderedPaintTests','OrderedPaintTests.vcxproj',239),('OpeningControllerTests','opening-tests/OpeningControllerTests.vcxproj',384),
 ('CanvasRuntimeTests','canvas-tests/CanvasRuntimeTests.vcxproj',391),('HudAssemblyTests','hud-tests/HudAssemblyTests.vcxproj',864)]
jobs=[]
for name,project,count in specs:
 dest=out/'validation'/name
 command=[msbuild,str(tests/project),'/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(out/'DDrawCompat'),
  '/p:CanvasContractInclude='+str(out/'DDrawCompat/Research/MenuCanvas'),'/p:OutDir='+str(dest/'bin')+'\\','/p:IntDir='+str(dest/'obj')+'\\','/v:minimal','/nologo']
 build=subprocess.run(command,capture_output=True,text=True,timeout=180);dest.mkdir(parents=True,exist_ok=True);(dest/'build.log').write_text(build.stdout+build.stderr)
 assert build.returncode==0,build.stdout+build.stderr
 jobs.append((name,dest/'bin'/(name+'.exe'),count,[str(base/'main-button-assets-v7')] if name=='OrderedPaintTests' else []))
for old in load(parent/'validation/results.json')['tests']:
 if old['name'] in ('Canvas565Tests','PredicateBridgeTests','PublicationStateTests','TransitionStateTests'):
  binary=Path(old['binary']);assert sha(binary)==old['binary_sha256'];dest=out/'validation'/old['name']/'bin';dest.mkdir(parents=True,exist_ok=True);shutil.copyfile(binary,dest/binary.name)
  jobs.append((old['name'],dest/binary.name,old['expected_assertions'],[]))
def run(item):
 name,binary,count,args=item;r=subprocess.run([str(binary)]+args,capture_output=True,text=True,timeout=120)
 return dict(name=name,binary=str(binary),binary_sha256=sha(binary),expected_assertions=count,arguments=args,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=r.returncode==0 and r.stdout.startswith('PASS '+str(count)+' '))
with ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(run,jobs))
result=dict(schema='quantum-production-runtime-preflight-1',source_manifest_sha256=identity,source_files_verified=len(files),tests=results,
 test_policy='All six changed/new controller/provider/dependency suites rebuilt against frozen source; four unchanged pure dependency binaries SHA-verified and rerun.',game_launched=False,passed=all(r['passed'] for r in results))
(out/'validation/results.json').write_text(json.dumps(result,indent=2)+'\n')
exports=lambda p:[(x.ordinal,x.name) for x in pefile.PE(str(p)).DIRECTORY_ENTRY_EXPORT.symbols]
checks=dict(schema='quantum-production-runtime-build-checks-1',source_manifest_sha256=identity,wrapper_sha256=sha(out/'Release/ddraw.dll'),
 build_log_sha256=sha(log),clean_build=True,exports_match_parent=exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll'),
 results_sha256=sha(out/'validation/results.json'),source_files_verified=len(files),changed=changed,added=added,game_launched=False,passed=result['passed'])
checks['passed']=checks['passed'] and checks['exports_match_parent'];(out/'validation/opening-build-checks.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=identity,wrapper_sha256=checks['wrapper_sha256'],passed=checks['passed'],tests=[dict(name=r['name'],stdout=r['stdout'],passed=r['passed']) for r in results]),indent=2));assert checks['passed']
