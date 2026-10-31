# Runtime-sized native output: bounded source guidance

2026-10-30. Rendering leads implementation from passed esc-menu-perf-source-v1. Read-only guidance, no code/build/install/launch/Current change or competing implementation.

## Exact parent and reusable interfaces

Passed child: `E:\Codex\research\dominion-rendering\esc-menu-perf-source-v1`, manifest `e0933cd5271c5f74963c65771e7ad9abf3866a1a0aa112889e48c5f53f5e20d4`, wrapper `8479cc50983ddf8f44e0bfbf479330bf8c63d976906f985d0cf66a4d29cc4886`. This preserves both mission Escape->Main repair and Preferences ordered-paint axis-cache optimization. See rendering-research/ESC-MENU-FIX-RESULT-01.md. These are passed research children, not yet promoted Current.

No newer Resolution runtime-sizing implementation exists to import. Reuse native-resolution/build_engine.py's exact15-site inventory, HudAssembly/Layout.h runtime fit/group transforms, original source coordinate model, OpeningFlow/HdHistory native lease/history and shared geometry. Keep existing recenter/focus/recovery bodies; do not replace them with old960 scripts.

## One invariant: choose output once at startup

Introduce a single immutable `NativeOutput{physicalW,physicalH,displayIdentity,origin,epoch}` selected before the original first screen/layout request. Use current physical pixel mode of the actual selected game display, under correct DPI awareness, not summed virtual desktop size, work-area/taskbar dimensions, GetSystemMetrics from a virtualized context or asset size. Do not change the user's desktop mode. Validate the actual created client/render target/presentation against that decision.

Initial scope should be automatic STARTUP sizing, not arbitrary live WM_SIZE/monitor-drag/hotplug recreation. Existing origin0 primary constraint and the separate selection of a nonprimary/negative-origin monitor must be handled explicitly; varying W/H alone does not establish those workflows. A first primary-display-any-valid-W/H candidate is useful but must not be advertised as arbitrary-monitor support. If selection cannot be known until HWND exists, coordinate launcher-provided display identity or a verified pre-create screen request seam; do not infer a different monitor after allocation without an actual recreate transaction.

## Smallest engine change: same reviewed operands, one runtime decision

The15 entries in rendering-research/native-resolution/build_engine.py cover14 width/height operands plus projection. Changing only401860 or globalW/H misses original later default/reset/transition writers and input bounds.

| Sites | Role |
|---|---|
|428C84 height /428C89 width|Initial application layout request|
|42B5B5 width /42B5BF height|Mission default dimensions7137D0/D4|
|4A6480 width /4A6487 height|Screen-object defaults|
|433498 height /43349D width|Initial screen request|
|4AD865 height /4AD86A width|Generic application reset request|
|429883 height /429888 width|Initial input SetRect bounds|
|4292D9 height /4292DE width|Application transition mode request|
|40117A projectionP00|Original aspect-calibration initializer|

Contained option: keep ONE exactly pinned engine image and apply this audited operand recipe once in process from the selected NativeOutput before any of those instructions execute. The same image/config then runs across supported displays without a per-mode disk EXE table. Verify the entire expected parent instructions and all destinations first, range/overflow-check dimensions, make a transactional patch, restore code page protection, flush instruction cache and refuse initialization if any site differs. These details are Engine/Rendering implementation authority, not authorization to write speculative addresses. Preserve installed-file hash gates; a known, recorded in-memory15-operand delta is not arbitrary EXE acceptance.

Alternative: intercept all corresponding argument/default writer consumers using one shared decision, but that is more hooks and must still cover the full inventory. Do not add temporary global800/600 spoofing. Original startup/import ordering must prove this initialization precedes the first screen request; final UI hooks are too late. Keep native recenter hook49E5EE/private code occupancy untouched.

## Projection and physical layout

The current builder computes original pre-HUD P00 as `2*(800/W)*((H-154)/446)` and retains P11=3.5/depth terms. HudAssembly currently multiplies that canonical pre-HUD P00 by `sceneBottom/(H-154)`. Thus final intended P00 simplifies to `2*(800/W)*(sceneBottom/446)` for the common HUD policy. Derive from pinned canonical calibration, never cumulatively multiply the prior frame's value. Keep preview projection isolation using the pre-reserve native baseline; do not substitute the final HUD-reserve matrix into preview processing.

Existing prepareFrame already uses actual targetW/H and Layout::fit, then publishes5FF694 reserve,5FF698 sceneBottom,5FF6A4/A8 sceneWH,5FF6B0/6AC halves, world input size and four viewport records. Preserve its429330 entry barrier before processing and its original-source restoration/recapture semantics.4070C0 world picker receives the same final projection/physical dimensions. Do not update viewport only at draw submission after vertices were preprocessed.

