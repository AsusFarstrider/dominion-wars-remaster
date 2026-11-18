"""Freeze a new combined opening candidate from the isolated working source."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
PARENT='68685b25061f2452fec38de591dd33fd278d76cae4c8b6decac35d8bc6742637'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))

def freeze(work,out):
    if out.exists():raise ValueError('Use a NEW frozen source')
    parent=work/'parent-main-source.json'
    if sha(parent)!=PARENT:raise ValueError('Wrong passing Main parent')
    m=load(parent);base={x['path']:x['sha256'] for x in load(ROOT/'asset-replacement/baseline-wrapper-files.json')['files']}
    expected=set(base)|{x['path'] for x in m['changes']}
    expected.update(p.relative_to(work).as_posix() for p in (work/'DDrawCompat/Research/OpeningFlow').iterdir() if p.is_file())
    expected.add('DDrawCompat/Research/FlightRecorder/Durability.h')
    for rel in sorted(expected):
        p=work/rel;dst=out/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dst)
    for name in ('validation-sources','contract-snapshots','transition-validation-sources','transition-contract-snapshots'):
        if (work/name).exists():shutil.copytree(work/name,out/name)
    for p in sorted(work.glob('build-*.log')):shutil.copyfile(p,out/p.name)
    (out/'Release').mkdir();shutil.copyfile(work/'Release/ddraw.dll',out/'Release/ddraw.dll')
    tests=out/'opening-validation-sources';tests.mkdir()
    for p in HERE.iterdir():
        if p.is_file() and p.suffix in ('.py','.cpp','.h','.vcxproj','.ps1'):shutil.copyfile(p,tests/p.name)
    contracts=out/'opening-contract-snapshots';contracts.mkdir();rows=[]
    paths=[Path('E:/Codex/research/quantum-engine-kb/docs')/n for n in ('BINK-NATIVE-FIT-CONTRACT-01.md','OPENING-FLOW-ENGINE-HANDOFF-01.md')]
    paths.append(Path('E:/Codex/research/dominion-resolution/OPENING-FLOW-EXTENSION-PLAN-03.md'))
    for p in paths:
        shutil.copyfile(p,contracts/p.name);rows.append(dict(path=str(p),sha256=sha(p)))
    m['parent']=str(work);m['parent_manifest_sha256']=PARENT
    m['capture_mode']='Combined opening UI/movie fit plus metadata recording/durable checkpoints; original native gameplay/HUD'
    m['changes']=[dict(path=rel,before=base.get(rel),after=sha(out/rel)) for rel in sorted(expected) if base.get(rel)!=sha(out/rel)]
    m['ui_canvas']['sources']={name:sha(out/'DDrawCompat/Research/MenuCanvas'/name) for name in m['ui_canvas']['sources']}
    m['opening_flow']=dict(schema='quantum-opening-flow-source-1',opt_in='QFR_OPENING_FLOW=1',native_engine_sha256='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6',
        passing_parent_sha256=PARENT,wrapper_sha256=sha(out/'Release/ddraw.dll'),work_source=str(work),
        source_files={rel:sha(out/rel) for rel in sorted(expected)},
        validation_sources={p.name:sha(p) for p in tests.iterdir() if p.is_file()},contracts=rows,
        intended_flow='Startup videos/cards -> Main -> New Game -> faction intro -> Briefing -> Fleet ship/captain -> first native mission -> normal quit',
        supported_state_candidates=[4,5,6,7,8,9,10,11,12],gameplay_hud_changed=False,
        limitations=['Combined runtime not yet validated; preview/custom input are explicit playthrough questions.',
            'Loading constructor/direct progress route is original-sized in this first combined candidate.',
            'World, HUD and unrelated dialogs/menus retain original native-engine layout.',
            'Original-density content, not a higher-density artwork import backend.',
            'Crash recovery retains confirmed prefix; queued tail after durable checkpoint can be lost.'])
    shutil.copyfile(parent,out/'parent-main-source.json')
    (out/'qfr-source.json').write_text(json.dumps(m,indent=2)+'\n')
    print(json.dumps(dict(source=str(out),manifest_sha256=sha(out/'qfr-source.json'),wrapper_sha256=m['opening_flow']['wrapper_sha256'],files=len(expected))))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('work',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();freeze(a.work,a.output)
