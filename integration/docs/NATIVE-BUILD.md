# Native presentation build — developer procedure

## Adaptive-output successor

The adaptive candidate uses `intake-adaptive-20261030.1` and
`version-review-20261030-1.15.json`. Its native-display-v1 profile requires the
neutral E81 executable recipe (800×600 on-disk defaults); runtime binds the
primary physical output once before game CRT startup. The fixed native-1440-v1
DE3 recipe remains an explicit, separate profile. Do not mix their components.

Freeze a NEW adaptive intake with `tools/snapshot_adaptive_component.py --handoff
handoffs/master/rendering-adaptive-output-20261030 --previous-intake
integration/components/intake-native-20261029.1 --output
integration/components/NEW-INTAKE` from repository root (prefix the tool path
with `integration/`). The identified specialist tree and handoff must be present
for this collection step; player installation and subsequent package rebuilds
do not need those original specialist directories. The frozen intake carries
467 production files, current/inherited tests and contracts, final launcher
source, engine recipe and attributed live receipts; its complete lock is verified
by the ordinary native builder. No game media enters source.

The existing private 12-asset cache is reusable because visual identities are
unchanged. The materializer below can instead recover those inputs from the
selected built package and owned original media. Identified source inputs must
be committed before building. Keep the source decision and registry approval
separate: a candidate's version proposal is not a successful milestone.

```powershell
python -B integration/tools/build_native_remaster.py --intake integration/components/intake-adaptive-20261030.1 --decision versioning/decisions/version-review-20261030-1.15.json --original-game C:/Owned/Verified-1.04 --asset-inputs E:/Private/native-inputs --output E:/Private/new-adaptive-build
```

Run integration/versioning Python tests and WinPS5.1 Test-AdaptiveDisplay.ps1,
Test-FocusIdentity.ps1, Test-NativeIdentity.ps1, Test-Core.ps1 and packaged
Test-Lifecycle.ps1. The first two exercise capability/engine gates, bounds,
device/mode recheck, process-only environment and actual helper-prefix admission.
Their mocked display cases do not constitute live multi-resolution evidence.

Specialist fixture projects under the intake's evidence directories accept
`/p:WrapperSourceInclude=ABSOLUTE_INTAKE/wrapper-source/DDrawCompat` and
`/p:CanvasContractInclude=ABSOLUTE_INTAKE/wrapper-source/DDrawCompat/Research/MenuCanvas`.
Use VS18 v145 with `/p:Platform=Win32` and NEW OutDir/IntDir paths. Their earlier
logs and two-mode human passes are attributed component evidence, not new tests
of the rebuilt package. Record any new automatic live smoke separately.

The versioned player package exposes only original-data deltas and authorized
restored visuals, not complete original game files. Each successful release
requires a new immutable integration identity, manifest/checksums and explicit
Master approval; never replace an old tag's ZIP in place.

## Historical fixed-1440 workflow

This is a new managed candidate, not an automatic Current promotion or publication.
Select an explicit frozen intake and candidate decision. The preserved first intake,
components/intake-native-20261028.1/source-lock.json, freezes464 source files but
its recording-off mission test FAILED. Never select it as a repaired production
component. New fixes receive a new intake/build ID without overwriting that evidence.
The legacy int-20261018.1 builder remains separate; do not overlay that EXE.

Prerequisites: identified clean Git source revision; Python3.12; pefile2024.8.26;
verified North American official1.04 original game files; exact12 visual inputs
listed in the intake lock; NEW private output directory. The native engine recipe
uses the existing frozen Launcher recenter emitter unchanged. No player compiler,
Python, Git, codec encoder or RAD tools are required.

```powershell
python -B integration/tools/build_native_remaster.py --intake 'integration/components/SELECTED-NEW-INTAKE' --decision 'versioning/decisions/SELECTED-DECISION.json' --original-game 'C:/Owned/Verified-1.04' --asset-inputs 'E:/Private/native-inputs' --output 'E:/Private/new-trial-build'
```

The builder refuses changed inputs or dirty/uncommitted integration/versioning source. It
generates an exact native executable privately, a version-specific engine delta,
three packet-aware movie deltas, nine converted visual payloads, ordinary installer
and launcher, shared version.json/runtime config, corresponding wrapper source,
manifest/checksums and deterministic ZIP. The frozen wrapper binary is collected
with its full source/build provenance; byte-identical MSVC recompilation is not
claimed. Build record pins source revision/tree, native inputs, manifest and ZIP.

No original complete game EXE, decoder, BOLT archive or soundtrack is packaged.
Every original compressed audio packet and12-byte metadata is a source COPY range;
only restored video/index/header changes are literals. Output hashes enforce exact
reconstruction. The creator authorized supplied restored visuals for this project;
this does not transfer original copyright/trademark or license the commercial codec.

The private input cache is not stored in public source. To reproduce inputs from
a distributed package and verified owned1.04 movies:

```powershell
python -B integration/tools/materialize_native_inputs.py --intake 'integration/components/SELECTED-NEW-INTAKE' --package 'E:/Extracted Trial Package' --original-game 'C:/Owned/Verified-1.04' --output 'E:/Private/new-native-inputs'
```

This verifies package payloads/intake binding, copies the nine newly supplied
visual payloads into a NEW private cache and reconstructs the three movies from
owner audio plus the packaged deltas. The resulting12 inputs must match the frozen
asset hashes. Thus another environment need not rely on a conversation or the
original developer's temporary workspace to reconstruct the build.

Run integration Python tests, Test-Core.ps1, Test-NativeIdentity.ps1 and packaged
Test-Lifecycle.ps1. Rebuild production fixture projects with v145/Win32 using the
snapshot wrapper source include path. Automated/offline success does not establish
real startup, picture/audio, version text, focus, gameplay or save/load.

The package's recording-off version/display/buttons/Simon combined live acceptance
and ordinary managed lifecycle tests are required before promotion. Preserve prior
Current and test directories. Promotion and public release remain held until that
acceptance; never edit a tested package in place to change identity or status.
