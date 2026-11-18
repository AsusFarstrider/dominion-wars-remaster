# Shared runtime HUD layout interface: bounded recommendation

2026-10-25. Design/source handoff only. No implementation, game launch, test-install change, competing patch or Current modification. Rendering owns implementation. Read HUD-CAPTAINS-RESULT-02 and frozen HudCaptain/Core.h; reused existing401860/429030/479C60 findings and original integrated HUD builders.

## Decision

Keep the proven owner-scoped mixed-primitive captain adapter, but replace Transform::fit's temporary free-strip placement/independent scale with a transform supplied by ONE immutable runtime HUD layout snapshot. The same snapshot must supply every HUD group, scene reserve, minimap/preview viewport and associated input context. Do not finalize separate free-space anchors for each panel.

Logical artwork/layout dimensions, source texture density, physical UI scale and native3D render dimensions are four different things. No new asset-density contract is implied.

## Same-day Engine refinement: authoritative domain split

Read `E:\Codex\research\quantum-engine-kb\docs\BOTTOM-HUD-IMPLEMENTATION-CONTRACT-01.md` after this recommendation. Its explicit original-consumer map refines the broad seam table below:

- Physical scene outputs include5FF694 reserve AND5FF698 scene bottom, not only5FF6A4/A8 and halves.408630 clips object bounds against5FF698;439EF0 sizes world input from it;42B690 clips direct scene indicators. Update all of these from the shared physical scene generation before processing.
- Keep original-native HUD widget/source anchors, radar radius/center/edges5FF6BC..D4 and preview size/origin5FF6D8..E4 in the owned source model. Do not overwrite them with physical mapped coordinates:479C60/47DFE0 and radar construction/cache consumers still need source values. Four actual viewport descriptors and private clear/output vertices are physical adapters, not permission to globally replace those source globals.
- Capture source readiness after479C60 and459F20 at42B31F, then prepare the shared physical outputs at429330 frame entry before original input/secondary/scene/UI processing. Do not call429030 per frame or recapture already overridden values as canonical.
- Add sibling4EA588/id2 (draw497890, original178x99) to the shared assembly alongside captain/status/orders/preview/radar roots. Auxiliary bounds may exceed nominal widget height; use actual validated producer/descendant bounds.
- Radar4590C0 is not merely a native viewport: it constructs a source Clear2 at459104 and pretransformed output via4AD160/4AD230/4AD3F0 ->4A5F10/4A5E00. Their deferred command branch can replay after the owner stack has unwound. Transform private vertices once at scoped submission with retained lifetime/role generation, or carry that ownership through deferred replay. A final Device3 draw hook scoped only by the live4590C0 stack can miss it. Keep contact caches/hit rectangles source-local for inverse input.
- Map the preview's private clear from sourceE0/E4+D8/DC separately from4F0200. Its original processing inherits main projection: isolate a captured pre-reserve native preview projection before preview vertex processing, restore main projection afterward, and keep persistent4F0308 main/physical for world picking. This policy needs live confirmation; a mapped viewport alone does not preserve preview shape.
- Generic rectangle-only inverse cannot cover circle/pixel-masked HUD nodes. Engine's proposed earlier private-local lease is41DA93..41DA98, restoring locals at41DCFC and preserving original shape/disabled/capture dispatch. Radar direct coordinate consumers45A040/45A210/45A360 additionally need scoped source-absolute inverse. The detailed byte/register and pre-hit mask contracts remain in Engine's handoff; global mouse stays physical.

These refinements supersede any reading of the table below as globally making radar/preview source globals physical. They preserve the one shared layout policy and the single combined candidate/test scope; no competing implementation is proposed.

## Proposed interface (not an existing engine structure)

Use a versioned `HudLayoutSnapshot` containing:

- generation, surface/device lifetime token, outputW/H, faction/resource/layout revision;
- common rational uiScale, horizontal assembly offset, logical bottom reference600;
- physical sceneRect, physical reserveRect, actual HUD visual/interactive bounds;
- named groups: sourceRoot/sourceAnchor (actual engine coordinates), canonical logical anchor/bounds, destinationAnchor, clip and inverse-transform contract;
- radarRect, shipPreviewRect, sceneProjection policy/parameters and logical input roles.

