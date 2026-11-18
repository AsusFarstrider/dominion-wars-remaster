# One native1440 opening flow: existing Engine boundaries and focused ABI answers

2026-10-24. Source handoff to Rendering's **single combined wrapper candidate**. Keep the native executable/dimensions, approved launcher behavior, Current and frozen passing Main/Selection unchanged. Reuse existing geometry, original-density artwork and the shared hook/owner registry. No game launch, new agent or implementation by Engine; no comprehensive new ordinary-menu exports were added for this handoff.

## Reuse map

|Opening owner|Existing state / node / callbacks|Small coordinated extension or limit|
|---|---|---|
|Startup logos/cinematic|4;4E3138/id3E9; constructor46AA40, draw46AAE0, input46AAC0|BINK-NATIVE-FIT-CONTRACT-01: persistent unscaled800x600 MovieCanvas, original Copy once, fit under original Lock, publish after matching Unlock.|
|Static cards|5/6;4E31C8/id3E8 and4E3258/id3EA; constructors46AB90/46AD30, draw46AC60, input46AC40/46ADC0|Ordinary completed-screen capture/fit family with no child-table requirement. Resources101/103, palette/group100.46AC60 has original native clear when background narrower than wrapper, forces opaque background, then timed pending6/7. Preserve draw-driven transition/input separation; this is not Main's event9999 animation.|
|Main|7;4E47C0/id405;46ADD0/46AFA0, children46B440|Reuse passing frozen Main control; native clear exception, palettes, animation and causal10/18 departure remain as reviewed.|
|Mission/faction selection|10;4E5050/id43F;472630/4729F0|Reuse passing Selection control and original broad hitboxes. Federation441 queues9/media5CA/return12; Dominion440 queues9/media5CC/return12.|
|Faction cinematic/tutorial|9/8;4E4FC0/id444;472420/4723A0 constructors, draw472360/input472380|Same MovieCanvas strategy as startup, with exact slot/asset/return ownership. These are full-screen roots, not established inset briefing video.|
|Briefing|12;4C7AB0/id3EB; constructor415550, painter415B10, input415EC0; objective helper4158D0|Completed native800x600 capture/fit is a practical original-density control if source-history and ROI guards hold. Original backgroundB101/C201 is submitted directly at0,0; no forced-opaque/full-native-clear sequence is present in its inspected entry. Preserve body clip4C7F38 and objective rectangle415A5E.|
|Fleet|11;4E3378/id406; constructor46C560, painter46F030, input46CC40|Same completed-screen strategy if the actual no-world output lies in800x600. Dynamic heap children exist.137 historical direct image/font/wrapped-text sites and ten clips are already inventoried; no new per-site offset patch is needed when reading the completed source region.|
|Loading/first mission|1/42B040 setup, then3 with scene4022A0 and UI42A7B0|Loading has widget4DA738/draw434700 and direct updater434560, including constructor-time painting. Retire menu/movie ownership before setup/recreation. Preserve native scene/projection/world picking; native small HUD remains an explicit limitation unless separately covered.|

State installer429980 and cleanup429FB0 remain original phase/lifetime authority. Current4D0A24 and pending4D0A28 differ during callbacks; actual previous is4D0A20. Source root flags for cards/video/Brief/Fleet are base0, not Main84 or Selection4. Live Brief/Fleet roots have hover20000. Do not paste Main's exact flag/children predicate over these families.

Original global5F7ED8 is native-sized, origin0, flags4. Keep all native world/layout globals and all four viewport records intact. Widget parent/child geometry supplies logical coordinates; the fit is an owner-specific output/input relationship, not a global change to screen size.

## What the existing capture actually supplies

Engine reread `ui-context-1440-01/capture.sqlite` without another game session. Live ancestry confirms:

- Brief4C7AB0, draw415B10/input415EC0, logical0,0,800,600, child4C7E10 in the retained snapshot.
- Fleet4E3378, draw46F030/input46CC40, logical0,0,800,600, with a dynamically allocated child in the retained snapshot.
- Root-attributed producer samples: Brief29,587 software-image entries and3,673 font entries; Fleet1,391 software-image and158 font entries. No hardware-image entry was attributed to those **root callbacks** in that scan.

