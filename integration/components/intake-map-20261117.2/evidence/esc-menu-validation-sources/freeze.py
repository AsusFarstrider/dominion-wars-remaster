import hashlib,json,shutil,subprocess,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
here=Path(__file__).resolve().parent;base=Path('E:/Codex/research/dominion-rendering')
parent=base/'production-runtime-source-v2';work=base/'esc-main-menu-work-01';out=base/'esc-main-menu-source-v1'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
assert not out.exists();assert sha(parent/'qfr-source.json')=='c992873dcabf1047b196d4fbf708f3bcae046506c7413318ed9fea12c7944b84'
m=load(parent/'qfr-source.json');pins=m['opening_flow']['source_files']
assert all(sha(parent/r)==h for r,h in pins.items())
changed=[r for r,h in pins.items() if sha(work/r)!=h]
assert changed==['DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp'],changed
log=work/'build-esc-menu-01.log';assert all(x in log.read_text(encoding='utf-8-sig') for x in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
for r in pins:
 p=out/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/r,p)
for p in parent.iterdir():
 if p.is_dir() and (p.name.endswith('sources') or p.name.endswith('snapshots')):shutil.copytree(p,out/p.name)
 elif p.is_file() and p.name.startswith('parent-'):shutil.copyfile(p,out/p.name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-production-runtime-v2.json')
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
tests=out/'esc-menu-validation-sources';shutil.copytree(here,tests,ignore=shutil.ignore_patterns('__pycache__'))
contracts=out/'esc-menu-contract-snapshots';contracts.mkdir()
engine=Path('E:/Codex/research/quantum-engine-kb/docs/ESC-MAIN-REENTRY-CONTRACT-01.md');shutil.copyfile(engine,contracts/engine.name)
capture=base/'esc-menu-state-01'
for name in ('events.ndjson','audit.json'):shutil.copyfile(capture/name,contracts/name)
ms='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe';results=[]
for name,source,expected in [('EscMenuTests',out,0),('ParentNegative',parent,1)]:
 dest=out/'validation'/name;dest.mkdir(parents=True)
 cmd=[ms,str(tests/'tests/EscMenuTests.vcxproj'),'/t:Rebuild','/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(source/'DDrawCompat'),'/p:CanvasContractInclude='+str(source/'DDrawCompat/Research/MenuCanvas'),'/p:OutDir='+str(dest/'bin')+'\\','/p:IntDir='+str(dest/'obj')+'\\','/v:minimal','/nologo']
 b=subprocess.run(cmd,capture_output=True,text=True,timeout=180);(dest/'build.log').write_text(b.stdout+b.stderr);assert b.returncode==0,b.stdout+b.stderr
 binary=dest/'bin/EscMenuTests.exe';r=subprocess.run([str(binary)],capture_output=True,text=True,timeout=90)
 assert r.returncode==expected and (r.stdout.startswith('PASS 480 ') if not expected else r.stderr.strip()=='FAIL observed re-entry default filter admits exact original Main family')
 results.append(dict(name=name,command=cmd,source=str(source),binary_sha256=sha(binary),exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr,expected_parent_failure=bool(expected),passed=True))
exports=lambda p:[(s.ordinal,s.name) for s in pefile.PE(str(p)).DIRECTORY_ENTRY_EXPORT.symbols]
assert exports(out/'Release/ddraw.dll')==exports(parent/'Release/ddraw.dll')
validation=dict(schema='quantum-esc-reentry-validation-1',tests=results,clean_build=True,build_log_sha256=sha(out/log.name),exports_match_parent=True,source_files_verified=len(pins),game_launched_with_child=False,passed=True)
(out/'validation/results.json').write_text(json.dumps(validation,indent=2)+'\n')
for r in m['changes']:
 if r['path'] in changed:r['after']=sha(out/r['path'])
m['opening_flow']['source_files']={r:sha(out/r) for r in pins};m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['esc_menu_fix']=dict(schema='quantum-esc-reentry-source-1',status='offline validated; corrected live re-entry test pending',parent_manifest_sha256=sha(parent/'qfr-source.json'),baseline='int-20261029.1',source_changes=changed,wrapper_sha256=sha(out/'Release/ddraw.dll'),validation_sha256=sha(out/'validation/results.json'),
 test_sources={p.relative_to(tests).as_posix():sha(p) for p in tests.rglob('*') if p.is_file()},contract_snapshots={p.name:sha(p) for p in contracts.iterdir()},
 scope='Keep zero filters supported; admit original47DDC0/post0 only for exact existing Main/Selection families with IDs>0xA0 and readable command mode718690==0. No callback/global/provenance writes, constructor replacement or changed rendering geometry.',
 evidence='Read-only 184 snapshots,113 stable failed Main samples; forced closure after capture. Fixture480 includes391 inherited checks; exact approved parent negative fails at observed admission.',
 limitations=['External sampler is not per-call/pixel/publication evidence; no internal publication flags or button image/event-mask fields captured.','Live retry and ordinary quit pending. No release/Current change.','Inherited component manifest sections describe parent evidence; current child scope is esc_menu_fix and validation/results.json.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
print(json.dumps(dict(source=str(out),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=sha(out/'Release/ddraw.dll'),passed=True),indent=2))
