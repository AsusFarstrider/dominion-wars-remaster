# Paramount1440: bounded native movie-copy design

2026-10-26. User authority relayed by Movies/Textures: test true1440p Paramount, coordinate with Rendering/Resolution, prefer a contained engine-native repair, preserve Current/accepted copies. **Rendering confirmed sole implementation/build/install/launch owner.** Movies/Textures owns the source asset, exact original compressed RDFT soundtrack and offline shipped-DLL decoding; Resolution checks native geometry/presentation/input. Engine supplies this static contract only. No implementation, codec execution or game launch here.

## Decision and evidence

**A contained repair is plausible.** Original40F250 does not allocate an800x600 movie framebuffer: it locks the existing draw target and lets the codec copy into it. Native1440 already supplies a2560x1440 target. Fixed startup placement80,60 comes from46AA40; fixed800x600 history/fitting is the current adapter's policy. Neither is evidence of an inherent800x600 decoder/engine limit.

First candidate: a5C5-only provider at the original movie-copy seam, with source-sized owned RGB565 history and direct1:1 native composition. Extend the existing proven lifetime/Lock/Unlock provider; no replacement renderer, decoder, graphics enhancement or full40F250 rewrite is justified. Installation through the existing wrapper/runtime module is an implementation location; distinguish this engine-callsite repair from wrapper GPU upscaling. No existing arbitrary-resolution QUANTUM movie scaler has been established. General output fitting remains a separate bounded adapter function.

First asset profile:2560x1440,15fps,225frames, original compressed audio packets/metadata. Movies/Textures supplied the62,576,052-byte asset SHA256 `5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3` and exact shipped-DLL offline receipts. Size is the intended new variable; the earlier audible640x480 remux pass does not certify live HD playback.

## Exact engine seams

| Original point | Contract |
|---|---|
|46AA40|Sets global movie placement80,60 via455F20 and requests external media5C5; retain later logo/Intro sequence.|
|455C90 /455E00|External5C5 resolves through DWORD[749020+4*5C5]. The live74A9F0+24hex*slot contains that internal index, not5C5.455E00 opens before publishing slot/handle, then original clear→present→clear.|
|40F190 /40F1E0|Memory/file Open paths, flags04000000/0; then Goto1 and unpause. Bind a monotonic handle lifetime and explicit open intent; do not read memory-backed Open input as a filename.|
|40F250|Wait40F269, DoFrame, original Surface4 Lock call40F2C1 (return40F2C4), Copy CALL40F2E7 through IAT4BD2FC (return/caller token40F2ED), original Unlock via4A7B40, original NextFrame40F301. Preserve ready/not-ready/frame-end decisions and one-call cadence.|
|4A7B40 /4A79B0|Unlock reloads current alias; match the actual lease. Original video flagA38776 bit2 selects native Blt presentation. Keep that policy and original present count.|
|455F40 /40F070 /40F230|Scheduler completion, loop/seek and close own lifetime changes. Startup skip46AAC0 and cleanup46AAB0 remain authoritative.|

The replacement is Copy destination/pitch/private XY and, while initializing history, the reviewed full-copy flag. Leave global placement, public/internal decoder headers, CopyHeight, frame counters, audio backend, Wait/Do/Next/Goto and scene/menu geometry unchanged. Other movies retain their existing provider.

## Source buffer and conversion

Allocate guarded, suitably aligned RGB565 storage before exposing the admitted HD handle: pitch5120, height1440, payload7,372,800bytes (7.03125MiB). Validate overflow-safe products, positive pitch, width divisible by4/even height, public W/H=internal+1E4/+1E8=2560x1440, allowed handle flags, exact asset/DLL/profile identity and copyHeight1440. This allocation is a design requirement, not measured total decoder memory or a resource-budget claim.

Call the original import chain once with `(handle,ownedHistory,5120,originalHeight,0,0,flags)`. Original80,60 is authored placement for the old640x480 image, not an HD buffer origin. Even on a native2560x1440 target it would reach2640x1500 and exceed the allocation. CopyHeight is not a positive-pitch RGB565 clip height; reducing it cannot make the old copy safe.

Shipped DLL10008DF0 checks CALL flag80000000 at10009172 to bypass masked conversion and select full RGB56510012570. Its converter1000CFB0 treats geometry-mode bits70000000 separately. **Proposed initialization:** use8000000A until the first conversion return0, then ordinary0A selective copies into persistent source history. Do not seed HD history by resampling/copying the old800x600 canvas or the already-fitted target. The asset owner's history receipt reports225 converted frames, no skips, matching first-full/subsequent-selective versus full outputs and intact guards. Engine checked the receipt, source and asset identities, without executing the codec. That comparator makes two copies on one handle per decoded frame, with no audio backend/Wait pacing; it is not the normal runtime cadence. A cheap independent-handle comparison was requested to strengthen this precise limit; no new reconstruction is required.

