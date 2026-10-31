# Ordered Main sprites, S&S and production gates: source review

2026-10-28. Bounded review of14 actual `production-runtime-work-01` source files. **No normal-path ordered-composition blocker was identified. One concrete startup portability issue was returned: the mandatory ledger is created beneath the game installation, with a PID-only CREATE_NEW filename.** This is source review, not built/live acceptance. Rendering owns corrections/build/fixtures/tests; Engine changed no implementation and ran no game/build/codec/fixture.

## Ordered foreground and replacement shapes

The layer selects ordered capture when all six button banks are available. Original button CPU drawing still occurs, including into the private reference raster, but those old button pixels are not appended as raw replay commands. The native replay instead appends the matching immutable RGBA frame. Other raw producers own immediate RGB565 colors and independent source-index write masks. Thus an unpainted/transparent replacement pixel exposes the replayed underlay, not old button ink; zero-key modal/glyph holes also avoid copying old shadow contents. Opaque raw black remains a write.

Commands retain original encounter order, so Main/version/buttons precede later modal operations. There is no final button batch above the modal. RGBA sampling accumulates weighted premultiplied color and alpha before blending with the current RGB565 destination, avoiding transparent-RGB fringes. The full180x100 rectangle determines source mapping, while a separately mapped logical clip bounds output;720x400 is source density only. Original 800x600 fit/input remains separate. The existing rounded-edge inverse matches raw command rectangle partitions.

When native background art is disabled, the exact original201 raster is copied immediately after its original background draw and fitted before ordered foreground. Later reference-raster button pixels cannot contaminate that saved background. Unsupported producers, unreadable spans, bad witnesses or limits reject the layer. Composition marks `nativeWritten` before native writes and reuses the existing matching Lock/Unlock, gap retirement and single inner/outer publication safeguards; no stale top-left snapshot fallback after native writing is introduced.

## Actual selected-resource witness

MainButtons installs only after complete immutable bank loading and the exact8-byte gate at46B540. Its bridge observes original registers/stack before displaced instructions: EAX source, ESI node, archive/resource arguments still atESP/ESP+4. It validates original node/id/parent/painter/bank parameters and the current layer, then records scope/node/resource/source. Consumption requires the same owner thread, one-use receipt and exact46B57E/source; foreign observation invalidates admission. Final activation therefore uses the real postcallback resource, with no new timer/index/callback reconstruction. Original descriptors and hit geometry remain unchanged.

This review does not establish that the catalog is the user's final approved button artwork. Six immutable hashes/35 sequential resource IDs per720x400 bank constrain the loaded bytes; Textures and the user own the final asset decision. The current installer logs an explicit original-art fallback when a bank fails admission.

## S&S and startup

The profile table now distinguishes5C7/281frames/30:1/originalXY80,60 from Paramount and Gizmo. Managed movie hashing admits only the original or explicitly permitted restored SSLogo1; the no-opt-in oversized-header loop includes all three installed movies. Early Open/Close/Goto safety hooks remain ahead of managed functional startup; provider/resource/lifetime/Lock/history gates remain in the Copy path. No second conversion, presentation or sequence callback was added by this delta. Recorder-independent transitions and readiness63 retain the previously reviewed correction.

**Ledger issue:** `Runtime.cpp::openLedger()` creates `.dw-remaster/logs` under the game directory and treats creation/open failure as fatal managed-load failure. A protected/read-only installation consequently cannot start. In addition, `rendering-PID.ndjson` with CREATE_NEW can collide with an earlier run after PID reuse. Rendering was asked to use a per-user writable log location and a unique per-launch name/retry. The exact reviewed source with this issue is preserved; a later correction needs its own follow-up receipt.

Rendering reports239 ordered/alpha/resource checks,356 opening checks,48 runtime/font checks and7 recorder-off transition checks. Engine inspected the actual ordered and S&S test bodies, not their execution or the changing build. OrderedPaint tests cover alpha holes, owned palette-black writes, later-modal occlusion, clipping, fractional outputs, source detail and one-use/foreign resource receipts; these are not full live MainBackdrop/old-shape or hardware acceptance. The next isolated combined test remains Rendering's responsibility, with the release hold intact.

Exact source snapshots/hashes: `evidence/production-ordered-main-review-01/manifest.json`. This review adds no campaign/HUD reconstruction, Current edit, launch or publication authorization.