This narrows the practical extension. It does not prove that every descendant/helper/preview is CPU-only, identify every target, or place every output pixel inside800x600. Avoid inventing a generic GPU rewrite merely because the source has alternate image branches. Conversely, completed-target snapshot requires fresh wrapper synchronization of any GPU work that actually occurs.

Reuse Resolution's `OPENING-FLOW-EXTENSION-PLAN-03.md`, `NATIVE-UI-ARCHITECTURE-HANDOFF-01.md`, and frozen history builders as coordinate/owner inventories. The plan's older phrase “briefing inset movie” is superseded by the Bink full-screen-owner reconstruction; no genuine inset video has been established here. Historical+80 translations/caves are provenance, not new patches for this native executable.

## Focused clear and Unlock ABI

**4A7A70 original zero fill:** ECX is the screen object; one stackDWORD is color; helper ends `RET4` at4A7AB5. It allocates a100-byte DDBLTFX, writes `dwSize=0x64` and `dwFillColor` atFX+50, loads `[screen+44]`, then makes the COM call at4A7A99 (`ff5114`). The actual Surface4Blt caller/return is **4A7A9C**.

The six COM stack arguments are `(this,destRectNULL,sourceNULL,sourceRectNULL,flags0x01000400,fx)`. For the movie-opening clear, used color is0. Other unusedFX members can be uninitialized; do not reject or dereference them as active data. Match the actual successful Blt HRESULT, object, used flags/FX fields and target/storage interval. A helper-entry or nominal full-target rectangle is not a receipt for every presentation buffer.

**Movie Unlock:**40F250 calls helper4A7B40 at40F2F2, outer return40F2F7. The helper reloads `[screen+44]`, pushesNULL and that object, then calls vtable+80 at4A7B48 (`ff9180000000`). The actual COM Unlock caller/return is **4A7B4E**, args `(this,NULL)`. Qualify that object against the still-active original Lock token; a changed active alias must not close/publish the old candidate merely because an Unlock happened.

For reference, original Lock COM caller is40F2C4; Copy import caller is40F2ED. The typed Copy is seven32-bit stdcall arguments and returnsEAX/pops28. The audited Bink contract supplies result/format/history restrictions. Keep original clear→present→clear and subsequent presentation branches; no extra codec call, clear/Flip or return-address substitution is required by these ABI facts.

## Completed-root boundary: what is included and excluded

421ECB starts a node's draw callback after palette selection. Record the **actual owner node**, thread and currentESP. Matching421F60, before its three pops, completes that traversal invocation after the node callback and descendants, including415B10/46F030 when they are the registered root painters. Do not finish at421EDE: that is the callback-return boundary before child drawing. At421F60, ESI may be the last child, so matching only `ESI==ownerRoot` would miss valid completion; retain the entry owner/ESP token and subtree/topology checks.

421F70 processes pending confirmation creation through423760 **before** traversal, resets font effects/palette tracking, calls421E80, then may call tooltip422030.42A290 subsequently draws **native cursor43B140**, then ends UI graphics state. Thus matching-root completion precedes tooltip and cursor, as required for the proposed snapshot.43B140 is not the tooltip. Correcting the earlier shorthand: **421F70 does not directly call4A1660**; that deferred surface helper is a separate path (the existing export identifies429030 as a caller). It must not be given an invented fixed position in the ordinary traversal.

Taking the source snapshot at that boundary must use an actual fresh RGB565 Lock whose read-modify semantics synchronize prior GPU writes. Copy800x600 into distinct owned memory before fitting back, with actual native pitch and allocation bounds; never read and rescale overlapping source/destination in place. Callback completion alone is not GPU completion. A renderer-specific preview or constructor helper outside this subtree is not magically included because the screen has no world-scene callback.

## Source history is the important additional guard

