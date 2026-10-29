"""Freeze a three-file child, with actual install-path positive/negative controls."""
import hashlib,json,shutil,subprocess,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent;workspace=here.parents[2]
base=Path('E:/Codex/research/dominion-rendering')
parent=base/'production-runtime-source-v1';work=base/'production-runtime-work-02';out=base/'production-runtime-source-v2'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists()
assert sha(parent/'qfr-source.json')=='4812ccb9c32034f1d5129b7525a0e5c57010927761ca01f23124657233b9e23e'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files']
assert all(sha(parent/r)==h for r,h in pins.items())
changed=sorted(r for r,h in pins.items() if sha(work/r)!=h)
assert changed==['DDrawCompat/Common/VtableHookVisitor.h','DDrawCompat/Remaster/Runtime.cpp','DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp'],changed
log=work/'build-production-09.log'
assert all(s in log.read_text(encoding='utf-8-sig') for s in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
gizmo=workspace/'cinematic-extraction/original-gizmo-hd-v5'
assert sha(gizmo/'GizmoLogo-1440p-original-audio.bik')=='b506781006eddd97fe6f86108d67259639f8044cbd5bcf71fcd0734a14013688'
for r in pins:
 p=out/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,p)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-production-runtime-v1.json')
tests=out/'gpu-hook-validation-sources';shutil.copytree(here,tests,ignore=shutil.ignore_patterns('__pycache__'))
contracts=out/'gpu-hook-contract-snapshots';contracts.mkdir()
for name in ('HANDOFF.md','asset-report.json','preparation.json'):shutil.copyfile(gizmo/name,contracts/('gizmo-v5-'+name))
engine=Path('E:/Codex/research/quantum-engine-kb/docs/PRODUCTION-RECORDER-DEPENDENCY-DIAGNOSIS-01.md')
shutil.copyfile(engine,contracts/engine.name)
review=engine.parent/'GPU-VISITOR-OFF-REVIEW-01.md'
shutil.copyfile(review,contracts/review.name)
for name in ('build-production-07.log','build-production-08.log','build-production-09.log','prepare-receipt.json'):shutil.copyfile(work/name,out/name)
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll')
msbuild='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe'
results=[]
def test(name,project,include,expected,parent_failure=False):
 dest=out/'validation'/name;dest.mkdir(parents=True)
 command=[msbuild,str(project),'/t:Rebuild','/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(include/'DDrawCompat'),'/p:OutDir='+str(dest/'bin')+'\\','/p:IntDir='+str(dest/'obj')+'\\','/v:minimal','/nologo']
 b=subprocess.run(command,capture_output=True,text=True,timeout=180);(dest/'build.log').write_text(b.stdout+b.stderr)
 assert b.returncode==0,b.stdout+b.stderr
 binary=dest/'bin'/(project.stem+'.exe');r=subprocess.run([str(binary)],capture_output=True,text=True,timeout=120)
 passed=(r.returncode==1 and r.stdout=='' and r.stderr.strip()=='FAIL three functional slots installed at INFO/record OFF and diagnostics modes') if parent_failure else (r.returncode==0 and r.stdout.startswith('PASS '+str(expected)+' '))
 result=dict(name=name,source=str(include),command=command,binary=str(binary),binary_sha256=sha(binary),exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr,expected_assertions=expected,expected_parent_failure=parent_failure,passed=passed)
 results.append(result);print(json.dumps(result),flush=True);assert passed
test('GpuVisitorTests',tests/'GpuVisitorTests.vcxproj',out,90)
test('GpuVisitorParentNegative',tests/'GpuVisitorTests.vcxproj',parent,None,True)
for name,rel,count in [('HudAssemblyTests','hud-tests/HudAssemblyTests.vcxproj',864),('RuntimePolicyTests','RuntimePolicyTests.vcxproj',51),('OpeningControllerTests','opening-tests/OpeningControllerTests.vcxproj',384)]:
 test(name,out/'production-validation-sources'/rel,out,count)
exports=lambda p:[(s.ordinal,s.name) for s in pefile.PE(str(p)).DIRECTORY_ENTRY_EXPORT.symbols]
assert exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll')
current=workspace/'integration/current.json';current_sha=sha(current)
result=dict(schema='quantum-gpu-hook-fix-validation-1',tests=results,clean_build=True,build_log_sha256=sha(out/log.name),exports_match_parent=True,
 source_files_verified=len(pins),changed=changed,wrapper_sha256=sha(out/'Release/ddraw.dll'),game_launched=False,current_modified=False,current_json_sha256=current_sha,passed=True)
(out/'validation/results.json').write_text(json.dumps(result,indent=2)+'\n')
entries={r['path']:r for r in m['changes']}
for r in changed:
 if r not in entries:entries[r]=dict(path=r,before=pins[r])
 entries[r]['after']=sha(out/r)
m['changes']=[entries[r] for r in sorted(entries)];m['opening_flow']['source_files']={r:sha(out/r) for r in pins}
m['opening_flow']['wrapper_sha256']=result['wrapper_sha256'];m['opening_flow']['work_source']=str(work)
m['production_runtime']['status']='Superseded failed v1 offline component; see gpu_hook_fix and validation/results.json for current child evidence.'
m['gpu_hook_fix']=dict(schema='quantum-production-runtime-source-2',status='offline validated; recording-off managed mission retest pending',parent=str(parent),parent_manifest_sha256=sha(parent/'qfr-source.json'),
 source_changes=changed,source_files=len(pins),wrapper_sha256=result['wrapper_sha256'],validation_sha256=sha(out/'validation/results.json'),
 test_sources={p.relative_to(tests).as_posix():sha(p) for p in tests.rglob('*') if p.is_file()},
 contract_snapshots={p.name:sha(p) for p in contracts.iterdir()},gizmo_v5=dict(path=str(gizmo/'GizmoLogo-1440p-original-audio.bik'),sha256=sha(gizmo/'GizmoLogo-1440p-original-audio.bik'),size=73633988,resource='5C6',width=2560,height=1440,frames=601,fps_n=2997,fps_d=100),
 readiness='63 is engine-hook readiness before lazy COM object creation; actual vtable install/dispatch is independently regression tested.',
 scope='Exact three GPU hook installation exemptions plus new Gizmo v5 allowlist hash at two existing sites. No adapter/math/layout/input/recording policy changes.',
 limitations=['Parent production v1 failed live with recording off; recorder-on A/B restored expected output. Preserve both.',
 'Fixture executes actual installation/enumeration/dispatch with logging/recorder/probe and adapter callees stubbed; real HUD adapter math covered separately.',
 'Exact black-world clear/order mechanism remains unproved; actual repaired recording-off mission must pass before integration.',
 'New Gizmo v5 reused established format and supplier packet/audio receipts; older v3 codec guard evidence is not attributed to v5.',
 'No game launched, package built, identity changed, Current modified, or publication performed.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=result['wrapper_sha256'],passed=True),indent=2))
