# Native1440 trial build — developer procedure

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