Rendering's completed-native-region strategy is smaller than redirecting GPU/CPU targets and preserves original mixed producer order. It works when the source region is truly defined at the original logical density. **Copying first avoids overlap during that one operation; it does not by itself prevent next-frame feedback.** The fitted destination overlaps the native top-left800x600 source area. Original partial redraws, transparent backgrounds or cached imagery can subsequently read/leave those fitted pixels.

For Brief/Fleet, retain a verified **unscaled**800x600 source history and restore it before original partial painting, or establish complete original replacement of every source pixel for that pass. The inspected entries415B10 and46F030 submit their backgrounds without Main's forced-opacity step; full opaque coverage cannot be assumed. Seed a new owner from a known native/unscaled baseline, not the previous owner's fitted frame. Coordinate transitions/constructor painting with that retirement or unscaled-history handoff; do not clear after native preview/scene work. This is the same bounded history issue as the Bink selective-copy path, not a request to reconstruct every menu.

If an original full background/clear and all content are demonstrably rebuilt in800x600 each frame, history restoration is unnecessary for that family. If a cached preview is produced elsewhere or outside the source ROI, record that actual exception and use the small scoped producer extension or leave that element unsupported. Native layout's preview position can depend on actualH, so the mission preview record is not evidence that Fleet's visual preview is inside the ROI. Do not globally move all viewports or resample the world to hide a missing preview.

## Ancestry, helpers and input scope

`owner.next==NULL` alone is insufficient. Additional state can exist in:

- Its+73 descendant tree, including heap widgets, callback-created controls and changes in cached-next traversal.
- A new global modal head inserted before the owner; owner.next can stayNULL while its previous link changes.
- Direct body/objective clips, font/palette state, cached image/resource helpers and independent map/preview consumers, without their own widget next links.
- Confirmation/tooltip/cursor and constructor/progress painting outside the chosen root-completion interval.

Require the global head and owner previous/parent relation, bounded ancestry, descendant origin accumulation, actual source/pending/resource/target generation and callback ownership. This allows a shared owner registry, not six-button hardcoding or a universal list of every possible ordinary menu widget.

The common rectangular insertion remains41DB02 with cached parent-relative point at siteESP+14/+10 and parent arguments+34/+38. Preserve physical mouse globals and original41DADC blocking/disabled handling,41DC62 miss cleanup, capture/release and sibling order. A broader ancestry test **must still check predicate type**: hit→41DADC bypasses the later400000 pixel test and is valid only when that extra test is absent. Circular paths branch before this hook; custom tactical-map picking/drag logic may use other coordinates. Do not label those covered by the rectangle inverse. Native keyboard events continue unchanged. Record any real unhandled path in the agreed opening playthrough rather than expanding into exhaustive menu reconstruction first.

Keep visual completion separate from strict interactive current=pending eligibility. The original loop may still paint the old owner after input or a draw timer requests its destination. Extend causal render admission for known opening transitions while topology/resources remain live; do not silently accept every pending value or skip original callbacks. Stop full-screen menu/movie fitting before loading setup and the first native mission scene. The actual native world picker4070C0, mission scene viewport4F01A8 and native cursor remain outside the UI inverse.

## Deliverable and practical next step

Use one candidate and one opening-flow acceptance path: startup movies/cards → Main → New Game → Federation/Dominion selection and intro → Briefing scroll/objectives → Fleet ship/captain interaction → first mission → normal exit. Retain Main/Selection as the regression control, without a new project for each screen. The already-pinned old builders identify local clips and producers; the new completed-region strategy avoids transplanting their offsets.

Source/ABI support here is sufficient to build a bounded prototype with explicit unsupported outcomes. It does not prove custom input, preview placement, source-history restoration, GPU synchronization or live fit results. Preserve durable logs of owner/epoch, actual producer bounds, original and fitted input routes, snapshot/restore/Lock/Unlock results, exceptions and the first unsupported condition. Targeted ABI/lifetime questions can then be answered from these existing anchors. Current remains untouched.
