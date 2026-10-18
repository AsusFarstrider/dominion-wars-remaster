# Bounded render-target telemetry

October 17, 2026. Separate diagnostic build derived from the existing focus v3b
source, not substituted into the known-good functional installation.

- Build: `diagnostic-source-v1/Release/ddraw.dll`, 3,473,920 bytes, x86 PE32.
- SHA256: `0E4790D24A030529DA62E3B8A98C5283E0953BBB926BDC7D9C8AA66C316CB566`.
- Version: `v0.7.1-resolution-diag1`. All 42 named exports/ordinals match the
  preserved focus-control DLL. Full verification in diagnostic-build-v1.json.
- Independent game root: `game-diagnostic-v1`. Original 1.04 executable/assets.
  Working `game-test`, remaster game copies, normal installation and public
  repository remain unchanged.
- Source retains the focus guard. Source delta is exactly DeviceState.cpp,
  DrawPrimitive.cpp, the local WDK include path in the project, and the diagnostic
  version-generation script. Per-file before/after hashes are recorded.
- Release/x86 build: MSVC v145, SDK 10.0.26100.0, existing workspace WDK headers.
  Initial sandbox PDB-service failure required an outside-sandbox compile; explicit
  project include paths then resolved the missing d3dumddi.h header. No packages,
  drivers or new toolchains installed. Existing code remains BSD-0-Clause.

## What is measured

With process-local `DW_RESOLUTION_TELEMETRY=1`:

1. DWRT_BIND: records the original driver SetRenderTarget return status, handle,
   subresource and resource descriptor dimensions (maximum 64 distinct events).
2. DWRT_DRAW: records successful/failed actual driver draw return status plus
   app/logical target, current driver-facing target, app/current viewport,
   source texture-0 dimensions, transformed-vertex flag, depth-buffer presence,
   primitive kind/count and formats. Each distinct dimensions/viewport case is
   logged at sample 1 and sample 128; maximum 96 lines process-wide, 48 cases/thread.

No full debug logging, framebuffer readback, per-frame file I/O or draw-state
overrides. After the cap is reached the added logger returns immediately. This
is still a different DLL with some diagnostic overhead, not a performance test.
Resource pointers are session-local and cannot identify objects across runs.

The source's existing getAppState/getCurrentState paths supply dimensions; the
telemetry does not force them. A driver-accepted bind followed by a successful
indexed triangle draw to the matching handle/size is stronger evidence than
allocation or requested configuration alone. It is not a GPU pixel readback,
proof that every rendering path scales, or a source-texture-quality measurement.

## Run / recovery

```powershell
.\Run-ResolutionTest.ps1 -Scale 2 -BilinearReadback -Diagnostic
```

Same hash gates, focus harness, display watchdog, mutex and guarded profile restore
as functional tests. The telemetry environment flag applies only to the child
process tree and is restored in the driver process afterward. Default launches
still use the exact original focus-control wrapper, not the diagnostic copy.
Recovery requires the matching `-Diagnostic` switch as well as scale/filter/run.
The diagnostic wrapper remains in its separate copy; no DLL swap/recovery is
needed in the functional installation.

## Analysis

```powershell
python .\analyze_render_targets.py '<saved wrapper log>' --output '<new evidence JSON>'
```

The analyzer matches chronological successful binds with successful indexed
triangle draws, checking dimensions and valid viewport bounds. Allocation/config
alone, failed/missing/later/mismatched binds, failed draws, nonindexed/line draws
and invalid viewports do not qualify. Ten synthetic unit tests pass. Output
creation refuses existing files. Check source/scene timing and raw logs as well.

Run/result metadata starts with ActualRenderDimensionsVerified=false; it is not
automatically promoted by launching. The separate analyzed evidence is authoritative
for what was actually observed. Missing telemetry is a diagnostic gap, not proof
of failure or proof that app(2) worked. Follow with instrumented 1x control if
needed for a same-build comparison before making broader claims.
