"""One-time identified source intake. NEW snapshot only, no game writes.

The normal build consumes this frozen intake, never arbitrary research files.
Private art delta is kept outside the source repository by design.
"""
import argparse
import importlib.util
import json
import subprocess
from pathlib import Path

from binary_delta import sha, make_delta, make_bolt_delta

ROOT = Path(__file__).resolve().parents[1]
FOUNDATION = '4f889dea81385c3a604d09eb82452a1b48fafa25'
ORIGINAL = '03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a'
RESOLUTION = 'ea1ea1b8e580265d304c3905199951be2288e9fc87bb9dae503d23191c29139b'
LAUNCHER = 'fb972d8fbabd6dcc929e78f74a8c3c158ea2f8a8a66dbc040291e643a9e91cd7'
WRAPPER = '0e4790d24a030529da62e3b8a98c5283e0953bbb926bdc7d9c8aa66c316cb566'
BOLT = '40ad59d38a6424558f80837735a47b5b9083b35c298ba4e49e8198a6f7bd2a8b'
SABER = '8bc7ac0f88a5112773938738e93df454f58e63b14879d0fd3bb33f9381a1dda0'


def put(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('xb') as stream:
        stream.write(data)


def encoded(value):
    return (json.dumps(value, indent=2, sort_keys=True)+'\n').encode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--modernizer', type=Path, default=Path('E:/Codex/dominion-wars-modernizer'))
    parser.add_argument('--resolution', type=Path, default=Path('E:/Codex/research/dominion-resolution'))
    parser.add_argument('--startup', type=Path, default=ROOT.parent/'startup-research')
    parser.add_argument('--original', type=Path, default=Path('E:/Codex/research/installed-1.04'))
    parser.add_argument('--private-output', type=Path, default=Path('E:/Codex/remaster-private-inputs/20261018.1'))
    args = parser.parse_args()
    snapshot = ROOT/'components/intake-20261018.1'
    if snapshot.exists() or args.private_output.exists():
        raise FileExistsError('Intake/private destination exists; preserving it')
    git = 'C:/Program Files/Git/cmd/git.exe'
    def git_read(*arguments):
        return subprocess.check_output([git, '-C', str(args.modernizer), *arguments])
    if git_read('rev-parse', FOUNDATION).decode().strip() != FOUNDATION:
        raise ValueError('Foundation revision unavailable')
    base = (args.original/'ds9dw.exe').read_bytes()
    current = (args.resolution/'game-selection-loading-v3/ds9dw.exe').read_bytes()
    branch_input = (args.startup/'input-window-probe-v2.exe').read_bytes()
    dll = (args.resolution/'game-selection-loading-v3/ddraw.dll').read_bytes()
    for data, expected in ((base, ORIGINAL), (current, RESOLUTION), (branch_input, LAUNCHER), (dll, WRAPPER)):
        if sha(data) != expected:
            raise ValueError('Reviewed component fingerprint differs')
    # Frozen, public-compatible foundation from a COMMIT, not the dirty worktree.
    names = git_read('ls-tree', '-r', '--name-only', FOUNDATION).decode().splitlines()
    for name in names:
        if name.startswith('releases/') or name.startswith('experiments/'):
            continue
        if Path(name).is_absolute() or '..' in Path(name).parts:
            raise ValueError('Unsafe foundation path')
        put(snapshot/'modernizer'/name, git_read('show', f'{FOUNDATION}:{name}'))
    put(snapshot/'resolution.patch.json', encoded(make_delta(base, current)))
    put(snapshot/'ddraw.dll', dll)
    # Preserve actual diagnostic/focus source; no compiled Build/Release products.
    wrapper_source = args.resolution/'diagnostic-source-v1'
    for name in ('DDrawCompat', 'Tools'):
        for path in sorted((wrapper_source/name).rglob('*')):
            if path.is_file():
                put(snapshot/'wrapper-source'/path.relative_to(wrapper_source), path.read_bytes())
    for name in ('DDrawCompat.sln', 'LICENSE.txt', 'README.md'):
        put(snapshot/'wrapper-source'/name, (wrapper_source/name).read_bytes())
    # Exact scripts used in latest tested no-switch path and shared recovery fix.
    for name in ('Launch-NoDesktopSwitch.ps1',):
        put(snapshot/'launcher'/name, (args.startup/'input-window-v2-tools'/name).read_bytes())
    for name in ('DominionWarsLauncher.psm1', 'Restore-DominionWarsDesktop.ps1'):
        put(snapshot/'launcher'/name, (args.resolution/'game-selection-loading-v3'/name).read_bytes())
    for name in ('Watch-NoResizeFocus-v2.ps1', 'build_input_window_v1.py', 'build_input_window_v2.py',
                 'test_input_window_v1.py', 'test_input_window_v2.py', 'test_win_modifier_v2.py',
                 'NO-RESIZE-FOCUS-v2.md', 'INPUT-WINDOW-v2.md', 'input-window-patch-v2.json'):
        put(snapshot/'launcher'/name, (args.startup/name).read_bytes())
    put(snapshot/'wide960-displayfit.ini', (args.resolution/'profiles/wide960-displayfit.ini').read_bytes())
    for pattern in ('build_*.py', 'test_*.py'):
        for path in sorted(args.resolution.glob(pattern)):
            put(snapshot/'resolution-history'/path.name, path.read_bytes())
    for name in ('SELECTION-LOADING-v3.md', 'selection-loading-patch-v3.json', 'FLEET-ORIGIN-v1.md',
                 'WIDE960-v1.md', 'RECOVERY-READ-RACE-v1.md', 'diagnostic-build-v1.json',
                 'Test-RecoveryRead-v1.ps1', 'DIAGNOSTIC-V1.md'):
        put(snapshot/'resolution-history'/name, (args.resolution/name).read_bytes())
    # Original data inventory: hashes only, no commercial data enters source.
    game_files = []
    for path in sorted(args.original.rglob('*')):
        if path.is_file() and (path.relative_to(args.original).parts[0].lower() == 'rs' or
                               path.name.lower() in ('ds9dw.exe', 'binkw32.dll', 'gizmolog.ico', 'gamespy.ico')):
            game_files.append(dict(path=path.relative_to(args.original).as_posix(), size=path.stat().st_size,
                                   sha256=sha(path.read_bytes())))
    put(snapshot/'original-files.json', encoded(game_files))
    # Private optional art: delta only; no full archive shipped, no assumed clearance.
    old_bolt = (args.original/'rs/Bolts/ds9blt.blt').read_bytes()
    new_bolt = Path('E:/Codex/research/dominion-remaster/native-damage-v1-20261017/native-all-states-v1.blt').read_bytes()
    if sha(old_bolt) != BOLT or sha(new_bolt) != SABER:
        raise ValueError('Private Saber inputs changed')
    put(args.private_output/'saber.patch.json', encoded(make_bolt_delta(old_bolt, new_bolt)))
    source_lock = dict(schema=1, intake_id='20261018.1', modernizer_commit=FOUNDATION,
        resolution_sha256=RESOLUTION, launcher_branch_sha256=LAUNCHER, original_exe_sha256=ORIGINAL,
        wrapper_sha256=WRAPPER, inputs=[dict(path=path.relative_to(snapshot).as_posix(),
                    size=path.stat().st_size, sha256=sha(path.read_bytes()))
                    for path in sorted(snapshot.rglob('*')) if path.is_file()],
        private_optional_saber=dict(path=str(args.private_output/'saber.patch.json'),
             sha256=sha((args.private_output/'saber.patch.json').read_bytes()), output_sha256=SABER,
             distribution='private owner-media only, not cleared for public redistribution'),
        source_provenance='Immutable hash-identified intake of reviewed research files; resolution patch is reproducible COPY/literal source against pinned retail exe.',
        source_thread_ids=dict(resolution='01a14a65-a9ab-77e1-805f-195c3483122d',launcher='01a1506e-c5cb-7511-bb7e-98aaaebbdd38',
                               compatibility='019f7c4d-08ba-79e0-899f-87bd7c87d953',textures='01a14a98-2dc1-7350-994b-3351b1c40496'))
    put(snapshot/'source-lock.json', encoded(source_lock))
    print(json.dumps(dict(snapshot=str(snapshot), files=len(source_lock['inputs']),
                          lock_sha256=sha((snapshot/'source-lock.json').read_bytes()),
                          private_patch_bytes=(args.private_output/'saber.patch.json').stat().st_size), indent=2))


if __name__ == '__main__':
    main()
