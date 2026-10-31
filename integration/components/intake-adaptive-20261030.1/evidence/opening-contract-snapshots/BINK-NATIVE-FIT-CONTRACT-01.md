# Bink video fitting: original ownership and guarded native-output contract

2026-10-24. Source assessment for Rendering; no scaler, game/launcher/Current change or game launch by Engine. Movies are separate from the successful Main/Selection control. Original engine SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`; shipped Bink DLL SHA256 `65358b3cde7281df10cb68ac8e1c9f8705893a47df33f87bb882073fd0955c90`.

**Recommended first control: one persistent owned800x600 RGB565 movie canvas, one original Bink copy per ready frame, then one fit into the already-locked native target before the original Unlock.** Keep decoder dimensions, Copy height/XY/flags, audio, Wait/DoFrame/NextFrame, skip callbacks and presentation scheduling. Redirect only the supported Copy destination pointer and its corresponding pitch1600. The separate compositor performs scaling. Reuse the validated uniform-fit implementation, with independent movie ownership/publication; never stretch the world frame or replay UI callbacks.

This is the movie portion of **one coordinated native1440 opening-flow candidate**, reusing the passing Main/Selection implementation and original-density art. It does not require a separate project or launch for each screen. This is a concrete guarded design, not an assertion that those guards already exist. Live unsampled Lock/lifecycle ownership, initial unscaled-history seeding, successful Unlock publication and movie-root input routing must be implemented/reviewed before a scaler is enabled. The companion OPENING-FLOW-ENGINE-HANDOFF-01.md supplies the existing card/Brief/Fleet/mission boundaries.

## 1. What changed in our understanding

The earlier report correctly separated source-size copying from native output, but left format, result semantics and movie roles partly unknown. Static inspection of the **actual shipped DLL**, alongside the original executable, now establishes:

- `_BinkDDSurfaceType@4` returns10 for masksF800/07E0/001F: the captured format is RGB565.
- `CopyToBuffer` can return1 **before conversion**, when late relative to its audio/timing path. It is not an HRESULT. Under another flag condition1 can also reach conversion, so a bare result1 must not be called a copied frame.
- The DLL has full and masked/selective RGB conversion paths. A transformed native target must not become the next selective copy's unscaled history.
- For the selected **positive-pitch, ordinary RGB565** path, the fourth argument is not a destination clipping rectangle or a scaler. It is read for negative-pitch address adjustment and other format paths; RGB conversion uses internal content width/height. Preserve the original argument rather than substituting native surface height.
- State8 tutorials and state9 faction cinematics attach a full-screen800x600 video widget. An800x364 movie atY118 is a letterboxed full-screen presentation, **not evidence of an embedded briefing panel**.
- The per-frame video function sets screen+6 bit2, which makes original4A79B0 choose its **Blt presentation branch**, rather than its normal Flip/BltFast branch. Keep that original policy.

These are source findings. The proportion of masked conversion actually used in the captures, complete audio synchronization and final presented pixels remain unobserved.

## 2. Source and existing evidence

`evidence/bink-native-fit-contract-01/audit.json` independently matches2,707 listed instructions in44 original-engine function exports and1,652 instructions in six shipped-DLL bodies. It pins all12 installed BIK assets and their header dimensions/frame counts, plus the current Main source and prior Resolution handoff. Original-engine exports were read-only. Bink DLL inspection used separate disposable imported projects, subsequently deleted; the DLL was never loaded for execution and the canonical engine project was not saved. Loader DEBUG messages about Windows WEVT resources do not supply Bink semantics; every used target instruction is separately compared with the shipped DLL bytes.

Engine independently verified raw hashes and scanned:

|Existing capture|Post-Copy samples|Observed groups: source prefix / XY|
|---|---:|---|
|main-menu-1440-02|39|640x480/225 frames at80,60 (12);800x600/601 at0,0 (12);800x364/6045 at0,118 (15)|
|ui-context-1440-01|473|The same groups plus800x364/2274 at0,118|

All those samples have pitch5120, format10 and result0. The current Main run raw hash is `a178f35220bab990f66316a50006f31745ba55cc20ba9b86c2160ae6bbccc324`; the older context hash is `adfde9164d17dd0d2a0daa68f3318ce5210faf91b7c0817cb58982d9726935e4`.

Engine independently checks the first current-run metadata interval: successful Surface4 Lock305 on COM410137200, returned2560x1440 RGB565/pitch5120/pointer93196320; post-Copy observation2973 matches that pointer/pitch; successful same-COM Unlock306 follows. This proves an ordered fresh-lock metadata witness, not Bink entry/end or pixel contents. Rendering's broader `native-movie-metadata-01.json` reports15 such joins and24 explicit sampled-out-lock gaps among39 observations; its inventory, not a timing guess, owns those additional joins. Header prefixes alone do not bind a handle to an asset/open lifetime. No movie pixels were captured in these native profiles.

The older approved-hybrid `qfr-gameplay-03/movie-correlation-01.json` remains a separate attributed result:7,166 snapshots,12 selected lifetime witnesses and both0/1 returns. Its native buffer was960x600/pitch1920; its added80 X policy must not be imported into the native executable. The recorder's `pitch * CopyHeight` payload is a base prefix, not necessarily the whole movie or surface whenY is nonzero.

## 3. Exact movie ownership and adjacent non-movie screens

|State|Constructor / root|Movie slot / placement|Draw and skip behavior|
|---|---|---|---|
|4, startup sequence|46AA40;4E3138/id3E9|Slot index4E47B8; initially id5C5 at80,60|46AAE0 checks55EF0 and conditionally opens later logos/Intro;46AAC0 stops media and alters sequence flags|
|8, tutorial|4723A0;4E4FC0/id444|Slot4E4FBC, id4E4FB8;80,60|472360 waits for slot completion;472380 skips/stops media; destination75F054, normally10 for tutorial selection|
|9, faction movie|472420; same4E4FC0/id444|Slot4E4FBC, id4E4FB8;0,118|Same draw/skip callbacks; destination75F054,12 for recovered campaign-intro selection path|
|5, static title/card|46AB90;4E31C8/id3E8|**No Bink copy owner**|Painter46AC60, resource101; timed/click/key transition to6|
|6, static title/card|46AD30;4E3258/id3EA|**No Bink copy owner**|Painter46AC60, resource103; timed/click/key transition to7|

The movie roots have flags0, origin0, size800x600, event mask4001, no original children; they attach under global5F7ED8. They are rectangular event regions. States4/8/9 install shared input42A210, secondary42A230, draw42A290 and no scene callback through429980. Require the actual initialized root/global topology; do not infer ownership from dimensions or CopyXY alone. Require pure ownership for a full-target movie fit: no scene, foreign root, UI overlay, second video producer or concurrent canvas pass.

46AAE0's conditional startup sequence after initial5C5 can request5C7, then5C6 withXY0,0, then5C9 withXY0,118, before pending5. Flags4E47BC/BD/BE and skip behavior determine which clips run. Do not force every registered logo into that sequence. State4 cleanup46AAB0 removes its root; state8/9 cleanup472400 removes theirs and reenables the cursor flag5F7E58. States5/6 are ordinary image producers with their own palette, timer, native clear and input; a movie scaler must not silently claim their rendering/input family.

Original media initialization453910 registers these filename identities through454640:

|Id|File|Source dimensions / frames|
|---|---|---|
|5C5|Paramount.bik|640x480 /225|
|5C6|GizmoLogo.bik|800x600 /601|
|5C7|SSLogo1.bik|640x480 /281|
|5C8|PanLogo.bik|640x480 /225|
|5C9|IntroDS9.bik|800x364 /6045|
|5CA /5CB|FedIntro.bik /FedEnd.bik|800x364 /2274,671|
|5CC /5CD|DomIntro.bik /DomEnd.bik|800x364 /3373,605|
|5CE /5CF /5D0|Tut1.bik /Tut2.bik /Tut3.bik|640x480 /3354,3611,4405|

Tutorial handler472270 chooses ids448/449/44A +186, queues state8 and return10. Selection4729F0's Federation/Dominion handlers choose5CA/5CC, queue state9 and return12. Other ending/caller policies are not exhaustively reconstructed here; retain the selected media id and return target in the owner identity. Paramount/PanLogo share dimensions/counts, illustrating why header equality is not asset identity.

**No genuine embedded/inset movie owner is established by this trace.** The four-slot scheduler could support other arrangements, but those are outside this contract. If an inset owner is later found, it needs its own destination rectangle, background layering, input and lifetime policy; it must never receive a whole-target black fill merely because its decoder size matches a full-screen clip.

## 4. Media, decoder, lock and presentation lifecycle

455C90 resolves an external media id through749020, selects/recycles one of four video slots and calls455E00. Video slot stride is24 hex: queue/current media at74A9F0+24*i, type at74AA08, handle at74AA0C and loop policy at74AA10.74AABC counts active videos;74AAC8 enables media. Confirm exactly one live type2 video slot, matching the owning widget's slot index and live decoder handle, before admitting a full-screen scaler.

455E00 opens either memory-backed40F190 or pathname40F1E0.40F190 passes BinkOpen flag04000000; DLL10007CA0 confirms that path reads the BIK header directly from the supplied memory.40F1E0 passes0 for file opening. Both original wrappers Goto(frame1,0) and unpause. Do not blindly log a memory-open argument as a filename string. Bind opening/replacement/closing to a monotonically increasing **handle lifetime**, not its pointer or header alone.

455E00 also performs the original **clear → present → clear** after a successful open. State4/8/9 constructors do a similar sequence before opening. These are native full-target4A7A70 zero-color Blt fills, not Main's4A1710 CPU solid. Preserve both sequences and their results/order. No new clear, Flip, present or Lock API call is proposed.

49E490 drains Windows messages, then calls455F40. It can run in normal input/secondary handling and the post-presentation limiter; decoding is therefore not bounded by421ECB/421F60 or a UI painter.455F40 iterates active video slots and invokes40F250. If that returns false it either restarts through40F070 (loop policy1) or closes through40F230 and advances the slot queue. Audio-only type2 slots in the other media table use40F150, which has Wait/DoFrame/NextFrame but **no CopyToBuffer**; do not route them through a video fitter.

For one ready video frame,40F250:

1. BinkWait(handle); nonzero returns true without DoFrame/copy.
2. For nonNULL handle, setsA38776 bit2 and calls BinkDoFrame once.
3. Surface4 Lock on current aliasA387B4: rectangleNULL, descriptor size124, flags1, eventNULL.
4. BinkCopyToBuffer(handle, returned.lpSurface, returned.lPitch, handle[+4], globalX,globalY,cachedFormat).
5.4A7B40 reloads the active alias from screen+44 and Unlocks it withNULL.
6. If handle frame+0C differs from total+08, NextFrame once and return true; otherwise return false.

Original40F250 checks none of the three operation results. A new fitter must check its own write eligibility without adding retries or changing the engine's frame progression. The original Unlock reload means an alias change during decode/copy is a real hazard; retaining a COM reference does not prove matching storage/Lock ownership.

With video bit2 set,4A79B0 takes primary-surface Blt using the active draw source and screen rectangle. Normal frame429330 invokes UI and then presentation; a Copy made later in the limiter can be shown by the next original presentation. Movie-root callbacks can also start a new clip and perform its clear/present sequence during UI. Do not add another presentation to force a guessed phase join. On skip, original callbacks clear bit2; subsequent presentation may return to the other branch. Observe that handoff instead of overriding it.

## 5. Shipped DLL semantics that constrain staging

All DLL addresses below are preferred-base static addresses; any runtime module reference must use its loaded base plus RVA and the exact DLL identity.

`10007920` inspects the surface pixel format. At10007A00..26 the exact565 masks return0A. Cached5DE69C is established by40EFF0 once; that initializer also sets the DirectSound backend and resets XY. Do not call it as a convenient per-frame format refresh: those are additional audio/position side effects. Verify the cached code against each fresh Lock format; format changes are unsupported until a coordinated refresh path is established.

`10008DF0` is the shipped `CopyToBuffer@28`, seven32-bit stdcall arguments. It combines call flags with handle+20 bits70030000. The positive-pitch RGB565 converters receive internal content width/height at handle+1E4/+1E8; the fourth Copy argument is not forwarded as their destination clipping height. BinkOpen initially assigns those internal content dimensions from the BIK header. High-bit modes can change public/output dimensions and conversion stepping; the DLL contains discrete modes, not evidence of arbitrary modern fitting. The first control must require internal dimensions to equal publicW/H and exclude effective70000000 modes, negative pitch and other formats. All installed assets are width divisible by4 and even height; a first implementation may retain those bounds rather than generalizing every converter.

The format10 full path calls exported16bpp conversion10012570; its masked path calls10016620. Both delegate to converters with source content geometry. Call flag80000000 bypasses the masked selection in the inspected DLL, but **changing Copy flags is not needed in the first control**. A future force-full initialization alternative needs its own side-effect/performance review; do not guess flags from a newer SDK.

The return accumulator starts0. NULL handle/destination also return0 early, so result0 alone is insufficient. A late-audio path returns1 at10009100..0B before conversion. If flag80000 is present in call or handle flags, the DLL can instead set return1 and continue converting. For the first control, require call flags exactly10 and handle+20 bit80000 clear. Under that restricted identity/path,1 is a skipped RGB copy: retain the prior valid source canvas, do not retry and do not claim a newly decoded-pixel receipt.0 on the validated nonNULL format10 path reaches conversion; it may be selective, so prior unscaled canvas contents remain relevant. Any other result invalidates fit eligibility while the original result is forwarded unchanged.

This resolves the old uncertainty sufficiently for a guarded design; it does not reinterpret every captured result1 without the previously unrecorded handle flags. The old16 result1 snapshots cannot be retrospectively classified as skipped or copied on the current evidence alone.

## 6. Smallest guarded copy/fit protocol

Use a separate MovieOwner containing current state/root, media id/slot, decoder pointer **and open lifetime**, sourceW/H/flags/XY, movie epoch, target/storage epoch, native alias and active Lock token. Keep an owned, aligned, tightly packed800x600 RGB565 canvas with pitch1600 and guards. This is the original authored video canvas, not the native 3D target or Main's active shadow allocation.

**Seed before redirecting.** The first selective copy needs correct unscaled history. With a fresh successful native Lock and fully validated exclusive movie owner, copy the native buffer's top-left800x600 rows into owned storage **before this owner has written any transformed pixels**. This reproduces exactly the buffer history the original call would update. On a later clip/seek/storage change, only seed from a proved unscaled baseline: e.g. the original full native clear successfully applied to the storage subsequently returned by this Lock, or another explicitly verified native repaint. Mere clear entry, sameCOM, a guessed black background or copying back an already-fitted frame is insufficient. If unscaled history cannot be established, keep original copying and do not begin fitting. Preserve owned unscaled history between ordinary calls; do not zero it before selective updates.

For each admitted original Copy:

1. Verify exact engine/DLL identities, expected Copy caller40F2ED, stable current=pending movie state, exact pure root/callbacks, no scene/foreign canvas/overlay, unique live video slot and matching decoder lifetime. Validate observed XY against this clip's authored placement; it must agree with5DE6A0/A4 and fit800x600. Never add the historical80 correction.
2. Match a **fresh unsampled** successful original Surface4 Lock lease on the same thread/alias. Require original Lock caller40F2C4, NULL rectangle/event, flags1, descriptor124, actualRGB565 masks, positive pitch at least2*nativeW, bounded full allocation and required native dimensions. The passed destination/pitch must equal that returned descriptor. Cached image words, a previous frame's pointer or sampled log proximity cannot authorize writes.
3. Require enabled5DE6A8, cached format10, nonNULL live decoder, positive bounded publicW/H, internalW/H equalpublic, effective scaling modes absent and handle80000 clear. Require X+W<=800 andY+H<=600 without overflow, along with buffer alignment/extent and source format guards. For the initial native1440 control, retain the reviewed native executable/target identity rather than broadening untested output modes.
4. Call the previous real/import-hook chain **once** with `(sameHandle, owned800x600Canvas,1600, sameOriginalCopyHeight, sameX,sameY,sameFlags)`. CopyHeight must still match handleH as the original engine supplied. Do not change decoder/header fields, global coordinates, sound backend, frame counters, Wait/Do/Next/Goto, return value or LastError. The destination pointer and its real row stride are the only changed Copy arguments. Preserve the existing hook chain so observation wrappers do not recursively call themselves or invoke the codec twice.
5. After return0, keep the updated unscaled canvas. After restricted return1, keep the previously valid canvas and label any display as repeated source history, not fresh conversion. Recheck all owner/epoch/target/Lock guards after the original call; the DLL can wait and service its internal audio path. Never read or write a native pointer after its matching lease ends.
6. Fit this authored800x600 canvas into the already-locked native buffer using the existing rounded uniform-fit implementation. Write the owned full-screen movie output and black margins through that CPU span. This is composition under the original Lock, not an added DirectDraw clear or whole-game stretch. Keep the owned source canvas unscaled for the next selective copy.
7. Mark a pending display candidate, then let40F250 perform its **original**4A7B40 Unlock. Only successful matching Unlock on the same alias/storage/epoch can publish transformed movie input/display coherence. A Copy-hook return is too early for interactive publication. If Unlock fails, is missing or targets another alias, quarantine transformed movie input and reject the candidate. Original presentation remains responsible for showing it; successful write/Unlock still does not prove scanout.

The native Lock may hold a larger pitch; copy seed rows using that actual pitch, and owned rows using1600. Preserve guard regions. The positive565 code's use of CopyHeight does not limit writes to `pitch*CopyHeight` at base whenY>0. Diagnostics must capture the actual owned800x600 span or complete video ROI, not repeat the old prefix-only assumption.

The counterexample in `selective-update-counterexample.json` shows why post-Copy in-place scaling without source history fails: scaling `[1,2;3,4]` to4x4 leaves top-left2x2 `[1,1;1,1]`; the next selective update of its bottom-right pixel yields `[1,1;1,9]`, rather than `[1,2;3,9]`. This is a synthetic architectural example, not an observed Bink frame or a claim about which particular capture used the masked path.

## 7. Fit policy, skip input and fallback

**First control policy: uniformly fit the original800x600 presentation canvas.** That preserves intentional80,60 logo margins and0,118 cinematic letterboxing, aligns with the Main/Selection fit geometry, and resamples video once. At2560x1440 the canvas occupies[320,0,2240,1440): a640x480 logo at80,60 maps to[512,144,2048,1296), and an800x364 cinematic at0,118 maps to[320,283,2240,1157), using shared rounded edges. This changes display scale while keeping original pixel data and authored relationships.

A separately named future policy may fit a cinematic's decoded aspect directly to the full native display. That can use more widescreen area but changes the original padding relationship. It is a presentation choice, not an inherent capability unlocked by changing pitch or destination height. Do not infer or crop encoded black bars without asset evidence. Derive either policy from runtime output dimensions; do not create per-resolution coordinate patches or refit an already-rounded rectangle with different rounding.

Movie widgets receive event0 and keyboard eventE through the original shared dispatcher. Startup46AAC0 clears the video presentation bit, stops media through454A00 and can disable an optional subsequent logo.472380 queues75F054, clears the presentation bit and stops audio/video slots. Natural completion is recognized by55EF0 during the root draw; it can queue a destination or open/clear another clip. Preserve these callbacks once, their original event consumption and audio consequences. Do not substitute a new Escape-only handler or independently close the decoder.

For pointer skipping to follow the fitted authored canvas, route only the two exact movie-root rectangle predicates through that MovieOwner's **committed** fit/inverse and the original41DADC/41DC62 continuations. Current=pending owner, root topology, input scope, handle/storage lifetime and display publication must match. Keep global mouse coordinates physical and leave keyboard dispatch unchanged. Fit margins may remain misses under this faithful authored-canvas policy; making the entire physical window skip is a separate explicit policy. Integrate routing through the existing shared predicate dispatcher; do not install a conflicting second detour at41DB02 or borrow Main/Selection's generations. Before any scaled movie write, original input can retain original behavior; after a failed/uncertain scaled write, original small hitboxes must not be silently reenabled.

**Fallback must respect unscaled history.** Before any fitted output, unsupported calls pass through exactly once with original arguments. After fitting, blindly sending the next selective copy into a transformed native target is unsafe. A supported recovery can restore the owned unscaled800x600 history and its native background under a fresh valid full-screen Lock before returning to legacy copy, or wait for a verified original native clear/repaint/new owner. It must have the same full-target ownership and bounds guarantees. If those fail, make no speculative native write, retain original decoder/audio progression, invalidate publication and report unsupported recovery. Never call Copy again, add a Flip, read freed storage or pretend interface equality proves visual retirement.

Close/recycle/seek, source/XY/format change, concurrent video, owner/pending/root change, focus/device/Restore/release, storage uncertainty, foreign thread/reentry, missing Lock/Unlock and coverage loss revoke relevant leases before writes. Distinguish decoder/source-history lifetime from target-storage lifetime: a target Restore does not itself corrupt a separately owned valid source canvas, but it invalidates its old native Lock/publication. Unvalidated seeks/loops need explicit history preservation or reseeding; first constructors request nonloop playback, so general looping/embedded-video/focus-stress recovery remains outside the initial control.

## 8. Concrete hook/ABI surface and next verification

The already-installed typed `_BinkCopyToBuffer@28` interception is the principal conversion boundary:

`int32_t __stdcall copy(void* handle, void* destination, int32_t pitch, uint32_t height, uint32_t x, uint32_t y, uint32_t flags)`

At entryESP, words+4..+28 are those seven arguments; return isEAX, callee pops28 bytes. Original call40F2E7 is `ff15fcd24b00`, return40F2ED. The original Surface4 Lock call40F2C1 is `ff5164`, return40F2C4. Original unlock-helper call40F2F2 is `e849880900`; that helper reloads screen+44. Keep stdcall/COM conventions, flags, nonvolatile registers, LastError and the original return; do not guess an HRESULT from the Copy result.

If a per-frame engine context hook is necessary,40F250's first five bytes are `a0a8e65d00`; a register/flags/x87/SSE-preserving entry bridge can observe the original handle atESP+4 and replay those complete bytes. It is optional if exact caller plus an unsampled Lock/slot registry supplies equivalent ownership.40F070 and40F230 provide original restart/close boundaries; record opens/replacements and close-before-free with explicit lifetime ids. A typed Open observer must distinguish memory/file input; no decoder ABI beyond pinned/read-validated fields is assumed. Do not replace return addresses or call engine functions with guessedC signatures.

Maintain the live Lock/Unlock and full-native-clear ledger **before metadata sampling**, independent of NDJSON retention. An api counter group or last pointer cannot substitute for a valid transaction token. Reuse existing native COM instrumentation where possible; no extra Surface Lock/GetDesc/Unlock/Present is necessary merely to obtain a descriptor already returned by the original call.

Before a frozen scaler review, meaningful mock checks should cover selective updates with persistent history, result1 without conversion/no retries, header/geometry/format/negative-pitch rejection, seed refusal after transformed history, allocator/guard failure, two slots, handle reuse/close/seek, matching and mismatched Lock/Unlock/alias/storage epochs, original hook chaining once, skip/natural completion mid-owner, separate movie/UI generations, and integer fit/margin input. Preserve Main/Selection regression tests. These are proposed checks, not new executed scaler tests.

The coordinated revision can admit **states4/8/9 through the same known full-screen-owner strategy**, rather than separate movie projects. The agreed opening flow uses startup state4 and campaign-intro state9, then Brief/Fleet and the first mission; tutorial8 shares the mapped strategy but need not be visited in that playthrough. Keep explicit frame timing and original audio call counts. Static cards5/6 use the ordinary screen strategy in the companion handoff. Genuine inset video, other codecs/formats, scaling flags, arbitrary concurrent media, focus stress and every presentation backing member remain outside this bounded scope. Existing native metadata can seed the design; a new logger launch is not needed just to rediscover the first Lock witness. Live fitting/pixels, decoder flags/internal dimensions, exact open-lifetime binding, unsampled transaction guards, audio continuity and skip/input/presentation behavior still require evidence from the later implemented combined control.

Deliverables: this contract; original/shipped-DLL audit and synthetic history counterexample; `QREQ-0025-bink-native-fit-contract.json`; sealed `validation/bink-native-fit-contract-01-manifest.json`. Rendering owns any later implementation. No game, launcher, Current, frozen Main/v5 source or new agent was changed/started by Engine.
