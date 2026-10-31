"""Build/run retained menu and opening-buffer/abrupt-recording fixtures."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import shutil
import sys
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
MSBUILD=Path('C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def imported(name,path):
    spec=importlib.util.spec_from_file_location(name,path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
def exports(path):
    return [(e.ordinal,e.name) for e in pefile.PE(str(path)).DIRECTORY_ENTRY_EXPORT.symbols]
def validate(source,reuse=None,controller_assertions=41):
    output=source/'validation/opening-build-checks.json'
    if output.exists():raise ValueError('Use a NEW frozen candidate')
    m=load(source/'qfr-source.json');identity=sha(source/'qfr-source.json');wrapper=source/'DDrawCompat';module=wrapper/'Research/MenuCanvas'
    for name,digest in m['opening_flow']['source_files'].items():
        if sha(source/name)!=digest:raise ValueError('Source changed: '+name)
    previous=load(reuse/'qfr-source.json') if reuse else None
    if previous:
        changed=[n for n,h in m['opening_flow']['source_files'].items() if previous['opening_flow']['source_files'].get(n)!=h]
        allowed={'DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp','DDrawCompat/Research/FlightRecorder/Queue.h','DDrawCompat/Research/FlightRecorder/Recorder.cpp','DDrawCompat/Research/MenuProbe/Probe.cpp','DDrawCompat/Research/NativeProbe/Probe.cpp'}
        if not set(changed)<=allowed or m['ui_canvas']['sources']!=previous['ui_canvas']['sources'] or m['ui_canvas']['validation_sources']!=previous['ui_canvas']['validation_sources']:
            raise ValueError('Regression binary reuse requires unchanged retained controller sources/dependencies')
    builds=[]
    for name in (*m['ui_canvas']['expected_assertions'],'OpeningTests','OpeningControllerTests'):
        if previous and name not in ('OpeningControllerTests','OpeningTests'):
            if name=='OpeningTests' and any(m['opening_flow']['validation_sources'][n]!=previous['opening_flow']['validation_sources'][n] for n in ('OpeningTests.cpp','OpeningTests.vcxproj')):raise ValueError('Opening buffer fixture changed')
            old=reuse/'validation'/name/'bin'/(name+'.exe');dest=source/'validation'/name/'bin'/(name+'.exe');dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(old,dest)
            builds.append(dict(test=name,reused_from=str(old),binary_sha256=sha(dest),exit=0,reason='Sources/dependencies unchanged; executable rerun against new provenance below'))
            continue
        folder=source/('opening-validation-sources' if name.startswith('Opening') else 'validation-sources')
        cmd=[str(MSBUILD),str(folder/(name+'.vcxproj')),'/p:Configuration=Release','/p:Platform=Win32',
            '/p:WrapperSourceInclude='+str(wrapper),'/p:CanvasContractInclude='+str(module),'/p:CanvasCoreInclude='+str(module),
            '/p:OutDir='+str(source/'validation'/name/'bin')+'\\','/p:IntDir='+str(source/'validation'/name/'obj')+'\\','/v:minimal','/nologo']
        p=subprocess.run(cmd,capture_output=True,text=True,timeout=120)
        log=source/'validation'/(name+'-build.log');log.parent.mkdir(parents=True,exist_ok=True);log.write_text(p.stdout+p.stderr)
        builds.append(dict(test=name,exit=p.returncode,log_sha256=sha(log),command=cmd))
        if p.returncode:print(p.stdout,p.stderr);raise ValueError('Test build failed: '+name)
    result=source/'validation/results.json'
    imported('opening_inherited_validation',source/'validation-sources/validate_canvas.py').validate(source,result)
    fixtures=[];recover=imported('opening_recovery',source/'opening-validation-sources/recover_recording.py')
    binary=source/'validation/OpeningTests/bin/OpeningTests.exe'
    for name,args,code in (('normal',[],0),('abrupt',['terminate'],77)):
        run=source/'validation/opening-fixtures'/name
        env=dict(os.environ,QFR_DIR=str(run),QFR_OPENING_FLOW='1');env.pop('QFR_TEST_FAIL_AFTER_EVENTS',None)
        p=subprocess.run([str(binary),*args],env=env,capture_output=True,text=True,timeout=20)
        r=recover.inspect(run)
        passed=p.returncode==code and r['durable_checkpoints']>0 and not r['issues'] and (
            r['capture_complete'] and 'PASS 27 ' in p.stdout if code==0 else not r['capture_complete'] and bool(r['exception_records']))
        row=dict(case=name,exit=p.returncode,expected=code,stdout=p.stdout,stderr=p.stderr,passed=passed,recovery=r)
        fixtures.append(row);(run/'fixture-result.json').write_text(json.dumps(row,indent=2)+'\n')
    controller=source/'validation/OpeningControllerTests/bin/OpeningControllerTests.exe'
    c=subprocess.run([str(controller)],capture_output=True,text=True,timeout=30)
    controller_result=dict(exit=c.returncode,stdout=c.stdout,stderr=c.stderr,binary_sha256=sha(controller),passed=c.returncode==0 and c.stdout.startswith('PASS '+str(controller_assertions)+' '))
    baseline=Path('E:/Codex/remaster-builds/playable-int-20261018.1/ddraw.dll')
    log=sorted(source.glob('build-*.log'))[-1];text=log.read_text(encoding='utf-8-sig')
    result=dict(schema='quantum-opening-build-checks-1',source_manifest_sha256=identity,wrapper_sha256=sha(source/'Release/ddraw.dll'),
        build_log_sha256=sha(log),clean_wrapper_build=all(s in text for s in ('Build succeeded.','0 Warning(s)','0 Error(s)')),
        exports_match_approved=exports(source/'Release/ddraw.dll')==exports(baseline),export_count=len(exports(baseline)),
        inherited_tests=load(source/'validation/results.json'),builds=builds,opening_fixture_sha256=sha(binary),fixtures=fixtures,controller=controller_result,
        game_launched=False,live_opening_validated=False,source_review_pending=True,
        limitations=['Console fixtures do not execute Bink or validate live graphics/input/audio.',
            'Abrupt termination is of the disposable fixture, not the game.',
            'Retained menu tests compile optional opening delegation out; combined integration needs the live flow.'])
    result['passed']=result['clean_wrapper_build'] and result['exports_match_approved'] and result['inherited_tests']['passed'] and controller_result['passed'] and all(x['passed'] for x in fixtures)
    output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(passed=result['passed'],source_manifest_sha256=identity,wrapper_sha256=result['wrapper_sha256'],fixtures=[dict(case=x['case'],passed=x['passed'],retained_events=x['recovery']['last_inspected_sequence']) for x in fixtures])))
    if not result['passed']:raise SystemExit(1)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('--reuse-regressions',type=Path);p.add_argument('--controller-assertions',type=int,default=41);a=p.parse_args();validate(a.source,a.reuse_regressions,a.controller_assertions)
