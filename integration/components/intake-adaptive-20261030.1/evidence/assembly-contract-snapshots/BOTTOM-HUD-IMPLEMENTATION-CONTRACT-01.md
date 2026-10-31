# Bottom HUD: bounded source contract for Rendering

2026-10-25. Companion to PRE-HUD-ARCHITECTURE-REASSESSMENT-01 and the accepted captain retry. Read Resolution SHARED-HUD-LAYOUT-INTERFACE-06. Static review only: no executable/HUD edits, new Ghidra export, game launch or Current change.

**Recommendation:** use one shared layout generation, retain original-native HUD source coordinates and callbacks, transform private primitive/clear outputs, and inverse-map private HUD input. Publish physical scene geometry before processing. Extend the passed mixed-primitive captain implementation; do not introduce a flattened mission/HUD screenshot layer. This contract is a proposed implementation boundary, not runtime acceptance of the combined HUD.

## Confirmed owners and reusable behavior

Addresses below describe original static records; live ancestry, resource-filled dimensions and dynamically replaced child tables remain authoritative.

| Root | Original draw / input | Relevant behavior |
|---|---|---|
|4E5E90, id29|47DFE0 /47DEC0|Preview overlay; painter reads preview width5FF6D8. Input body47DEC0..47DECC acts on selected ship, without direct mouse reads.|
|4EA588, id2|497890 /NULL|Additional sibling: flags104,178x99, baseBC01. Actual draw body497890..49790B calls421D70, then tinted BC3A/CD3A at local95,18. Include its validated descendants. The font at497A2F belongs to the separate497910 callback.|
|4E5F20, id45|47B8E0 /NULL|Sprites and direct font4A2650. Child template4E61F0 uses input47D660.|
|4E5FB0, id46|NULL /NULL|Captain subtree; accepted normal/paused mixed GPU/CPU trial. Auxiliary controls extend above its nominal51-pixel height.|
|4E6040, id44|47C6B0 /NULL|Masked child families4E69D0/4E6790 use47C850; actions can replace child tables.|
|4E60D0, id2D|47BD10 /NULL|Crew/status children4E65E0/4E6670 use47BC40, draws47BD60/47C050.|
|4E6160, id43|47CFB0 /NULL|Masked4E72D0 uses47D2D0; actions can replace child tables.|
|4DF678, id65|NULL /NULL|Radar subtree: circular4DF708/id64 input45A360; scan4DF798/id62 input45A430, draw45A5D0.|

The checked ordinary action windows are ID/event driven; they do not justify recreating command callbacks. Rendering's preserved inventory observes hardware sprites and raw8-bit flags3 glyphs across the observed bottom roots. The original source independently confirms direct font calls. Preserve original metrics, wrapping, palette and masks; that observed format coverage is not certification of every faction/state.

## Private input boundary

41DB02 rectangle admission is too late for circles and must not bypass image masks. Candidate earlier lease:41DA93 (`8B4604 A804`, resume41DA98). At that point ESI is the current node; private cached parent-relative X/Y are [ESP+14]/[ESP+10], parent X/Y [ESP+34]/[ESP+38]. Reconstruct absolute physical point, apply the committed owner's inverse, then subtract the original parent origin. Restore both locals at common next-node boundary41DCFC, including miss/disabled paths. Handle recursion/reentrancy and hidden bypass explicitly; do not transform subsequent unrelated siblings. Disable the previous rectangle-only inverse for leased nodes.

Let original dispatch continue: flags2 circle uses node XY and radius node+0C; image-sensitive400000 chooses the current resource/state and rejects transparent pixels; modal blocking, disabled/capture/release and event order remain original. Pre-hit eventD/mask2000 occurs earlier and is outside this lease: validate actual participating masks. World-input root4DC530/id63, modal roots and global native mouse remain outside HUD inversion.

Radar has additional direct reads that need the same source-absolute inverse, scoped to its committed input role:

| Consumer | Exact global-load sites / destination |
|---|---|
|45A040|45A06D CX=X;45A086 CX=Y;45A138 EDX=X;45A13F ECX=Y|
|45A210|45A258 AX=X;45A270 AX=Y|
|45A360|45A3A0 EAX=X;45A3A7 EDX=Y; paired14 bytes resume45A3AE|

Preserve upper register bits for AX/CX writes, signed extension for MOVSX, live registers/flags and x87 state. These are instruction contracts, not inferred C callback signatures. 45A430's checked body is event/action driven. Do not globally spoof mouse coordinates.

## Explicit coordinate domains

| Keep physical | Keep original-native source values |
|---|---|
|5FF670/674 outputW/H;5FF694 reserve;5FF698 scene bottom;5FF6A4/A8 sceneW/H;5FF6B0/6AC halves; world widget4DC530 bounds; projection4F0308; four viewport records4F01A8/D4/0200/022C.|Packed HUD roots/descendants; radar5FF6BC radius,C0/C4 center,C8/CC/D0/D4 edges; previewD8/DC W/H,E0/E4 XY. Store canonical/source baselines and actual anchors in the owned snapshot.|