Radar source radius/center/edges5FF6BC..D4, preview source size/originD8..E4 and original HUD widget coordinates stay source-space. Private transformed outputs/clears/native viewports and inverse input remain supplied by the one runtime layout generation. Avoid introducing per-resolution+80 or new group offsets.

## Existing HUD rounding and limits

HudAssembly/Layout::fit accepts W>=800,H>=600 and extents<=16384; resource panelP2..300, panelHeight1..400, inset<=154. It chooses common scale=min(W/800,H/600), bottom anchors all HUD groups, derives reserve from logical154 and preserves radar overlap. Assembly width uses Transform::rounded and left uses `(W-roundedAssemblyWidth)/2` integer truncation. This differs at odd leftover pixels from independently rounding exact centered edges. Reuse the ACTUAL existing policy consistently for HUD visual/input/clear/viewports rather than silently replacing it with menu Canvas centering. One-pixel placement differences between the two policies are not a reason to change source geometry.

Menus use Canvas::fit's half-open edge partition, signed rational arithmetic and margin rejection. OrderedPaint's new row/column caches already exercise exact-equivalence at fractional/minified/4K/ultrawide simulated sizes; they remove repeated inverse search but do not establish real display support. Keep primitive density/sourceUVs separate from layout scale.

Practical initial minimum is800x600 because current full HUD and snapshot path explicitly require it, even though build_engine.py allows640x480. Do not claim sub800 support merely from the builder bounds. Current builder maximum8192 and composition/resource limits are guards, not guaranteed usable modes in a32-bit process. Six button banks alone consume about231MiB; multiple native buffers and media history add memory. Apply checked pixel/byte/allocation limits and visible safe rejection. Portrait/narrow aspects can leave a tiny scene despite valid arithmetic; define a minimum scene width/height or mark those aspects unsupported until tested. Do not impose16:9 as an accidental requirement.

## Fixed1440 checks that must change together

- Remaster/Runtime.cpp currently EnumDisplaySettings-checks exact primary2560x1440 and exits; replace with the chosen NativeOutput policy and actual-target checks, preserving visible error reporting. Do not just remove the gate.
- Remaster/Identity.h schema currently requires profile `native-1440-v1`; introduce a separately versioned runtime-output profile with source/config/package binding. Do not reinterpret released1.10 metadata silently.
- OpeningFlow hdProfile's2560x1440 checks describe ASSET source and should remain for those pinned HD files. copyHd's target==history and HdHistory::identity are OUTPUT constraints: generalize to source-sized persistent decode then fit sourceWH->actual outputWH, with identity fast path when equal. Do not downsize history or decode into a fitted target; preserve first-full/selective/result1/audio/lifetime and unsafe-fallback refusal.
- HD/card skip predicates must use the committed output dimensions/fit policy, not source2560WH or old800plan dead bands. Card assets remain2560source; equality between publication.cardWH and targetWH must not silently disable clicks when fitting to1080.
- Native Main background and RGBA button placement use runtime physical fit over unchanged180x100 logical buttons. Preserve ordered coverage/modal ownership, exact original overlaps, ESC pre-input callback repair and original animation timings. Do not size buttons from their720x400 source raster.
- Launcher/installer/preparation/native inventory/profile hashes may still assume fixed native engine identity/dimensions. Bind ONE new engine/runtime recipe and chosen-output record across those boundaries, not a list of separately patched EXEs. Modernizer and released compatibility profile remain independent.

## Bounded tests, not per-resolution implementation forks

Offline: all15 startup/default/reset/input operands read the same selected values; rejects are atomic; projection final/pre-reserve relations; odd-width rounding1366x768;800x600/1280x960/1920x1080/2560x1440/3440x1440/3840x2160 layout assertions; source-sizedHD decode+downsample/upsample and native-target lease publication; card/fullviewport skip; modal axis cache equality and ESC repair retained. Those are test modes, not a shipping EXE table or live support claims.

One new runtime-sized engine/profile, disposable copies only. First live non1440 check1920x1080 on the actual1080 display, then baseline1440 regression using the SAME installed engine/DLL/config policy. Check natural openings/audio/skip, menu/modal clicks, first mission HUD/radar/preview/world picking/projection/camera, pause/ESC Main/Preferences responsiveness and normal quit. Additional aspect4:3/ultrawide/highDPI/multimonitor origins can be added as available, with explicit result boundaries. Unsupported modes must fail visibly before unsafe HD decode/output—not hang on black or fallback into legacy80,60.

Current int-20261029.1 stays immutable. This is the user's new generalization task, superseding the earlier release-only recommendation not to implement new modes; it does not retroactively certify them or promote this future candidate.
