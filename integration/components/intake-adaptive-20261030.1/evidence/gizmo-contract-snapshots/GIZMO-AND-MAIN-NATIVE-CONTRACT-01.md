# Gizmo video and Main background: bounded source handoff

2026-10-27. Rendering remains sole implementation/build/install/launch owner on tested opening-cards-source-v1. This is an existing-source contract and implementation recommendation, not code or live acceptance. No Engine game/codec/build execution or unrelated UI reconstruction.

## 1. Gizmo5C6 uses the existing startup movie owner

Original GizmoLogo.bik is BIKi800x600,601frames,rate2997/div100=29.97fps; nominal video duration60100/2997=20.05338672seconds. SHA256 `c80976eacec03c9dbe032b46524aaa99f2686d155bb846b34711e1a67d003e7e`,4,455,744bytes. Its one44100Hz track has original12-byte metadata `001c020044ac00e000000000`. Preserve this movie's packets/metadata/timing, not Paramount's225frames/15fps/soundtrack.

46AAE0 retains state4/root4E3138/id3E9,draw46AAE0,input46AAC0 and slot index4E47B8 throughout startup. When the current slot completes, it conditionally consumes flags in this order:

| Flag | Original request |
|---|---|
|4E47BC|Clear flag; queue5C7.|
|4E47BD|Set global movieXY0,0 via455F20; clear flag; queue**5C6 GizmoLogo**.|
|4E47BE|SetXY0,118; clear flag; queue5C9 IntroDS9.|
|None|Set pending5, entering the publisher card.|

These are conditional original branches, not instructions to force every logo.5C5 begins in46AA40 at80,60;5C6 is a later clip at0,0, not a new state/widget or a separate MassMedia slot.46AAC0 clears the video-presentation bit, calls454A00 to stop media and clears only4E47BC when set. Do not replace this with “skip the whole opening” or clear BD/BE yourself.

Reuse the accepted native movie source-history/Lock/Unlock/seek/lifetime protocol. Extend its explicit profile table to distinguish resource5C6,601,2997/100,originalXY0,0 and the exact new asset. Runtime slot media is the internal index resolved by DWORD[749020+4*5C6]. Open occurs before slot publication; bind the handle lifetime to the resource once published. Do not admit by matching dimensions alone or retain5C5's225/15/80,60 guard. Original40F250 and455F40 already use decoder frame count/completion; no new engine timer is needed. Preserve Wait/Do/Copy/Next and original skip/close/open-failure progression, including following Intro/cards. A new clip invalidates prior publication even though the root/state and potentially reused handle address match.

Private source output remainsXY0,0/pitch5120 at2560x1440, with full8000000A until first actual conversion0, then selective0A/history. Restricted result1 repeats prior valid history and never initializes an empty one. Keep the original result/call count; no catch-up copies. Extend early asset/readiness refusal to each installed HD profile; a5C6 header must never fall into old native copying merely because5C5's profile does not match. Existing FullMovieViewport skip policy can be reused for matching16:9 output, bound to the committed live profile/lifetime.

Movies/Textures supplied `cinematic-extraction/original-gizmo-hd-v1/GizmoLogo-1440p-original-audio.bik`,SHA `c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851`,76,069,912bytes,2560x1440/601/exact2997/100. Their independent-handle/oneCopy-per-frame receipt reports601 equal frames,0skips,intact guards,exit0 against the shipped DLL; audio packets/metadata and independently decoded PCM match the original. Engine checked receipt/file identities, not codec execution. The supplied fade/faint final MassMedia image is preserved in the encoded source according to the asset owner; do not add an engine fade or trim the ending. Audio output/Wait/presentation/skip remain live-unvalidated for5C6.

## 2. Main201 must remain a background role

Main7/root4E47C0/id405,flags84,logical800x600,draw46ADD0,inputNULL.46AED0 loads group200,initializes native-sized global UI,attaches six children and assigns root palette200 atroot+8C. Preserve the constructor and original blocking/container flags. Palette200 is zero-step/flags0/range0..255.201 is a clean800x600 indexed background with disk flags3;46ADD0 forces `(flags & ~2)|1`, making it opaque/clipped. It resolves through49E1E0(global7138EC,root+50),where root+50=201. The native-width predicate calls4A1710 at46AE15 before CPU background call46AE5E/return46AE63. Tagged8000 sources have the original40F630 alternative; the passed CPU-only parent does not certify it for the new layer.

