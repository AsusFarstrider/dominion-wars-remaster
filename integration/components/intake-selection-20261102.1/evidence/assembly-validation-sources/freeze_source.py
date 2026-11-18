"""Freeze the first combined HUD implementation and source-backed validation."""
import argparse,hashlib,json,shutil,subprocess
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering')
parser=argparse.ArgumentParser();parser.add_argument('--revision',type=int,default=1);parser.add_argument('--build-log',default='build-assembly-02.log');args=parser.parse_args()
parent=base/'hud-captains-source-v2';work=base/'hud-assembly-work-01';out=base/('hud-assembly-source-v'+str(args.revision))
here=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not out.exists(),'Use a new frozen source'
assert sha(parent/'qfr-source.json')=='758cc077fd2739b7f0511eed4e58d936a8d4aa19a435c2ffd8f0c8c7e60ee1a2'
m=json.loads((parent/'qfr-source.json').read_text());pins=m['opening_flow']['source_files']
assert all(sha(parent/r)==h for r,h in pins.items())
changed=sorted(r for r,h in pins.items() if sha(work/r)!=h)
assert changed==['DDrawCompat/Common/VtableHookVisitor.h','DDrawCompat/DDrawCompat.vcxproj',
 'DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp','DDrawCompat/Research/MenuProbe/Probe.cpp',
 'DDrawCompat/Research/MenuProbe/TransitionProbe.cpp','DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp'],changed
added=sorted(p.relative_to(work).as_posix() for p in (work/'DDrawCompat/Research/HudAssembly').iterdir() if p.is_file())
assert len(added)==4,added
log=work/args.build_log;text=log.read_text(encoding='utf-8-sig')
assert all(s in text for s in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
binary=base/'hud-assembly-tests-01/bin/HudAssemblyTests.exe'
test=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
assert test.returncode==0 and test.stdout.startswith('PASS 864 '),test
files=sorted(set(pins)|set(added))
for rel in files:
 dest=out/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(work/rel,dest)
for name in ('validation-sources','contract-snapshots','transition-validation-sources','transition-contract-snapshots',
             'opening-validation-sources','opening-contract-snapshots','captain-validation-sources','captain-contract-snapshots'):
 shutil.copytree(parent/name,out/name)
for name in ('parent-main-source.json','parent-recorder-source.json','parent-opening-source.json','parent-captain-source.json'):
 shutil.copyfile(parent/name,out/name)
shutil.copyfile(parent/'qfr-source.json',out/'parent-captain-retry-source.json')
contracts=out/'assembly-contract-snapshots';contracts.mkdir();rows=[]
for p in (Path('E:/Codex/research/quantum-engine-kb/docs/BOTTOM-HUD-IMPLEMENTATION-CONTRACT-01.md'),
          Path('E:/Codex/research/quantum-engine-kb/validation/bottom-hud-contract-01-manifest.json'),
          Path('E:/Codex/research/quantum-engine-kb/evidence/bottom-hud-contract-01/original-windows.json'),
          Path('E:/Codex/research/dominion-resolution/SHARED-HUD-LAYOUT-INTERFACE-06.md'),
          base/'hud-assembly-hook-sites-01.json',base/'hud-assembly-inventory-01.json'):
 shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
tests=out/'assembly-validation-sources';tests.mkdir()
for p in here.iterdir():
 if p.is_file() and p.suffix in ('.py','.ps1','.h','.cpp','.vcxproj'):shutil.copyfile(p,tests/p.name)
(out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll');shutil.copyfile(log,out/log.name)
entries={e['path']:e for e in m['changes']}
for rel in changed+added:
 if rel not in entries:entries[rel]=dict(path=rel,before=None)
 entries[rel]['after']=sha(out/rel)
m['changes']=[entries[r] for r in sorted(entries)]
m['opening_flow']['source_files']={r:sha(out/r) for r in files}
m['opening_flow']['wrapper_sha256']=sha(out/'Release/ddraw.dll');m['opening_flow']['work_source']=str(work)
m['ui_canvas']['sources']={n:sha(out/'DDrawCompat/Research/MenuCanvas'/n) for n in m['ui_canvas']['sources']}
m['hud_assembly']=dict(schema='quantum-hud-assembly-source-1',parent_manifest_sha256=sha(parent/'qfr-source.json'),
 opt_in='QFR_HUD_ASSEMBLY=1; QFR_HUD_CAPTAINS=0',contracts=rows,validation_sources={p.name:sha(p) for p in tests.iterdir()},
 expected_assertions=864,validation=dict(stdout=test.stdout,exit=test.returncode,binary_sha256=sha(binary)),live_validated=False,
 policy='One centered original800-wide bottom assembly; physical scene/reserve/viewports and persistent picker projection; source HUD geometry retained.',
 roots=[hex(x) for x in (0x4E5E90,0x4EA588,0x4E5F20,0x4E5FB0,0x4E6040,0x4E60D0,0x4E6160,0x4DF678)],
 limitations=['Combined candidate not yet game-tested.',
  'Original-density artwork. Source restoration and broader faction/state coverage remain separate.',
  'Radar contact caches stay source-space; private submitted TL copies remain alive until original context reset.',
  'Preview privately binds pre-reserve projection for processing, restores prior device projection; persistent picker matrix remains physical.',
  'Simulated resolution geometry and mocked COM/input checks do not establish hardware compatibility.'])
(out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
print(json.dumps(dict(source=str(out),source_files=len(files),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=m['opening_flow']['wrapper_sha256'],test=test.stdout.strip())))
