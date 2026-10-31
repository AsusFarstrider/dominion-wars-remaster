"""Rebuild focused native fixtures from a frozen intake into a NEW private output."""
import argparse
import json
from pathlib import Path
import subprocess

from binary_delta import sha
from build_remaster import data_json, put


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--intake', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--msbuild', default='C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe')
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('Fixture output must be new')
    args.output.mkdir(parents=True)
    intake = args.intake.resolve()
    lock = json.loads((intake / 'source-lock.json').read_bytes())
    for pin in lock['files']:
        if sha((intake / pin['path']).read_bytes()) != pin['sha256']:
            raise ValueError('Intake changed: ' + pin['path'])
    evidence = intake / 'evidence'
    projects = [
        'adaptive-validation-sources/policy-tests/RuntimePolicyTests.vcxproj',
        'adaptive-validation-sources/OutputModeTests.vcxproj',
        'adaptive-validation-sources/opening-tests/OpeningControllerTests.vcxproj',
        'production-validation-sources/hud-tests/HudAssemblyTests.vcxproj',
        'esc-perf-validation-sources/tests/EscMenuTests.vcxproj',
    ]
    results = []
    for relative in projects:
        project = evidence / relative
        name = project.stem
        output = args.output.resolve() / name
        cmd = [args.msbuild, str(project), '/t:Rebuild', '/p:Configuration=Release', '/p:Platform=Win32',
               '/p:WrapperSourceInclude=' + str(intake / 'wrapper-source/DDrawCompat'),
               '/p:CanvasContractInclude=' + str(intake / 'wrapper-source/DDrawCompat/Research/MenuCanvas'),
               '/p:OutDir=' + str(output / 'bin') + '\\', '/p:IntDir=' + str(output / 'obj') + '\\',
               '/v:minimal', '/nologo']
        built = subprocess.run(cmd, capture_output=True, timeout=180)
        put(args.output / (name + '-build.log'), built.stdout + built.stderr)
        if built.returncode:
            raise RuntimeError(name + ' build failed: ' + (built.stdout + built.stderr).decode(errors='replace')[-5000:])
        binary = output / 'bin' / (name + '.exe')
        run = subprocess.run([str(binary)], capture_output=True, timeout=60)
        put(args.output / (name + '.log'), run.stdout + run.stderr)
        if run.returncode:
            raise RuntimeError(name + ' failed: ' + (run.stdout + run.stderr).decode(errors='replace'))
        results.append(dict(name=name, exit_code=run.returncode, binary_sha256=sha(binary.read_bytes()),
                            log_sha256=sha(run.stdout + run.stderr), build_command=cmd))
        print(name + ': ' + run.stdout.decode(errors='replace').strip(), flush=True)
    put(args.output / 'results.json', data_json(dict(status='pass', tests=results,
        intake_lock_sha256=sha((intake / 'source-lock.json').read_bytes()), gameplay_tested=False)))


if __name__ == '__main__':
    main()