Full-copy flag does not bypass the earlier audio-lateness return1. With call/handle80000 absent, result1 precedes conversion: retain valid history; if none exists, withhold first image publication. Never treat a skipped copy as initialized history, add another copy to catch up, or add Do/Next calls. Under a valid nonNULL supported conversion path result0 reaches conversion; a null-destination refusal also returns0, and must be separately labelled. Other return/flag combinations remain unsupported.

Own history per decoder lifetime, independent of native surface storage. Keep it across output restore/recreation; invalidate output publication/lease instead. New Open, source geometry change or original seek/loop restart requires fresh full initialization. Close/skip cancels publication and retires the buffer after any active copy's owned reference returns. Do not hold an expired native Lock pointer.

## Native output, input and refusal

Compose only through a fresh matching original successful RGB565 Lock, same alias/thread/storage epoch/descriptor/pitch, before original Unlock. At2560x1440 source/output, copy5120 bytes per row directly; no800x600 round trip. For a different admitted target, uniformly aspect-fit source edges into native output and define margins, using an explicit movie plan. Do not silently broaden first-candidate output support. Recheck owner/lifetime/epoch and canaries after codec return; publish only after matching successful original Unlock.

Keep the HD display plan separate from the shared800x600 menu plan. Original movie root4E3138 remains800x600 and has its own skip events. Give the verified full-screen movie a declared native-screen skip region/private input mapping, including margins if that is the chosen full-screen policy; do not let the old4:3 fit exclude the sides of a16:9 movie or compare source-HD coordinates directly against an800x600 root. Preserve keyboard skip, original callbacks once and normal later-logo/card/menu transitions. Global mouse remains native.

**No HD fallback may call the old native-destination Copy at80,60.** This applies to unsupported metadata, wrong thread, reentrancy, exceptions, lost owner/lease and failed initialization, including branches before the current sameThread check. Preflight must refuse launch unless the exact safe provider/profile is installed. At selected HD Open, metadata/allocation failure should close the just-opened handle once and returnNULL through the original open-failure route; bind open intent before slot publication where necessary. Unknown HD opens must not fall into legacy copying.

Original failure route is confirmed: OpenNULL→40F190/1E0 returns0→455E00 fails→455C90 returns−1→46AA40 stores−1. Bounded455EF0..455F1B returns done=1 for a negative slot (455F04..06), so46AAE0 follows its existing next-logo/card transition. An early Open gate can be independent of later UI-code readiness. If the Open safety hook itself cannot install, the launcher must refuse; an uninstalled guard provides no protection.

If known-valid owned source storage remains but output admission fails, a single ordinary codec copy into that storage can preserve decode progression while withholding native writes/publication. If source safety itself cannot be established, quarantine/fail the run.40F250 ignores Copy's EAX result, so a private−1 refusal without native writes is a possible explicit error policy, not a forwarded codec result. Alternatively original Copy with NULL destination returns0 before handle dereference/conversion in this exact DLL; if used, label refusal/no-conversion. Neither is a successful frame or an audio-preserving normal path. No retry, fabricated successful conversion or guessed dimensions. Refusal behavior is part of required tests, not accepted playback.

## Bounded verification before one candidate launch

1. Asset owner: pin HD asset/header/audio packets; exact shipped DLL guarded5120-pitch/XY0 decode; first-full+selective versus all-full outputs where conversion occurs,225frame completion/canaries. Offline audio decode is not live audible acceptance.
2. Rendering: pure/provider tests for initial skipped copy, persistent selective history, output loss/reset, repeated lifetime/seek, malformed header/overflow/allocation failure, disabled/wrong-thread/reentrant/exception branches, no legacy fallback, matching Unlock publication and full-screen skip region. Prove original call counts/return forwarding on the normal path.
3. Verify isolated candidate/source/EXE/DLL/asset/opt-ins before launch. One actual1440 playback should check true source density, full-frame aspect, audible soundtrack/synchronization, ending→next movie/menu, skip and subsequent launch/reset continuity. Keep receipts bounded; do not enable a large pixel recorder by default. Original audio timing is retained, not presumed accepted because video completed.

Rendering can implement this bounded provider without waiting for further reconstruction. If direct native composition is unreliable, retain the same source-sized history and ownership contract in the existing wrapper fitting path; do not revert to a low-resolution history or replace the decoder. No Current/adopted-source mutation is authorized by this design note.

Source pins/instruction checks: `evidence/paramount-1440-design-01/audit.json` and `validation/paramount-1440-design-01-manifest.json`. The earlier asset-only restriction remains correct for the unchanged path; this note proposes the newly authorized code change.