Treat validity/publication as a transaction: the rendering primitives, rectangles/clips, CPU glyph adapter, input predicates, viewports and scene picking consume the SAME generation. A changed surface, layout resource, faction geometry, reserve or scale invalidates the prior snapshot; do not mix an old captain inverse with a new radar or scene viewport.

Rendering's tested Transform already represents sourceAnchor(ox,oy), destinationAnchor(dx,dy), rational scale(n/d), rect/vertex/inverse. Supply those values from the layout instead of calling its free-strip fit. Keep its proven palette/CPU/GPU handling and paused/cursor fixes. Geometry/input edge rounding remains shared; vertex sampling/source UV density stays independent.

## First policy: preserve original800-wide assembly and bottom anchoring

For the first joint HUD control, use one original-style800-wide assembly centered at the physical bottom, common `s=min(W/800,H/600)` (or an explicitly selected smaller common scale). Define half-open edge mapping:

`X(u)=rounded(left+s*u)`
`Y(v)=rounded(H+s*(v-600))`

Here left centers the scaled800-wide assembly. Derive endpoints once, not X plus an independently rounded width. At2560x1440 fit scale12/5 yields left320; this is a policy example, not a new validated HUD result. Do not force the menu scale if it leaves an unusable scene on another aspect; choose/clamp one common scale against an explicit minimum scene extent, not per-group free space.

Canonical group positions are original logical positions, including mutable resource dimensions. Actual engine roots may already contain nativeH-derived coordinates. Therefore use `physical = mappedCanonicalAnchor + s*(primitivePosition-actualSourceAnchor)` per owner. Do NOT apply Y(v) to a native1286-based root as though it were logical446, or add ancestor origins twice. Keep original widget source coordinates and callback geometry intact; adapt rendered primitives/private input at their owner boundary.

This centered original assembly is the smallest coherent layout control. Spreading left/right groups to widescreen edges or rearranging panels can be a later shared policy. No requirement to make the captain trial's current position part of that design.

## Reserve is154, not the complete visual height

401860's original scene uses H-154. With initial mutable P=WORD4DF684=165, Q=WORD4DF686=170 and preview insetWORD4E5FBE=51, the original800x600 coordinates are:

| Region | Original logical rectangle |
|---|---|
| Scene | [0,0,800,446) |
| Radar/panel | [0,430,164,600) |
| Ship preview | [164,497,324,600) |

Radar thus extends16 logical pixels ABOVE the reserved154 strip. Preserve this deliberate overlap initially; do not derive reserve170 merely from the tallest panel. At12/5, reserve top is mappedY446=1070 (rounded); radar top mapsY430=1032. The overlap becomes38 physical pixels by edge mapping. Actual resource values must be read/recomputed;165/170/51 are original initial values, not universal faction constants.

Define sceneRect initially [0,0,W,reserveTop), reserveRect [0,reserveTop,W,H). HUD visual/input bounds can extend above reserveTop. Native scene input must honor original HUD/modal priority in that overlap rather than claim the whole scene rectangle is clickable through radar. Empty gaps beside the centered assembly require a declared original-compatible input policy, not silent hit clamping.

## Exact existing ownership seams

| Seam | Reuse / required coordination |
|---|---|
|401860|Original layout arithmetic stores nativeW/H5FF670/674, sceneW/H5FF6A4/A8 and derived halves5FF6B0/6AC. Replace its final policy outputs under a reviewed hook or feed a separate snapshot; do not temporarily call it at800x600 and leak globals.|
|429030|Produces four44-byte D3DVIEWPORT2 records: scene4F01A8, radar/panel4F01D4, preview4F0200, fullscreen4F022C. Update each from its role, not one common rect. Fullscreen stays nativeW/H. Reapply after original configure/recreation writes.|
|479C60|Consumes same layout values to initialize packed widgets and faction resource families. Historical479CA5 and479CE4 moved absolute values into widget locals; shared layout must distinguish actual source roots from canonical destination anchors. Snapshot after resource/layout initialization; avoid blind+80/sub80.|
|459F33|Historical radar/scanning parent local-origin correction. Include scan buttons in radar owner transform, not just the visible map texture.|
|4023A9 / exact original Clear2 path|Historical strip-left0 correction and current captain right-edge fix prove extent problems. With a new reserve derive top/left/right/bottom from shared reserve/repaint policy, and keep original clear call identity/flags/result. Do not leave the oldH-154 top after enlarging HUD, clear through the radar overlap blindly, or add another unrelated fill/Present.|
|4070C0 /43857F|World ray normalization and cursor caller use scene dimensions, halves and projection. Update with sceneRect/projection; raw physical mouse globals and native recenter cache remain physical.|

