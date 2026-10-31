"""Freeze the captain primitive trial; preserve its accepted opening parent."""
import hashlib,json,shutil,subprocess
from pathlib import Path
BASE=Path('E:/Codex/research/dominion-rendering')
PARENT=BASE/'opening-submenus-source-v5';WORK=BASE/'hud-captains-work-01';OUT=BASE/'hud-captains-source-v1'
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))
if __name__=='__main__':
    if OUT.exists():raise ValueError('Use a new frozen root')
    assert sha(PARENT/'qfr-source.json')=='04dfe4c49b7d8cac1414f6e96e06347c6f68c701a0531a840dbb1c0678fd0b6f'
    m=load(PARENT/'qfr-source.json');pins=m['opening_flow']['source_files']
    for rel,digest in pins.items():assert sha(PARENT/rel)==digest,rel
    changed=sorted(rel for rel,digest in pins.items() if sha(WORK/rel)!=digest)
    assert changed==['DDrawCompat/Common/VtableHookVisitor.h','DDrawCompat/DDrawCompat.vcxproj','DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp','DDrawCompat/Research/MenuProbe/Probe.cpp','DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp'],changed
    added=sorted(p.relative_to(WORK).as_posix() for p in (WORK/'DDrawCompat/Research/HudCaptain').iterdir() if p.is_file())
    files=sorted(set(pins)|set(added));log=WORK/'build-hud-05.log';text=log.read_text(encoding='utf-8-sig')
    assert all(s in text for s in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
    binary=BASE/'hud-captains-tests-01/bin/HudCaptainTests.exe';test=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
    assert test.returncode==0 and test.stdout.startswith('PASS 535 '),(test.returncode,test.stdout,test.stderr)
    for rel in files:
        dest=OUT/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(WORK/rel,dest)
    for name in ('validation-sources','contract-snapshots','transition-validation-sources','transition-contract-snapshots','opening-validation-sources','opening-contract-snapshots'):
        shutil.copytree(PARENT/name,OUT/name)
    for name in ('parent-main-source.json','parent-recorder-source.json'):shutil.copyfile(PARENT/name,OUT/name)
    shutil.copyfile(PARENT/'qfr-source.json',OUT/'parent-opening-source.json')
    contracts=OUT/'captain-contract-snapshots';contracts.mkdir();rows=[]
    for p in (Path('E:/Codex/research/quantum-engine-kb/docs/CAPTAIN-GROUP-ENGINE-HANDOFF-01.md'),Path('E:/Codex/research/quantum-engine-kb/docs/PRE-HUD-ARCHITECTURE-REASSESSMENT-01.md')):
        shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
    tests=OUT/'captain-validation-sources';tests.mkdir()
    for p in HERE.iterdir():
        if p.is_file() and p.suffix in ('.py','.h','.cpp','.ps1','.vcxproj'):shutil.copyfile(p,tests/p.name)
    shutil.copyfile(HERE.parent/'opening-flow/RuntimeTestMemory.h',tests/'RuntimeTestMemory.h')
    (OUT/'Release').mkdir();shutil.copyfile(WORK/'Release/ddraw.dll',OUT/'Release/ddraw.dll');shutil.copyfile(log,OUT/log.name)
    entries={x['path']:x for x in m['changes']}
    for rel in changed+added:
        if rel not in entries:entries[rel]=dict(path=rel,before=None)
        entries[rel]['after']=sha(OUT/rel)
    m['changes']=[entries[rel] for rel in sorted(entries)]
    m['opening_flow']['source_files']={rel:sha(OUT/rel) for rel in files}
    m['opening_flow']['wrapper_sha256']=sha(OUT/'Release/ddraw.dll');m['opening_flow']['work_source']=str(WORK)
    m['ui_canvas']['sources']={n:sha(OUT/'DDrawCompat/Research/MenuCanvas'/n) for n in m['ui_canvas']['sources']}
    m['captain_trial']=dict(schema='quantum-captain-trial-source-1',parent_manifest_sha256=sha(PARENT/'qfr-source.json'),
        opt_in='QFR_HUD_CAPTAINS=1',contracts=rows,temporary_layout='free reserved strip beside original HUD; native reserve unchanged',
        validation_sources={p.name:sha(p) for p in tests.iterdir()},expected_assertions=535,
        validation=dict(stdout=test.stdout,exit=test.returncode,binary_sha256=sha(binary)),live_validated=False,
        limitations=['Candidate not live tested; GPU/CPU coverage, auxiliary visibility and actual input remain the mission trial.',
            'Original-density art; source restoration and global HUD layout remain separate.',
            'No free trial strip or unsupported producer retains/restores original behavior and records a reason.',
            'No whole-frame/world copy, no render-target swap, no scene/projection/picker/reserve changes.'])
    (OUT/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
    print(json.dumps(dict(source=str(OUT),manifest_sha256=sha(OUT/'qfr-source.json'),wrapper_sha256=m['opening_flow']['wrapper_sha256'],files=len(files),test=test.stdout.strip())))
