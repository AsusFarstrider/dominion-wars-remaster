"""Freeze the Gizmo/Main candidate and rebuild its changed controller dependencies."""
import hashlib,json,shutil,subprocess,sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent
base=Path('E:/Codex/research/dominion-rendering');parent=base/'opening-cards-source-v1';work=base/'gizmo-main-work-01';out=base/'gizmo-main-source-v5'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists(),'Use a new frozen tree'
assert sha(parent/'qfr-source.json')=='5f154f7d058b2795a9645577581aa190cb781e3d5ea195ded241f68be8d07443'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files']
assert all(sha(parent/r)==h for r,h in pins.items())
changed=sorted(r for r,h in pins.items() if sha(work/r)!=h)
assert changed==['DDrawCompat/Research/MenuCanvas/Canvas565.h','DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp',
 'DDrawCompat/Research/MenuProbe/Probe.cpp','DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp','DDrawCompat/Research/OpeningFlow/OpeningFlow.h'],changed
added=['DDrawCompat/Research/MenuCanvas/ForegroundMask.h','DDrawCompat/Research/MenuCanvas/MainBackdrop.h']
log=work/'build-gizmo-main-07.log';assert all(x in log.read_text(encoding='utf-8-sig') for x in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
files=sorted(set(pins)|set(added))
for r in files:
 d=out/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,d)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-opening-cards-source.json')
contracts=out/'gizmo-contract-snapshots';contracts.mkdir()
paths=[Path('E:/Codex/research/quantum-engine-kb/docs/GIZMO-AND-MAIN-NATIVE-CONTRACT-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/MAIN-FOREGROUND-COVERAGE-ADDENDUM-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/MAIN-BACKGROUND-SOURCE-REVIEW-02.md'),
 Path('E:/Codex/research/quantum-engine-kb/docs/MAIN-FLAGS13-CONTRACT-01.md'),
 Path('E:/Codex/research/quantum-engine-kb/validation/gizmo-main-contract-01-manifest.json'),
 here.parents[1]/'cinematic-extraction/original-gizmo-hd-v1/asset-report.json',
 here.parents[1]/'cinematic-extraction/original-gizmo-hd-v1/history-receipt.json',base/'main-background-assets-v1/assets.json']
rows=[]
for p in paths:shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
tests=out/'gizmo-validation-sources';tests.mkdir()
for p in here.rglob('*'):
 if p.is_file() and p.suffix in ('.h','.cpp','.vcxproj','.py','.ps1'):
  dest=tests/p.relative_to(here);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dest)
for name in ('CanvasRuntimeTests.cpp','CanvasRuntimeTests.vcxproj'):
 shutil.copyfile(here/'canvas-tests'/name,out/'validation-sources'/name)
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
entries={r['path']:r for r in m['changes']}
for r in changed+added:
 if r not in entries:entries[r]=dict(path=r,before=pins.get(r))
 entries[r]['after']=sha(out/r)
