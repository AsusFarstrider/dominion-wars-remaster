# True-HD Paramount proof: Resolution output and input contract

2026-10-26. Bounded coordination recommendation only. Rendering is the SINGLE implementation/build/install/launch owner; Engine supplies original/native copy/lifetime policy; Textures supplies the asset and shipped-decoder evidence. No code, executable, asset installation or game launch by Resolution.

## Pins and first scope

Preserve accepted HUD source `E:\Codex\research\dominion-rendering\hud-assembly-source-v2`, manifest `af8001718c1116a666bb3d4d6b17e8e343d210b6ed1bd7ec130fbd70f1d6bc4d`, wrapper `dc4bf2c4cb1148c2aeb5286358fec57860d184061351ab878d8e2b11b145a6b2`, native EXE `de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6`. These are the tested parent identities from HUD-ASSEMBLY-RESULT-01, not newly approved HD artifacts. Original shipped Bink DLL is `65358b3cde7281df10cb68ac8e1c9f8705893a47df33f87bb882073fd0955c90`.

Textures' audible original-audio640x480 control is `e3bf0a35a3402994f441b624b63459330702d66d6a5cb99eebe11ba413ab33df`. It is the audio/control reference, NOT the HD asset identity. First new asset is Paramount5C5 only,2560x1440,15fps,225frames, retaining exact original compressed RDFT packet assignment. HD asset hash and prepared output code/EXE/source/profile pins must be supplied before installation. Do not infer them from the640 control or source MP4.

## Coordinate domains

Keep separate:

1. Decoded video source bounds0,0,SWidth,SHeight and persistent unscaled RGB565 history at source resolution.
2. Runtime native target and current returned Lock pitch/format/allocation/lifetime.
3. Physical destination content rectangle from video aspect-fit.
4. Original logical movie widget/skip root800x600 and original event/callback state.

For first HD asset, source history is2560x1440, positive pitch5120,7,372,800 pixel bytes plus guard/alignment allocation. Decoder copy destinationXY is0,0; no original80,60 padding. Those values describe the new declared source buffer, not monitor hardcoding. Actual decoder public/internal dimensions, output format and supported flags must be checked independently. Source-sized history is not recreated from previously fitted native pixels.

## Runtime-derived aspect fit

Reuse `dw::ui::Canvas::fit({0,0,SWidth,SHeight},{0,0,targetW,targetH})` and the established shared-edge rounding for destination bounds. Do not use fit800x600, rescale an already rounded movie rectangle, stretch16:9 into4:3 or silently crop source bars. Source artwork aspect comes from the declared encoded raster; any crop/content aperture must be separately explicit.

At source2560x1440,target2560x1440, scale is exactly1 and contentRect=[0,0,2560,1440). Each source pixel has one destination pixel: no low-resolution ROI, decoder downscale flags or intermediate800-sized conversion. A direct row copy or identity composition is sufficient. On another runtime target the same formula is valid aspect-fit, but that is a separate scaled presentation result, NOT a2560x1440 1:1 proof. For the first experiment, reject/declare a mismatching target instead of claiming true1440p from an upscaled screenshot.

The existing geometry header is address-free C++17 and already used by Rendering. RGB565 Plan::fit accepts arbitrary validated source size; check actual limits/overflow/allocation before enlarging history. Nearest/integer partition sampling is an accepted identity proof at scale1; filtering quality at nonidentity scales is a separate choice.

## Native target and presentation

Use the actual current target from the original engine ownership chain and original full-surface Lock result, not desktop size, window placement, cached image pixels or a retained COM pointer alone. Require matching native dimensions, RGB565 masks, positive pitch>=2*targetW, full writable span and unchanged owner/lifetime through original Unlock. Target pitch can exceed5120; copy source rows with source pitch and destination rows with the returned pitch.

Movie playback uses a CPU span under that Lock. It does not need a new D3D viewport or a scene/HUD projection change. A native viewport getter alone therefore proves nothing about the video raster. Preserve native scene/HUD/UI layout and original presentation order. Verify the actual final presentation source/destination rectangles and wrapper/application scaling: the source-to-display chain must not secretly route through800x600, a small primary, stretched640 movie, resolution-scale factor or display-fit enlargement. Final screenshot dimensions alone cannot establish this.

Record first-frame and representative later receipts: source public/internalWH, source bufferWH/pitch/XY/format, exact copy caller/token/lifetime, destination LockWH/pitch/format, video contentRect/scale, fit identity flag, Unlock result, and original presentation source/destination extents. If pixel/readback evidence is available, retain a frame/region detailed enough to compare source-derived detail at1:1. A successful Lock/Unlock is write/presentation-submission evidence, not scanout certification.

## Clears, history and failure bounds

The existing800x600 movie history and its legacy restore routine must NEVER receive an HD header/copy. All allocation, row stride, copy height, sourceXY, guard, legacy/history restore and fit paths must be generalized together. The source-sized persistent history handles selective updates and restricted result1 repeats without feeding fitted output back into decode. Preserve original Copy call count, Wait/DoFrame/NextFrame, audio, natural completion and close lifetime. Do not seek/retry/copy twice to manufacture a fresh frame.