The same painter then selects the temporary font-color ramp with41E580,draws version font1404 at700,580,and restores5F7F88 via4A2CF0. Children46B440 draw the210 existing180x100 rawP8 flags3 animation images, using original palette and counters; activation completion9999 can create a modal or queue departure before the final frame is drawn. These producers are foreground. Keep immediate original-trampoline shadowing with the then-live4EC390 RGB565 lookup; defer neither palette conversion nor callback execution. The detailed ABI/scratch/clip rules remain MAIN-MENU-CANVAS-CONTRACT-01 and FONT-CONTRACT-01.

**Recommendation: add independent foreground coverage to the passed shadow mechanism.** The actual parent MenuCanvas/CanvasRuntime.cpp already mirrors201,button frames and glyphs into one flattened800x600 RGB565 image. Keep that original shadow for validated colors/fallback, but distinguish the background producer from foreground writes. At final valid Lock, compose the immutable native background, then map shadow colors only through foreground coverage using the existing800UI fit. No need to replay widgets or reconstruct their animations.

- Exclude only the admitted Main201 background producer and its expected original native solid from foreground coverage. Require their existing identity/clip/full-repaint witnesses; do not globally suppress all opaque images or solids. Native solid flags0/AND0/XOR0 has inactive/uninitialized pixel fields: never dereference it as image data or mirror its native extent into800storage.
- For admitted rawP8 source flags3, coverage follows nonzero source indices and exact original clip/origin. Zero-index transparency is tested **before** mask/xor/palette conversion. For flags1, every actual clipped write contributes, including index0/black. All six button banks are flags3 and overlap; rectangles, RGB565-black keying and before/after pixel differences cannot establish foreground coverage.
- Reset coverage for each fresh Main repaint and pair it with the original shadow's resulting colors. Preserve actual source writes/order and source-pitch/shared-scratch restoration. Unknown formats/GPU/unobserved writes must reject the new composition, not silently leave incomplete foreground. The original supported snapshot path is the fallback after a proved fresh original repaint.
- Coverage is visual only. Keep original overlapping rectangular hitboxes, traversal priority, timers/capture/release and the existing UI inverse. A native16:9 background does not imply FullCardViewport or pixel-mask input for Main buttons. Background raster density and UI geometry are separate.

The supplied Main PNG was visually inspected as a clean background with no baked buttons/version text. Its native image plan should remain separate from the existing foreground800fit/input plan. No original palette fade is requested by200; retain all original palette operations for foreground/fonts rather than treating indices as direct RGB.

## 3. Main-modal composition needs the same coverage contract

The actual parent OpeningFlow.cpp owns global-root completion for Main-backed Preferences4DB6F8 and Load4DF100. Modal root.next points to Main;421E80 paints next siblings first, so Main redraws before the modal. Pure Main CanvasRuntime admission rejects this changed global head; the existing outer OpeningFlow path instead snapshots/fits the completed800composite. That snapshot also contains201. Neither stamping a new background over completed Main nor fitting the old full composite over it preserves the intended layers.

Reuse **one foreground capture protocol** across pure Main and the outer Main/modal pass: include fresh Main buttons/version plus actual modal writes/opaque panel coverage, while excluding201/native Main clear. Compose native background followed by that logical foreground at the single owner-completion phase. Do not seed logical modal history from top-left800pixels of an already-native background/frame; those are different coordinates. Transparent holes in modal resources expose the composed underlay; black foreground pixels must remain opaque where the source wrote them. Additional source types require supported coverage or safe legacy fallback, not a speculative alpha reconstruction.

Retain existing mainEvent's causal9999/expectedModal receipt. A modal may be attached during the final activation draw but not painted in that traversal. Preserve the final Main visual once, then admit the new modal on its next real paint; do not replay its constructor or synthesize a missing draw. Likewise preserve proven departing Main frames to10/18/15/20 with input disabled. Do not let two independent Main and outer-modal composers overwrite each other or publish different generations.

Keep original native clear, present count, cursor/tooltip ordering and cleanup/resource-boundary invalidation. Before any new write mark transformed-storage provenance; failures revoke input. Only a fresh original complete repaint can authorize legacy snapshot fallback after native composition; never reinterpret fitted/native pixels as unscaled source history. Main group cleanup46AF70/49DE60 ends borrowed resource/palette leases even if static addresses repeat.

## Bounded next check

Rendering can implement this within the two existing producer/completion paths. Verify coverage with transparent holes/nonzero black/overlapping animation and version text, then one combined live check of5C6 natural audio/ending/skip/relaunch and Main idle/hover/activation/Preferences/Load/return/quit. Keep native background detail and foreground/input registration evidence separate. Do not broaden to unrelated UI, arbitrary movies or all source formats before this candidate.

Pins and static checks: `evidence/gizmo-main-contract-01/audit.json`; script `scripts/audit_gizmo_main_contract.py`; manifest `validation/gizmo-main-contract-01-manifest.json`. Original/Current/accepted sources remain unchanged.