**5FF698 is not HUD-only storage.**408630 uses it as a bottom clipping limit and writes clipped object bounds;439EF0 uses it to size world input;42B690 limits direct scene indicators with it. Keeping it at H-154 after changing physical reserve is inconsistent. Conversely, globally making preview/radar globals physical breaks original source painters and construction arithmetic. Mixed domains require the following explicit output adapters:

- Radar4590C0 builds its own Clear2 rectangle from source edges; original call459104 needs a mapped private rectangle. It manually computes screen positions from center/radius, calls431300 and4AD160/4AD3F0/4AD230, and delegates contact helpers. These are additional raster producers, not ordinary widget sprites. Map private screen-space outputs once; a mapped viewport does not scale pretransformed vertices.
- Exact output chain:4AD160/4AD230 submit FVF1C4 through4A5F10, whose immediate route reaches Device3 DrawIndexedPrimitive at4A600D;4AD3F0 submits through4A5E00, immediate DrawPrimitive at4A5EE6. These checked paths do not perform raw CPU pixel stores. Both submission helpers also have a deferred branch (context+AC nonzero), storing vertex pointers in192-byte commands with callbacks4A61E0/4A61B0. A final Device3 hook scoped only to the4590C0 call stack can therefore miss deferred work. Prefer private copied vertices at owner-scoped submission, retained through original queue consumption, or carry role/generation through deferred replay. Do not double-transform the immediate Device3 route.
- Radar stores contact positions (e.g. object+1AC/+1B0) and associated hit rectangles consumed by45A040/45A210. Retain that model/cache in source coordinates if input is inverse-mapped. Transforming stored rectangles as well would mix domains. Helpers generating the rectangles are not exhaustively reconstructed here; validate this invariant at the existing output boundaries.
- Preview4022A0 constructs its clear directly from E0/E4+D8/DC, independently of4F0200. Map the private clear; render its actual scene through the mapped native viewport. Keep preview overlay source geometry and transform it once. Do not rescale already-native preview output with the widget adapter.
- Preview projection needs isolation: the checked4022A0 path sets the main projection, then enters radar and preview without selecting a separate preview projection.402880 processes vertices (COM call402A09) before deriving screen positions. A reserve-driven main P00 change can therefore affect the preview. Proposed policy: use a private captured pre-reserve native projection for the uniformly enlarged preview, binding it before preview processing and restoring main state after the pass. Keep persistent4F0308 physical/main for picking; update processing caches through the same state path. This is not a runtime-validated preview repair.
- The existing strip Clear2 still needs the shared reserve top and full intended right edge, preserving original call/flags/order. Radar intentionally overlaps the scene above the154-unit reserve; preserve original HUD input priority there.

## Prepare and publish before processing

429330 frame entry is the smallest promising existing barrier: original calls primary input42934F, secondary42936C, scene429389, UI4293AB. Prepare before all four, not at final draw submission. Scene globals, world-input bounds, viewports and processing/picker projection must agree before any preprojected vertices are generated. Keep physical outputs stable for late input and paused rendering; retain source geometry separately.

Initial source readiness is after479C60 at42B315 and459F20 at42B31A, return42B31F. Capture actual source anchors/resource geometry then. Configure/recreation invalidates readiness; recompute after original writers, never adopt previously overridden values as canonical. 429030 allocates/initializes surfaces and fills viewport records: do not call it per frame. The initial-ready point is not proof every later reset path is covered.

Derive projection from a pinned canonical matrix/aspect, never multiply yesterday's result.401070 originally writes P00 at40117A and P11 at401184; native1440 modified P00. Retain depth/camera terms and the declared vertical-FOV policy.4070C0 reads the same scene dimensions/halves and supplied projection for picking. Fullscreen stays native. Publish input only with a successfully completed matching rendering generation; readiness and presentation publication are distinct.

## Bound the remaining uncertainty

Proceed with the user-approved combined candidate. Its practical hazards are radar cache/raster domain consistency, overlooked siblings/direct producers, dynamic child replacement, mask/circle fidelity, paused rendering and stale publication after recreation. Keep notification/world-label owners separate. One joint1440 check can exercise HUD controls, radar/scan contacts, preview alignment, world selection/rotation, overlap priority, Escape/resume and cursor cleanup. No exhaustive HUD reconstruction or per-button research campaign is prerequisite. Other factions, general resizing and dormant/editor consumers do not inherit this bounded source review or the captain acceptance.

Evidence: original SHA25603c20a35…be6a; existing401860/429030/479C60/4022A0/4070C0/41D9F0 exports; bounded original-byte windows and static records in evidence/bottom-hud-contract-01; source pins/checks in validation/bottom-hud-contract-01-manifest.json. Bounded windows can include adjacent routines: exact body claims above deliberately stop at verified returns.
