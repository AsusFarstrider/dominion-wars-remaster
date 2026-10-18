"""Build a verified package from a frozen source revision and owned NA1.04 data.

Python/pefile are developer-only. Player installation uses Windows PowerShell5.1.
Generated complete game executables remain in the PRIVATE output directory.
"""
import argparse
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import zipfile
import zlib
from pathlib import Path

from binary_delta import sha, apply_delta, make_delta

ROOT=Path(__file__).resolve().parents[1]
INTAKE=ROOT/'components/intake-20261018.1'


def data_json(value):
    return (json.dumps(value,indent=2,sort_keys=True)+'\n').encode()


def put(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open('xb') as stream:stream.write(data)


def load_window_builder():
    # An installed, version-pinned developer dependency is portable. The local
    # fallback is explicit for this workspace and never enters a player package.
    if Path('E:/Codex/tools/pydeps/pefile.py').exists():
        sys.path.insert(0,'E:/Codex/tools/pydeps')
    import pefile
    if pefile.__version__!='2024.8.26':raise ValueError('pefile2024.8.26 required')
    path=INTAKE/'launcher/build_input_window_v1.py'
    spec=importlib.util.spec_from_file_location('frozen_input_window_builder',path)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module,pefile


def compose_engine(original,lock):
    resolution=apply_delta(original,json.loads((INTAKE/'resolution.patch.json').read_text()))
    if sha(resolution)!=lock['resolution_sha256']:raise ValueError('Resolution artifact mismatch')
    module,pefile=load_window_builder()
    original_base=module.BASE
    # Retarget ONLY the version gate to this identified integration parent. The
    # proven import/hook/capacity guards and native stub emitter remain unchanged.
    module.BASE=lock['resolution_sha256']
    try:combined,report=module.build(resolution,hwnd_address=0xa387a4)
    finally:module.BASE=original_base
    before=pefile.PE(data=resolution);after=pefile.PE(data=combined)
    section=next(s for s in before.sections if s.Name.rstrip(b'\0')==b'.dwui')
    hook=before.get_offset_from_rva(0x49e5ee-0x400000)
    start=section.PointerToRawData+section.Misc_VirtualSize
    stub=bytes.fromhex(report['stub_bytes'])
    allowed=set(range(hook,hook+13))|set(range(start,start+len(stub)))|set(range(section.get_file_offset()+8,section.get_file_offset()+12))
    changed={i for i,(a,b) in enumerate(zip(resolution,combined)) if a!=b}
    if len(resolution)!=len(combined) or not changed<=allowed:raise ValueError('Integration touched unexpected resolution bytes')
    source_report=json.loads((INTAKE/'launcher/input-window-patch-v2.json').read_text())
    if stub[:-5]!=bytes.fromhex(source_report['stub_bytes'])[:-5]:raise ValueError('Native input body differs from passed v2')
    if report['stub_va']==source_report['stub_va']:raise ValueError('Input hook was not relocated away from resolution code')
    return combined,dict(resolution_parent_sha256=sha(resolution),launcher_parent_sha256=lock['launcher_branch_sha256'],
                         output_sha256=sha(combined),input_hook=report,allowed_changed_bytes=len(changed),
                         resolution_bytes_preserved_outside_input_port=True,source_native_stub_body_preserved=True)


def source_identity(development):
    git=shutil.which('git') or 'C:/Program Files/Git/cmd/git.exe'
    paths=sorted(p for p in ROOT.rglob('*') if p.is_file() and '__pycache__' not in p.parts
                 and not p.name.startswith('current.') and not any(part.startswith('.test') for part in p.relative_to(ROOT).parts))
    digest=sha(data_json([dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p.read_bytes())) for p in paths]))
    try:
        revision=subprocess.check_output([git,'-C',str(ROOT.parent),'rev-parse','HEAD'],stderr=subprocess.DEVNULL).decode().strip()
        dirty=subprocess.check_output([git,'-C',str(ROOT.parent),'status','--porcelain','--','integration']).decode().strip()
    except subprocess.CalledProcessError:
        revision=None;dirty='No source commit'
    if (not revision or dirty) and not development:raise ValueError('Commit and freeze integration source first; development mode is not promotable')
    return revision if revision and not dirty else 'uncommitted-content-'+digest,digest,bool(dirty or not revision)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-game',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--private-asset-delta',type=Path)
    parser.add_argument('--development',action='store_true')
    args=parser.parse_args()
    if args.output.exists():raise FileExistsError('Build output must be new')
    lock=json.loads((INTAKE/'source-lock.json').read_text())
    for item in lock['inputs']:
        path=INTAKE/item['path']
        if sha(path.read_bytes())!=item['sha256'] or path.stat().st_size!=item['size']:raise ValueError('Intake modified: '+item['path'])
    original_files=json.loads((INTAKE/'original-files.json').read_text())
    for item in original_files:
        path=args.original_game/item['path']
        if sha(path.read_bytes())!=item['sha256']:raise ValueError('Owned original input mismatch: '+item['path'])
    source_revision,tree_hash,uncommitted=source_identity(args.development)
    combined,composition=compose_engine((args.original_game/'ds9dw.exe').read_bytes(),lock)
    package=args.output/'package'
    # Everything below is generated into a NEW private build directory.
    args.output.mkdir(parents=True)
    package.mkdir()
    put(args.output/'private/combined-ds9dw.exe',combined)
    engine=make_delta((args.original_game/'ds9dw.exe').read_bytes(),combined)
    put(package/'patches/engine.json',data_json(engine))
    for path in sorted((INTAKE/'modernizer').rglob('*')):
        if path.is_file():put(package/'foundation'/path.relative_to(INTAKE/'modernizer'),path.read_bytes())
    runtime={}
    def runtime_put(target,data):
        put(package/'runtime'/target,data)
        runtime[target]=dict(source='runtime/'+target,target=target,size=len(data),sha256=sha(data))
    for path in sorted((ROOT/'runtime').iterdir()):
        if path.is_file():runtime_put(path.name,path.read_bytes())
    for name in ('Launch-NoDesktopSwitch.ps1','DominionWarsLauncher.psm1','Restore-DominionWarsDesktop.ps1'):
        runtime_put(name,(INTAKE/'launcher'/name).read_bytes())
    runtime_put('ddraw.dll',(INTAKE/'ddraw.dll').read_bytes())
    runtime_put('DDrawCompat-LICENSE.txt',(INTAKE/'wrapper-source/LICENSE.txt').read_bytes())
    helper=(INTAKE/'launcher/Watch-NoResizeFocus-v2.ps1').read_text(encoding='utf-8-sig')
    old="$expected='FB972D8FBABD6DCC929E78F74A8C3C158EA2F8A8A66DBC040291E643A9E91CD7'"
    if helper.count(old)!=1:raise ValueError('Unexpected helper version gate')
    helper=helper.replace(old,"$expected='"+sha(combined).upper()+"'")
    runtime_put('Watch-RemasterFocus.ps1',helper.encode('utf-8'))
    for path in sorted((ROOT/'installer').iterdir()):
        if path.is_file():put(package/path.name,path.read_bytes())
    put(package/'Remaster.Core.psm1',(ROOT/'runtime/Remaster.Core.psm1').read_bytes())
    profile=(INTAKE/'wide960-displayfit.ini').read_bytes()
    profiles=[]
    for name,text in (('d3d9on12',profile),('native',profile.replace(b'ForceD3D9On12 = on',b'ForceD3D9On12 = off'))):
        filename='profiles/'+name+'.ini';put(package/filename,text)
        profiles.append(dict(name=name,path=filename,sha256=sha(text)))
    asset=None
    if args.private_asset_delta:
        payload=args.private_asset_delta.read_bytes()
        if sha(payload)!=lock['private_optional_saber']['sha256']:raise ValueError('Private asset delta input mismatch')
        asset=json.loads(payload)
        archive=(args.original_game/'rs/Bolts/ds9blt.blt').read_bytes()
        result=apply_delta(archive,asset)
        if sha(result)!=lock['private_optional_saber']['output_sha256']:raise ValueError('Private asset round trip failed')
        put(package/'patches/private-saber.json',payload)
        asset=dict(path='patches/private-saber.json',target='rs/Bolts/ds9blt.blt',input_sha256=asset['input_sha256'],output_sha256=asset['output_sha256'],distribution='PRIVATE owner-media-derived art, NOT approved for public distribution')
    for path in sorted((ROOT/'docs').glob('*.md')):
        put(package/'docs'/path.name,path.read_bytes())
    # Corresponding wrapper source and build record accompany the modified DLL.
    for path in sorted((INTAKE/'wrapper-source').rglob('*')):
        if path.is_file():put(package/'third_party/DDrawCompat-source'/path.relative_to(INTAKE/'wrapper-source'),path.read_bytes())
    put(package/'third_party/DDrawCompat-build-provenance.json',(INTAKE/'resolution-history/diagnostic-build-v1.json').read_bytes())
    files=[dict(path=p.relative_to(package).as_posix(),size=p.stat().st_size,sha256=sha(p.read_bytes())) for p in sorted(package.rglob('*')) if p.is_file()]
    build_id='int-20261018.1'+('-saber-private' if asset else '')
    manifest=dict(schema=1,product='Dominion Wars Remaster',build_id=build_id,build_date='2026-10-18',
        flavor='private-saber' if asset else 'original-assets',
        status='integration-candidate',source_revision=source_revision,source_tree_sha256=tree_hash,uncommitted_source=uncommitted,
        baseline='Verified NA1.04 + Modernizer0.6.0-alpha runtime at commit '+lock['modernizer_commit'],
        modernizer_commit=lock['modernizer_commit'],intake_lock_sha256=sha((INTAKE/'source-lock.json').read_bytes()),
        resolution=dict(sha256=lock['resolution_sha256'],logical='960x600',aspect='16:10',scale='display(-1)',measured_branch_render='2304x1440 on2560x1440 desktop'),
        launcher=dict(branch_sha256=lock['launcher_branch_sha256'],no_startup_switch=True,source='input-window-v2 + no-resize-focus-v2',focus_recovery_cycle_cap=32),
        wrapper=dict(sha256=lock['wrapper_sha256'],version='DDrawCompat0.7.1-resolution-diag1',telemetry_default=False,license='BSD-0-Clause'),
        engine=dict(path='patches/engine.json',input_sha256=engine['input_sha256'],output_sha256=engine['output_sha256']),
        asset=asset,original_files=original_files,runtime_files=list(runtime.values()),profiles=profiles,files=files,
        build_tools=dict(python=sys.version.split()[0],pefile='2024.8.26',zlib=zlib.ZLIB_VERSION,zlib_runtime=zlib.ZLIB_RUNTIME_VERSION,player='Windows PowerShell5.1; no Python/Git required'),
        approvals=dict(internal_baseline='pending user acceptance',public_release='not authorized'),
        combined_validation=dict(automated='pending package tests',gameplay='pending',focus='pending',fresh_media_install='pending'),
        excluded=['cohesive Saber geometry draft','captain portrait replacements','unreviewed external model','true16:9 and arbitrary-resolution UI claims','DRM/no-CD changes'])
    put(package/'manifest.json',data_json(manifest))
    checks={p.relative_to(package).as_posix():sha(p.read_bytes()) for p in sorted(package.rglob('*')) if p.is_file()}
    put(package/'CHECKSUMS.json',data_json(checks))
    archive_path=args.output/('dominion-wars-remaster-'+build_id+'.zip')
    with zipfile.ZipFile(archive_path,'x',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
        for path in sorted(package.rglob('*')):
            if path.is_file():
                info=zipfile.ZipInfo(path.relative_to(package).as_posix(),date_time=(2026,10,18,0,0,0))
                info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o644<<16
                archive.writestr(info,path.read_bytes())
    with zipfile.ZipFile(archive_path) as archive:
        if archive.testzip() is not None:raise ValueError('Package ZIP CRC failure')
        for path in sorted(package.rglob('*')):
            if path.is_file() and archive.read(path.relative_to(package).as_posix())!=path.read_bytes():raise ValueError('Package ZIP contents differ')
    record=dict(build_id=manifest['build_id'],source_revision=source_revision,composition=composition,
        package=str(archive_path),package_sha256=sha(archive_path.read_bytes()),package_size=archive_path.stat().st_size,
        manifest_sha256=sha((package/'manifest.json').read_bytes()),private_assets=bool(asset),
        publish_allowed=False,promotion_allowed=False)
    put(args.output/'build-record.json',data_json(record))
    print(json.dumps(record,indent=2))


if __name__=='__main__':main()
