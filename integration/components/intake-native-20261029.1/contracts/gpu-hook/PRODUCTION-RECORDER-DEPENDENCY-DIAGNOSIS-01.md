# Recording-off GPU adapter installation gap

2026-10-29. **Master reports a live failure of production-runtime-source-v1: recording off gives a black mission scene and tiny GPU HUD; recording on restores the expected scene/HUD with the same85812305 wrapper and native engine/assets.** Engine has not viewed that screenshot or replayed the A/B capture. The original parent, failed run and recording-on diagnostic must remain preserved. Current/release remain held.

## Confirmed source finding

The frozen parent's `Common/VtableHookVisitor.h::visit()` returns early when its compatibility member is NULL, log level is below DEBUG, and `Qfr::requested()` is false. The Device3 compatibility table leaves DrawPrimitive and DrawIndexedPrimitive NULL; Viewport3's compatibility table leaves Clear2 NULL. The ordinary methods may still exist in the real vtable, but the visitor never installs their wrappers on this path.

Those wrappers contain functional `HudAssembly::Draw`, `HudAssembly::Clear` and retained captain adapters. Therefore enabling those features/readiness elsewhere does not establish that their API adapters are reachable. Recording on bypasses the installation return, explaining why a diagnostic setting can restore functional output. This is an installation-layer dependency, not evidence that recording data is intrinsically required for QUANTUM rendering.

The narrow repair should admit the exact functional Device3 DrawPrimitive/DrawIndexedPrimitive and Viewport3 Clear2 methods for the active runtime, preserving the real-slot nonnull check and normal compatibility behavior. Do not install every NULL compatibility method, force DEBUG, or ship recording as a workaround. Rendering owns a new child; Engine will review its actual delta when available.

## Clear/world causality: limit the claim

Missing Draw adaptation directly explains original-density GPU HUD primitives. Missing Clear2 adaptation prevents private reserve/radar/preview clear mapping. The original4022A0 source places strip clear return4023F2 before main world processing and places radar/preview work later. The retained HUD Clear adapter admits callers4023F2,459107 and4027C8 with different source/destination rectangles. Thus the strip must not be described as a proved late full-world erasure.

The A/B establishes a recording-dependent symptom, and source identifies a specific missing functional hook path. It does not by itself prove exactly which clear, clipping/result behavior or ordering erased/suppressed the entire world. The narrow repair/retest or an existing concrete rectangle/result join must establish that additional causal detail. No new engine reconstruction or experiment is requested here.

## Why the offline pass missed it

The earlier source review and offline intake explicitly did not establish live production behavior. Functional transition/startup tests covered one discovered dependency, while HUD tests exercised adapters directly; they did not demonstrate that the real vtable visitor installed those adapters at INFO with recording disabled. The regression must invoke the actual visitor, then dispatch through its resulting vtable with recording requested=false and the functional feature enabled. It should prove the three intended methods reach adapters/originals once, unchanged arguments/results outside adaptation, unrelated NULL compatibility methods remain skipped, and NULL real slots remain NULL. Directly invoking an adapter or a separate eligibility helper would miss the original installation bug.

Frozen parent files and original scene evidence are pinned in `evidence/production-recorder-dependency-01/manifest.json`. This diagnosis supersedes any pending/unexercised live-production status for this failed wrapper; it does not erase earlier sealed source, fixture or different-wrapper runtime evidence. Rendering owns fix/build; Master owns the new package/retest. Engine changed no candidate code or installation and launched nothing.