m['changes']=[entries[r] for r in sorted(entries)]
m['opening_flow']['source_files']={r:sha(out/r) for r in files}
m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['ui_canvas']['sources']={n:sha(out/'DDrawCompat/Research/MenuCanvas'/n) for n in m['ui_canvas']['sources']}
m['ui_canvas']['validation_sources']={n:sha(out/'validation-sources'/n) for n in m['ui_canvas']['validation_sources']}
m['ui_canvas']['expected_assertions']['CanvasRuntimeTests']=391
m['gizmo_main']=dict(schema='quantum-gizmo-main-source-1',parent_manifest_sha256=sha(parent/'qfr-source.json'),
 opt_in='QFR_MAIN_BACKGROUND=1 plus unchanged HD/card/UI/HUD parent opt-ins',
 profiles=[dict(resource='5C6',frames=601,fps=[2997,100],source=[2560,1440],original_xy=[0,0]),dict(resource='201',role='native background beneath independently covered logical foreground')],
 expected_controller_assertions=336,contracts=rows,source_changes=changed+added,
 test_sources={p.relative_to(tests).as_posix():sha(p) for p in tests.rglob('*') if p.is_file()},game_launched=False,
 limitations=['Only two separately identified HD movie profiles and Main background; no arbitrary replacements.',
 'Live Gizmo audio/end/skip, Main foreground/modal behavior remain unverified.',
 'Pure/mock controller tests do not establish real presentation or hardware compatibility.',
 'Source/draw operation coverage is separate from visual color and input hitboxes.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n');identity=sha(out/'qfr-source.json')
jobs=[]
for old in load(parent/'validation/results.json')['tests']:
 if old['name'] in ('OpeningControllerTests','HudAssemblyTests'):continue
 name=old['name'];folder=out/'validation-sources'
 command=['C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe',str(folder/(name+'.vcxproj')),
  '/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(out/'DDrawCompat'),
  '/p:CanvasContractInclude='+str(out/'DDrawCompat/Research/MenuCanvas'),'/p:CanvasCoreInclude='+str(out/'DDrawCompat/Research/MenuCanvas'),
  '/p:OutDir='+str(out/'validation'/name/'bin')+'\\','/p:IntDir='+str(out/'validation'/name/'obj')+'\\','/v:minimal','/nologo']
 build=subprocess.run(command,capture_output=True,text=True,timeout=120)
 logpath=out/'validation'/(name+'-build.log');logpath.parent.mkdir(parents=True,exist_ok=True);logpath.write_text(build.stdout+build.stderr)
 assert build.returncode==0,build.stdout+build.stderr
 jobs.append((name,out/'validation'/name/'bin'/(name+'.exe'),m['ui_canvas']['expected_assertions'][name]))
jobs += [('OpeningControllerTests',base/'gizmo-main-tests-01/bin/OpeningControllerTests.exe',336),
         ('HudAssemblyTests',base/'hud-assembly-tests-01/bin/HudAssemblyTests.exe',864)]
def run(item):
 name,binary,count=item;r=subprocess.run([str(binary)],capture_output=True,text=True,timeout=90)
 return dict(name=name,binary=str(binary),binary_sha256=sha(binary),expected_assertions=count,exit_code=r.returncode,
             stdout=r.stdout,stderr=r.stderr,passed=r.returncode==0 and r.stdout.startswith('PASS '+str(count)+' '))
with ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(run,jobs))
v=out/'validation';v.mkdir(exist_ok=True)
for r in results:
 d=v/r['name']/'bin';d.mkdir(parents=True,exist_ok=True)
 if Path(r['binary']).resolve()!=(d/Path(r['binary']).name).resolve():shutil.copyfile(r['binary'],d/Path(r['binary']).name)
result=dict(schema='quantum-gizmo-main-preflight-1',source_manifest_sha256=identity,source_files_verified=len(files),
 tests=results,retained_test_policy='Changed UI controller/core dependency fixtures rebuilt; modified OpeningFlow compiled into new336-check fixture; unchanged HUD binary rerun with recorded pin.',
 recorder='Unchanged pinned recorder implementation; prior normal/abrupt fixture evidence retained via parent manifest, not rerun here.',
 game_launched=False,passed=all(r['passed'] for r in results))
(v/'results.json').write_text(json.dumps(result,indent=2)+'\n')
def exports(path):return [(e.ordinal,e.name) for e in pefile.PE(str(path)).DIRECTORY_ENTRY_EXPORT.symbols]
checks=dict(schema='quantum-gizmo-main-build-checks-1',source_manifest_sha256=identity,wrapper_sha256=sha(out/'Release/ddraw.dll'),
 build_log_sha256=sha(log),clean_build=True,exports_match_parent=exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll'),
 results_sha256=sha(v/'results.json'),source_files_verified=len(files),changed=changed,added=added,game_launched=False,
 passed=result['passed'] and exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll'))
(v/'opening-build-checks.json').write_text(json.dumps(checks,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=identity,wrapper_sha256=checks['wrapper_sha256'],passed=checks['passed'],tests=[dict(name=r['name'],stdout=r['stdout'],passed=r['passed']) for r in results]),indent=2))
assert checks['passed']
