import hashlib,json,shutil,subprocess,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent;base=Path('E:/Codex/research/dominion-rendering')
parent=base/'esc-main-menu-source-v1';work=base/'esc-menu-perf-work-01';out=base/'esc-menu-perf-source-v1'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists();assert sha(parent/'qfr-source.json')=='05da2a8831a634e45bfc21b9c65d7e8845b1a327bfaaccfcaace363dafc301bb'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files'];assert all(sha(parent/r)==h for r,h in pins.items())
changed=[r for r,h in pins.items() if sha(work/r)!=h];assert changed==['DDrawCompat/Remaster/OrderedPaint.h']
log=work/'build-esc-perf-01.log';assert all(x in log.read_text(encoding='utf-8-sig') for x in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
for r in pins:
 p=out/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,p)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-esc-menu-source-v1.json')
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
tests=out/'esc-perf-validation-sources';shutil.copytree(here,tests,ignore=shutil.ignore_patterns('__pycache__'))
contracts=out/'esc-perf-contract-snapshots';contracts.mkdir()
engine=Path('E:/Codex/research/quantum-engine-kb/docs')
for name in ('ESC-MAIN-CHILD-SOURCE-REVIEW-03.md','ORDERED-PAINT-AXIS-CACHE-REVIEW-01.md'):shutil.copyfile(engine/name,contracts/name)
ms='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe';results=[]
def run(name,project,src,count=None,args=()):
 dest=out/'validation'/name;dest.mkdir(parents=True)
 cmd=[ms,str(project),'/t:Rebuild','/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(src/'DDrawCompat'),'/p:CanvasContractInclude='+str(src/'DDrawCompat/Research/MenuCanvas'),'/p:OutDir='+str(dest/'bin')+'\\','/p:IntDir='+str(dest/'obj')+'\\','/v:minimal','/nologo']
 b=subprocess.run(cmd,capture_output=True,text=True,timeout=180);(dest/'build.log').write_text(b.stdout+b.stderr);assert b.returncode==0,b.stdout+b.stderr
 binary=dest/'bin'/(project.stem+'.exe');r=subprocess.run([str(binary),*args],capture_output=True,text=True,timeout=120);assert r.returncode==0,r.stdout+r.stderr
 if count:assert r.stdout.startswith('PASS '+str(count)+' ')
 record=dict(name=name,command=cmd,arguments=list(args),source=str(src),binary_sha256=sha(binary),exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=True)
 results.append(record);print(json.dumps(dict(name=name,stdout=r.stdout)),flush=True)
 return json.loads(r.stdout) if count is None else None
run('EscMenuTests',tests/'tests/EscMenuTests.vcxproj',out,480)
run('OrderedMapTests',tests/'OrderedMapTests.vcxproj',out,159)
run('OrderedPaintTests',out/'production-validation-sources/OrderedPaintTests.vcxproj',out,239,[str(base/'main-button-assets-v7')])
run('OpeningControllerTests',out/'production-validation-sources/opening-tests/OpeningControllerTests.vcxproj',out,384)
before=run('SyntheticBefore',tests/'OrderedPerf.vcxproj',parent)
after=run('SyntheticAfter',tests/'OrderedPerf.vcxproj',out)
exports=lambda p:[(s.ordinal,s.name) for s in pefile.PE(str(p)).DIRECTORY_ENTRY_EXPORT.symbols]
assert exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll')
v=dict(schema='quantum-esc-menu-perf-validation-1',tests=results,clean_build=True,build_log_sha256=sha(out/log.name),exports_match_parent=True,source_files_verified=len(pins),synthetic_before=before,synthetic_after=after,gameplay_timing=False,child_launched=False,passed=True)
(out/'validation/results.json').write_text(json.dumps(v,indent=2)+'\n')
for r in m['changes']:
 if r['path'] in changed:r['after']=sha(out/r['path'])
m['opening_flow']['source_files']={r:sha(out/r) for r in pins};m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['esc_menu_perf']=dict(schema='quantum-esc-menu-perf-source-1',status='offline validated; live Preferences performance check pending',parent_manifest_sha256=sha(parent/'qfr-source.json'),source_changes=changed,baseline_changes=['DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp',*changed],wrapper_sha256=sha(out/'Release/ddraw.dll'),validation_sha256=sha(out/'validation/results.json'),
 test_sources={p.relative_to(tests).as_posix():sha(p) for p in tests.rglob('*') if p.is_file()},contract_snapshots={p.name:sha(p) for p in contracts.iterdir()},
 scope='Retain tested ESC return fix; raw ordered replay computes exact existing inverse mapping once per row/column; no sprite filtering/color/alpha/order/layout/input change. Nothrow map allocation failure returns before writes; sprite-only path allocates no maps.',
 limitations=['Synthetic CPU composition timings are not in-game frame timings or broad hardware proof.','159-case output comparison includes simulated dimensions; actual native1440 UI performance still requires user check.','Prior component manifest evidence is historical; current scope esc_menu_perf and validation/results.json.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=sha(out/'Release/ddraw.dll'),passed=True),indent=2))
