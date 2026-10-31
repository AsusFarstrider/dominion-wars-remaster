# Production label/startup: bounded initial source review

2026-10-28. Rendering's `production-runtime-work-01` derivative is incomplete; S&S and HD buttons are still separate unfinished work. **One concrete recorder-off blocker was returned to Rendering and its source correction subsequently observed: functional transition hooks had been disabled with recording.** No shipping approval, build or runtime acceptance is implied. Engine modified no implementation and ran no game/build/codec.

## Blocker and reviewed boundaries

Initial reading found `Runtime.h::feature(transitions)` returning `config.recording`. With recording off, `MenuTransition::install()` returned before installing its hooks. Those hooks contain functional operations: frame-entry index0 calls `HudAssembly::prepareFrame()` (the sole caller), which establishes HUD/scene/preview/picking geometry; installer index4 calls `OpeningFlow::resourceBoundary("state_install")`. The initial readiness mask could reach31 without either operation. Rendering received the exact source path and a request for a bounded recording-off check, without new reconstruction.

**Observed correction before this receipt was sealed:** managed transitions now remain enabled with recording off; successful transition-hook installation sets required bit32 and full readiness requires63. Both functional actions run once before the recorder-enabled diagnostic return; their old copies inside the capture branch were removed. The actual-observer fixture exercises repeated recording-off entry/install calls with no routine diagnostic events, while the policy fixture rejects readiness without transitions. Engine inspected these case bodies, not their execution. No additional blocker was identified in this bounded follow-up. Snapshots record the corrected source; the initial finding remains documented here rather than being presented as an archived old worktree.

The other reviewed startup seams move in the intended direction: config/version/engine/codec/movie identities load before the early HD gate; managed initialization precedes the first DirectDraw export; UI/canvas/opening/HUD features no longer require QFR environment flags. Managed disabled-DDrawCompat refuses initialization. API argument/result callbacks to OpeningFlow occur before recorder gating, so native Lock/Unlock leases are not inherently lost when recording is off. HD Open remains behind source-header/profile/owner/provider readiness and original Close on rejection. This source assessment does not establish successful cold start or completeness of the forthcoming profile table.

The label provider substitutes only the original font call's stack text argument at return46AEB9, before the original trampoline emits glyphs. It reads the pointer at4EC3B8, then the30-byte state; it retains the live original palette/style and does not redraw the root or change a global text pointer. Its ownership checks include Main state/root/painter/resource, current destination, original text and bounded runtime glyph metrics. The new text has stable managed storage. The reviewed happy path does not require a new font or additional draw.

## Exact original1404 metrics

`Remastered 1.10`, original font014/004 (decodedSHA256 `7333b1a9b17c30d08ac23127ebc0bf61f4bc493bcd9907c7a145803ecc8f5381`), tracking1/baseline0:

- Signed low-AX measured width: **59 logical pixels**.
- Starting pen700,580; final stored left-aligned penX **760** when ordinary advance is enabled.
- Union of glyph metric rectangles: **[700,580,759,589)**. These are rectangles, not a computed nonzero-ink bounding box.
- Per-character starting pens:700,704,708,714,718,722,726,730,734,738,742,747,750,753,756.

The dot at pen750 has originX1, hence rectangleX751; the space has width4/height1 and still advances5. These details matter for a synthetic test carrying the real metrics. Width is `sum(tracking+originX+width)-tracking`; do not confuse final pen760 with a60-pixel reported width.

## Minimal actual-resource witness

**Postlookup site46B540**, immediately after46B53B calls49E1E0: EAX is the returned source descriptor; ESI is the button node; the original prehook stack still holds archive argument atESP and actual selected resource argument atESP+4.46B55A removes those two arguments. EDI retains the prelookup resource, but the stack argument is preferable because it includes any original high-resource remapping. For these six banks no remapping is triggered.

This witness occurs after final9999 handling and therefore records the actual activation image despite changed flags. Bind a one-use, thread/pass/target-owned receipt to the subsequent raw blit at46B579/return46B57E and its identical source; invalidate on gap/foreign operation/owner exit/resource boundary. Do not re-derive the bank from postcallback flags. A generic49E1E0 entry witness could observe the same resource arguments with return46B540, but lacks the returned descriptor until paired; the postlookup site supplies both with one observation. Preserve registers/flags/FPU/SSE and displaced instructions;46B540's first instruction is4bytes, so a5-byte detour must cover the next complete instruction as well. No timer/callback changes or return-address rewriting is needed.

Corrected source snapshots, verified original instruction witnesses and reproducible font calculation: `evidence/production-label-startup-review-01`; script `scripts/audit_production_label_startup_review.py`. Later changes require a separate receipt; preserve the initial finding above for provenance. Rendering owns implementation/tests; Master owns integration/version/package records and the release hold.