The old rootX80, scene widgetX-80 at439F04, and absolute-vs-local corrections40190B/40195D/4019A9/479CA5/479CE4/459F33 are provenance for double-translation bugs, not native-layout constants to transplant.

## Native minimap/preview

Map their ORIGINAL logical destination edges through the shared HUD transform, then use actual native viewport/scissor dimensions so their3D content is rendered at the physical size. Keep radar/preview projection and custom point mapping paired with those viewports. Do not scale a screenshot of their prior native output or apply the captain sprite transform a second time to their internal3D geometry.

Retain native target/depth ownership and original ordering. Radar background, contacts, scan controls and preview border can use owner-scoped sprite/font transforms; the native map/preview pass consumes its own mapped viewport. Restore prior scene/fullscreen viewport and projection after those passes, including failure/paused paths. Their custom hit/picking routes need the inverse for the corresponding mapped rectangle, not world-scene inverse or generic menu hit cells.

## Projection/picking contract

Reserve change is a scene aspect change, not merely UI positioning. For first policy keep scene origin0,0 and native widthW; use the new reserveTop for scene height. Update viewport, scene globals, half dimensions and cached consumers consistently before processing vertices. The recorded engine can preproject vertices before draw submission, so changing draw-time viewport/matrix alone is not proof of correctness.

Declare FOV policy explicitly. Suggested bounded control: preserve vertical FOV and derive horizontal scale from actual old/new scene aspect while retaining depth terms: `newP00=oldP00*(newSceneH/newSceneW)/(oldSceneH/oldSceneW)`, `newP11=oldP11`. Bind old values to a captured canonical projection, not repeated multiplication each frame. This is a policy relation, not a claim the original2.0/3.5 constants exactly encode the nominal viewport aspect. Rendering/Engine must update the actual projection consumed by processing and the same projection used by4070C0. Do not modify near/far/depth entries or camera basis to fix UI placement.

World labels remain world-projected/native; objective banner494FC0 is a separate screen-UI owner. Scene camera zoom/rotation and world selection must remain aligned after reserve changes. If later sceneRect has nonzero origin, compensate ray normalization at its scoped consumer as well; do not infer support from the initial0,0 policy.

## Bounded next implementation/test stage

1. Add the shared layout snapshot/provider without moving more groups. Captain adapter consumes supplied common transform instead of free-strip fit. Assert one generation across render/input.
2. In one joint candidate add radar/scanning, preview, status/orders group anchors to the same assembly, and apply reserve/scene/projection/picking outputs together. Preserve native scene, pause modal and original clear semantics; do not intermediate-ship a taller HUD over an unchanged154-reserve scene as final layout.
3. Verify pure math/reference edges, group-anchor source subtraction, radar16px overlap, viewport roles and generation invalidation. Use the existing native scene/processing audit to compare actual projection/picker. No new parallel patch implementation is requested from Resolution.
4. Live1440 test: whole HUD visible, same scale across groups, captain selection/paging, radar/scan and preview alignment, status/orders hits, ship selection at scene center/edges, radar-overlap blocking, zoom/rotation, paused Escape/resume and native cursor cleanup. Include faction/resource differences later; initial reference values are not faction certification.

Source density remains separate: a portrait or glyph replacement's texel dimensions must not alter logical hitbox, common scale or physical reserve. This interface carries layout only; it does not expand the approved texture-replacement identities.
