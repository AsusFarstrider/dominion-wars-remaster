"""Bounded pre-playtest checks against the actual mutable adaptive provider."""
import hashlib,json,shutil,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent
base=Path('E:/Codex/research/dominion-rendering')
work=base/'dynamic-output-work-01'; parent=base/'esc-menu-perf-source-v1'
ms='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
dest=work/'adaptive-validation';dest.mkdir(exist_ok=True)
policy=here/'policy-tests';policy.mkdir(exist_ok=True)
for name in ('RuntimePolicyTests.cpp','RuntimePolicyTests.vcxproj'):
 shutil.copyfile(parent/'production-validation-sources'/name,policy/name)
p=policy/'RuntimePolicyTests.cpp';s=p.read_text()
anchor=' const auto o=options(goodOptions);'
assert s.count(anchor)==1
s=s.replace(anchor,''' const auto dynamic=replace(replace(goodOptions,"native-1440-v1","native-display-v1"),"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6","e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023");
 check(options(dynamic).dynamicOutput&&!options(dynamic).recording,"explicit dynamic profile admits only neutral engine independently of capture");
 check(!options(goodOptions).dynamicOutput,"legacy profile never silently becomes adaptive");
 rejects([&]{options(replace(dynamic,"native-display-v1","native-1440-v1"));},"neutral engine cannot claim fixed1440 profile");
 rejects([&]{options(replace(goodOptions,"native-1440-v1","native-display-v1"));},"fixed engine cannot claim adaptive profile");
 rejects([&]{options(replace(dynamic,"e81aacb1","e81aacb2"));},"adaptive profile rejects unreviewed engine");
'''+anchor)
p.write_text(s)
results=[]
receipt=dest/'results.json'
def save():
 receipt.write_text(json.dumps(dict(status='offline checks only; game not launched',results=results,wrapper_sha256=sha(work/'Release/ddraw.dll') if (work/'Release/ddraw.dll').exists() else None),indent=2)+'\n')
def run(name,cmd,timeout=240):
 r=subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=timeout)
 (dest/(name+'.log')).write_text(r.stdout+r.stderr)
 print(name+': '+str(r.returncode)+' '+(r.stdout.strip().splitlines()[-1] if r.stdout.strip() else ''),flush=True)
 assert r.returncode==0,r.stdout+r.stderr
 results.append(dict(name=name,command=[str(x) for x in cmd],exit_code=r.returncode,log_sha256=sha(dest/(name+'.log'))))
 save()
def fixture(name,project,args=()):
 d=dest/name
 run(name+'-build',[ms,project,'/t:Rebuild','/p:Configuration=Release','/p:Platform=Win32','/p:WrapperSourceInclude='+str(work/'DDrawCompat'),'/p:CanvasContractInclude='+str(work/'DDrawCompat/Research/MenuCanvas'),'/p:OutDir='+str(d/'bin')+'\\','/p:IntDir='+str(d/'obj')+'\\','/v:minimal','/nologo'])
 binary=d/'bin'/(Path(project).stem+'.exe')
 run(name,[binary,*args]);results[-1]['binary_sha256']=sha(binary)
if '--build-only' in sys.argv:
 for name in ('RuntimePolicyTests','OutputModeTests','OpeningControllerTests','HudAssemblyTests','EscMenuTests','launcher-python','launcher-powershell'):
  log=dest/(name+'.log');s=log.read_text();assert s.startswith('PASS') or ('Ran 6 tests' in s and 'OK' in s)
  results.append(dict(name=name,exit_code=0,log_sha256=sha(log),receipt='successful preceding invocation; command is defined in validate.py'))
else:
 fixture('RuntimePolicyTests',policy/'RuntimePolicyTests.vcxproj')
 fixture('OutputModeTests',here/'OutputModeTests.vcxproj')
 fixture('OpeningControllerTests',here/'opening-tests/OpeningControllerTests.vcxproj')
 fixture('HudAssemblyTests',parent/'production-validation-sources/hud-tests/HudAssemblyTests.vcxproj')
 fixture('EscMenuTests',parent/'esc-perf-validation-sources/tests/EscMenuTests.vcxproj')
 run('launcher-python',['C:/Python314/python.exe','-B','-m','unittest','discover','-s','startup-research/adaptive-display-v1-source/tests','-v'])
 run('launcher-powershell',['C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File','startup-research/adaptive-display-v1-source/tests/Test-AdaptiveDisplay.ps1'])
run('wrapper-clean-build',[ms,work/'DDrawCompat.sln','/t:Rebuild','/p:Configuration=Release','/p:Platform=x86','/p:PlatformToolset=v145','/v:minimal','/nologo'],600)
save()
print('ALL CHECKS PASSED',flush=True)
