"""Prepare one NEW opening-flow game copy; never launch or modify Current."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
HERE=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def prepare(source,engine,inventory,target,run):
    manifest=load(source/'qfr-source.json');identity=sha(source/'qfr-source.json')
    if manifest.get('opening_flow',{}).get('schema')!='quantum-opening-flow-source-1':raise ValueError('Wrong source')
    tests=load(source/'validation/results.json');checks=load(source/'validation/opening-build-checks.json')
    if not all(x['passed'] and x['source_manifest_sha256']==identity for x in (tests,checks)):raise ValueError('Successful matching preflight required')
    if sha(source/'Release/ddraw.dll')!=manifest['opening_flow']['wrapper_sha256']:raise ValueError('Wrapper changed')
    path=HERE.parent/'native-resolution/prepare_session.py'
    spec=importlib.util.spec_from_file_location('opening_native_prepare',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    module.prepare(source,engine,inventory,target,run)
    for name in ('results.json','opening-build-checks.json'):shutil.copyfile(source/'validation'/name,run/name)
    scripts=run/'opening-scripts';scripts.mkdir()
    for name in ('Launch-OpeningFlow.ps1','prepare_session.py','recover_recording.py'):
        shutil.copyfile(HERE/name,scripts/name)
    inputs=load(run/'inputs.json');inputs['opening_flow']=dict(schema='quantum-opening-flow-run-1',
        source_manifest_sha256=identity,preflight_sha256=sha(run/'results.json'),build_checks_sha256=sha(run/'opening-build-checks.json'),
        launcher_sha256=sha(HERE/'Launch-OpeningFlow.ps1'),
        scripts={p.name:sha(p) for p in scripts.iterdir()},current_untouched=True,
        known_original_sized=['constructor/direct loading progress','in-game HUD and other gameplay interface','unrelated menus/dialogs'],
        test='Startup logos/videos/cards -> Main/New Game -> faction intro -> briefing scroll -> fleet ship/captain -> first mission -> normal quit')
    (run/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    print(json.dumps(dict(game=str(target),run=str(run),source_manifest_sha256=identity,status='prepared_not_launched')))
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('source','engine','inventory','target','run'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();prepare(a.source,a.engine,a.inventory,a.target,a.run)
