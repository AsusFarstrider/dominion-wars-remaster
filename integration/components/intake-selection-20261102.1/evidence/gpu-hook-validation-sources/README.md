# Recording-independent GPU installation repair

October 29, 2026. This is a child of the failed immutable
`E:/Codex/research/dominion-rendering/production-runtime-source-v1`. Preserve that
source, failed package/runs and recorder-on diagnostic. Master owns the next
package, actual version identity and launch. No Current or release changes here.

## Scope

Exactly three production files change:

* `Common/VtableHookVisitor.h`: keep Device3 DrawPrimitive,
  DrawIndexedPrimitive and Viewport3 Clear2 installed at INFO with recording off.
  These contain functional HUD adapters even when their compatibility-table
  entries are NULL. Exact member-pointer checks and the existing nonnull real
  slot guard remain. No blanket method installation or diagnostic enablement.
* `Remaster/Runtime.cpp` and `Research/OpeningFlow/OpeningFlow.cpp`: replace the
  existing Gizmo allowlist hash with the user-approved v5 file identity. The
  5C6 profile, movie timing/sequence/decoder and runtime configuration are unchanged.

No HUD math, layout, input mapping, camera, engine executable or asset-loading
behavior changes. Readiness63 is the engine-hook gate established before COM
creation; later COM vtable installation is tested separately.

## Reproduction

`prepare.py` checks all464 immutable v1 production source pins and creates a new
work-02 child. It intentionally refuses an existing destination. The two Gizmo
hash changes are explicit source edits in the resulting child.

From that child, rebuild with Visual Studio18 Build Tools/WindowsSDK10.0.26100,
the existing WDK include dependency, and:

```
MSBuild.exe DDrawCompat.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86 /p:PlatformToolset=v145 /m:3 /v:minimal /nologo /fl "/flp:logfile=build-production-09.log;verbosity=normal"
```

Solution platform is x86; standalone test platform is Win32. An initial mistaken
solution Win32 invocation and a sandbox PDB service mismatch are preserved in
logs07/08. The final clean build receipt must be log09. Build outside the sandbox
if the PDB service reports C1902; do not alter source to hide it.

`freeze.py` requires the clean final build, verifies that only those three
production files changed, copies a new source-v2, rebuilds focused tests against
it, and records hashes/commands/results. It refuses an existing source-v2.
It also compiles and runs the same visitor test against v1 as an expected-failure
control. Required Python dependency: pefile at E:/Codex/tools/pydeps for export
comparison. No codec/game is launched by either script.

## Regression evidence and limits

`GpuVisitorTests` uses actual CompatVtable, real Device3/Viewport3 enumeration,
VtableHookVisitor installation and installed hookFunc dispatch. Logging,
recorder/probe and adapter bodies are fixture callees. INFO/OFF, INFO/ON and
DEBUG/OFF, both adapter and pass-through cases, original-once/locking/results,
NULL slots, unrelated method skips and Device2 exclusion total90 assertions.
Against v1 it fails at the missing-three-hooks check, as intended. This test
closes the installation-layer coverage gap; earlier HUD tests called adapters
directly. The existing real HudAssembly tests separately cover864 assertions.
RuntimePolicy51 and OpeningController384 are rebuilt to check the managed
configuration and existing movie/sequence path after the SHA-only update.

Master's unchanged-v1 recording A/B restored the expected scene/HUD. Its captured
session is preserved under `handoffs/master/int-20261027.1-native-trial-r1`.
That proves a diagnostic dependency, not repaired recording-off acceptance.
The missing Draw routes explain the mixed HUD. Do not claim a specific late
world-erasing Clear2: original strip clear precedes world rendering, and the
exact black-world mechanism was not joined. The new real mission test must
establish that the narrow repair resolves the visible failure.

## Gizmo v5

Supplier workspace: `cinematic-extraction/original-gizmo-hd-v5`. New Bink is
73633988bytes, BIKi2560x1440/601/2997:100, SHA256
`b506781006eddd97fe6f86108d67259639f8044cbd5bcf71fcd0734a14013688`.
Supplier preparation/report/HANDOFF are snapshotted in the child. They record
preservation of all original601 compressed audio packets/metadata and decoded
PCM equality; this component checks target SHA and reuses those receipts. Per
user direction there is no repeated codec campaign, and prior v3 guard evidence
is not attributed to v5.

## Master retest

Intake the frozen child, generate the real new build identity and bound config
with recording off, and include the pinned v5 video in the same candidate.
One combined live check: let Gizmo finish audibly and transition normally, enter
a mission, verify scene/HUD and ordinary interaction, pause/Escape/Alt-Tab and
quit normally. Keep recording off; do not use DEBUG or recorder as the repair.
Retain the independent bounded ledger for selected errors. No additional video
microtest campaign is requested. Integration/release remains held until Master
accepts the actual repaired managed candidate.
