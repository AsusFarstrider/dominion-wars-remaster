"""Freeze the coordinated submenu/loading candidate with exact parent provenance."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import argparse

BASE=Path('E:/Codex/research/dominion-rendering')
PARENT=BASE/'opening-recorder-source-v1'
WORK=BASE/'opening-submenus-work-01'
OUT=BASE/'opening-submenus-source-v1'
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revision',type=int,default=1)
    parser.add_argument('--controller-assertions',type=int,default=70)
    parser.add_argument('--build-log',default='build-submenus-03.log')
    args=parser.parse_args();OUT=BASE/('opening-submenus-source-v'+str(args.revision))
    if OUT.exists():raise ValueError('Use a new frozen source')
    parent_hash=sha(PARENT/'qfr-source.json')
    assert parent_hash=='6fee5d37b89c105ee350a34bea4776171cfe5bfcd62d42989ce547bb87d569b5'
    m=load(PARENT/'qfr-source.json');pins=m['opening_flow']['source_files']
    for rel,digest in pins.items():assert sha(PARENT/rel)==digest,rel
    changed=[rel for rel,digest in pins.items() if sha(WORK/rel)!=digest]
    expected=['DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp']
    if args.revision>=4:expected+=['DDrawCompat/Research/OpeningFlow/OpeningFlow.h','DDrawCompat/Research/MenuCanvas/CanvasRuntime.cpp','DDrawCompat/Research/MenuCanvas/MenuFamilies.h']
    assert sorted(changed)==sorted(expected),changed
    log=WORK/args.build_log;text=log.read_text(encoding='utf-8-sig')
    assert all(s in text for s in ('Build succeeded.','0 Warning(s)','0 Error(s)'))
    controller=BASE/('opening-submenus-tests-'+str(args.revision).zfill(2))/'controller/bin/OpeningControllerTests.exe'
    c=subprocess.run([str(controller)],capture_output=True,text=True,timeout=20)
    assert c.returncode==0 and c.stdout.startswith('PASS '+str(args.controller_assertions)+' '),(c.returncode,c.stdout,c.stderr)
    for rel in pins:
        dest=OUT/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(WORK/rel,dest)
    for name in ('validation-sources','contract-snapshots','transition-validation-sources','transition-contract-snapshots','opening-contract-snapshots'):
        if (PARENT/name).exists():shutil.copytree(PARENT/name,OUT/name)
    tests=OUT/'opening-validation-sources';tests.mkdir()
    for p in HERE.iterdir():
        if p.is_file() and p.suffix in ('.py','.ps1','.h','.cpp','.vcxproj'):shutil.copyfile(p,tests/p.name)
    contracts=OUT/'opening-contract-snapshots';rows=[]
    for p in (Path('E:/Codex/research/quantum-engine-kb/docs/REMAINING-MENU-ENGINE-HANDOFF-01.md'),
              Path('E:/Codex/research/quantum-engine-kb/docs/LOADING-PROGRESS-ENGINE-HANDOFF-01.md'),
              Path('E:/Codex/research/quantum-engine-kb/docs/STATE27-MENU-ENGINE-HANDOFF-01.md'),
              Path('E:/Codex/research/quantum-engine-kb/docs/DEPARTURE-AND-DETACHED-LOADING-HANDOFF-01.md'),
              Path('E:/Codex/research/dominion-resolution/SUBMENU-REUSE-MAP-04.md'),
              Path('E:/Codex/research/dominion-resolution/PREFERENCES-SLIDER-INPUT-05.md')):
        shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
    (OUT/'Release').mkdir();shutil.copyfile(WORK/'Release/ddraw.dll',OUT/'Release/ddraw.dll')
    shutil.copyfile(log,OUT/log.name)
    for name in ('original-identities.json','parent-recorder-source.json','parent-main-source.json'):shutil.copyfile(WORK/name,OUT/name)
    for entry in m['changes']:
        if entry['path'] in changed:entry['after']=sha(OUT/entry['path'])
    f=m['opening_flow'];f['source_files']={rel:sha(OUT/rel) for rel in sorted(pins)}
    m['ui_canvas']['sources']={name:sha(OUT/'DDrawCompat/Research/MenuCanvas'/name) for name in m['ui_canvas']['sources']}
    f['validation_sources']={p.name:sha(p) for p in tests.iterdir() if p.is_file()}
    f['wrapper_sha256']=sha(OUT/'Release/ddraw.dll');f['work_source']=str(WORK)
    f['supported_state_candidates']=[4,5,6,7,8,9,10,11,12,15,20,21,22,27]
    f['limitations']=['This submenu/loading candidate has not been live tested.',
        'Loading uses an explicitly cleared black surround and exact direct-call ownership, including detached constructor updates.',
        'Unmapped Multiplayer owners beyond20/21/22 and Skirmish27, and new networking support, remain outside this pass.',
        'In-game HUD and unrelated confirmation/tooltip paths retain their existing behavior.',
        'Original-density content fitted on native rendering; no replacement-artwork changes.']
    m['submenu_followup']=dict(schema='quantum-opening-submenus-source-1',parent_manifest_sha256=parent_hash,
        opt_in='QFR_OPENING_SUBMENUS=1',contracts=rows,controller_assertions=args.controller_assertions,live_validated=False,
        loading='400x110 original panel at logical200,245, fresh black surround, fit before direct present or at outer traversal',
        private_hooks=[dict(va=a,bytes=b) for a,b in (
            ('420FC9','8b4f778bc1'),('420032','8b77778b4642'),('41FB09','89442414894c2418'),
            ('434560','8b44240483ec08'),('4A79B0','8a410481ec00010000'))])
    (OUT/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
    print(json.dumps(dict(source=str(OUT),manifest_sha256=sha(OUT/'qfr-source.json'),wrapper_sha256=f['wrapper_sha256'],controller=c.stdout.strip(),live_validated=False)))