For a full native identity frame, clear/fill only the owned full-screen movie target as required by original presentation/history policy. For nonidentity aspect-fit, define the entire destination including margins under the existing valid Lock; no new Flip/Present or scene clear. Distinguish source history lifetime from presentation storage/device lifetime, including backing allocation changes behind stable COM.

Critically, an unsupported HD admission CANNOT simply call the original copy with legacyXY80,60 or the old800 history/pitch1600. Even a native2560-wide target is insufficient whenX+sourceWidth exceeds it. Engine must define a safe bounded decode/progression or rejection path before the HD asset is installed. Failed/uncertain writes revoke publication BEFORE they can restore original-small input; retain sticky quarantine/epoch behavior and matching original Unlock. No speculative restore from a transformed native allocation.

## Skip-input policy

Do not scale the game's800x600 root into a2560x1440 source buffer or repurpose its dimensions as decoder limits. Recommend explicit policy for this HD proof: pointer within the physical *video contentRect* may take the original movie-root hit continuation, preserving original disabled/modal/capture/release and event0 behavior; points in physical letterbox margins remain misses. This is an owner-scoped boolean admission for the exact movie root, not a generic uniform800x600 inverse (which would incorrectly create4:3 side bands on a16:9 video). If there are actual child/custom input consumers, they require their own contract; do not apply this shortcut to arbitrary descendants.

Keyboard skip dispatch and original callbacks remain unchanged. Bind pointer eligibility to the successfully published video owner/source lifetime/target/epoch after matching Unlock. A Copy return or source-header match is not enough. Preserve original stop/audio/menu-transition behavior; don't add a second Escape handler or independently close the decoder. Natural end and skip must both restore a fresh normal Main publication, not carry movie contentRect/generation into menu/HUD hit testing.

### Original callback verification and full-window policy clarification

Read original Engine export `exports/bink-native-fit-02/targets/0046aac0/instructions.json` and the corresponding decompilation.46AAC0 clears presentation bit2 atA38776, calls454A00 once, conditionally clears optional-logo flag4E47BC, and returns. It does not read pointer coordinates, widget dimensions or callback arguments. Source therefore supports routing an eligible exact-root hit through the existing dispatcher without synthesizing800x600 coordinates for this callback. Original event/mask/keyboard dispatch remains the authority for when it is called.

472380 is a different callback: it queues75F054 and stops audio/video slots. Do not substitute startup46AAC0 across other movie owners. These source bodies prove side effects, not every keyboard/mask eligibility rule; Engine retains that dispatch contract.

For source2560x1440 on target2560x1440, contentRect equals the whole native target: all physical pixels inside[0,W)x[0,H) should be eligible for exact startup-root pointer skipping after publication. In particular X0..319 and2240..2559 must not remain dead bands from the old800x600 fit. This is an exact-root physical hit test with original41DADC/41DC62 continuation, not the old logical rectangle contains() test or a global mouse rewrite.

Rendering additionally proposes full-native-window click-to-skip including any future aspect-fit letterbox bars. That is a reasonable explicitly named `FullMovieViewport` input policy, independent of picture fit. It supersedes the content-only margin policy above only when selected for this HD owner; it must not silently change accepted legacy movie/UI behavior. First matching16:9 proof has no letterbox pixels and therefore cannot validate the policy difference. Use actual committed target bounds, reject outside-client points, preserve modal/disabled/release gating, and return to fresh Main input after original stop/close.

## First proof gates and test

1. Textures: exact HD header/packet/audio identities; exact shipped DLL225-frame decode with source-sized RGB565 guards intact, sourceXY0/pitch5120 and no implicit scaling; original RDFT packet assignment pinned. Offline decode alone does not prove game presentation/audio.
2. Engine/Rendering: chosen contained native-first seam or source-sized OpeningFlow extension, but ONE implementation only. Preserve latest parent chain; no old complete EXE overlay. Add exact identity-copy bounds/history/failure/skip generation tests, including invalid legacy-offset rejection and no unsafe pass-through.
3. New frozen source/build/artifact and disposable session, HD asset installed only after safe copy/history/fallback path is ready. Actual runtime target must be2560x1440 for this1:1 claim; Current and accepted parent remain untouched.
4. Live natural run first: full detailed16:9 image, audible original soundtrack/sync, normal15-second ending and correctly working Main. Capture actual source->native output/presentation receipts. Then repeat separately for pointer/keyboard skip and menu continuity; retain normal exit and integrity/error results.

No higher frame-rate, other movie asset, broad GPU compatibility, codec replacement, HUD relayout or general media framework is required for the initial2560x1440 proof. If native engine changes require invasive global dimensions/ABI changes, the existing bounded owned-history/lease extension is an acceptable fallback, provided it demonstrably remains source-sized end-to-end.
