# Captain group: bounded draw/input/composition map

2026-10-25. Rendering requested only the first captain-group map for an approved isolated trial. Original executable SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`. No new exports, reconstruction campaign, build, game launch or Current edit.

**Choice:** scoped sprite/primitive transformation is better supported than a completed492x51 native snapshot for the **complete** group. That rectangle bounds the main row, but each slot owns initially hidden children extending90 pixels upward into the scene. A larger snapshot includes world pixels. The checked painters predominantly use40F630/40F650, allowing reuse of an owner-scoped image path without inventing a whole-HUD subsystem. The generic frame still has a possible CPU representation; handle or explicitly reject an unsupported producer before publishing a partial transformed group.

## Exact root and six slots

Root4E5FB0: id46, flags4, static492x51, drawNULL, inputNULL, mask0, no text.479C60 assigns its runtime origin from layout globals and attaches it to global5F7ED8. It attaches static slot block4E7BD0..4E7EA0 and then calls47ED70 for each slot. Original data:

| Slot address | ID | Local X/Y | Common callbacks |
|---|---|---|---|
|4E7BD0|59|0,0|draw47B610 / input47B1A0|
|4E7C60|5A|77,0|same|
|4E7CF0|5B|154,0|same|
|4E7D80|5C|231,0|same|
|4E7E10|5D|308,0|same|
|4E7EA0|5E|385,0|same|

Each has base flags1, eventmask000C, resourceB708, static W/H0 resolved on attachment, slot ordinal0..5 at+50 and static textNULL. Validate live geometry instead of using the128x64 padded texture allocation as a hitbox. State/faction resource selection remains original; B708..B70B identification does not prove all C8 states.

## Complete paint path

47B610's original sequence is:

1. Set widget padding+62=3,+64=2; call421D70 at47B630. This reaches the ordinary41E010 state-image path and40F900 dispatcher. It is not merely a direct B708 blit; actual representation can be TXTR or software.
2. Resolve displayed object using slot ID and page offsetA35AC8. If absent, set disabled10000 and return47B8D4; preserve that original side effect. Otherwise:
3. Call47B290 at47B676, anchored slot+41,+7. This emits status-selected images through40F630; resource bank selection includes the71386C faction adjustment.
4. Portrait at slot+3,+22: nonnegative portrait selector uses resourceA17B+index at47B6DB; the negative-selector path uses5F85A4 pointer table at47B70B. Both use40F630, not the named captain-frame provider.
5. Optional animated/colored overlays anchored slot+37,+4, then24 pixels lower, use BB50/BB59 families and40F650 at47B7B6/47B872. The fourth argument comes from4E80E4's color table; retain it rather than treating this as the three-argument white40F630 wrapper.
6. Restore palette via4A2CF0 as needed, then47B4B0 at47B8BA, anchored slot+23,+22, emits further state icons through40F630. Normal return47B8C7.

These exact slot callbacks have no direct font4A2650/41E7F0 call. The auxiliary painter below also uses images/colored quads rather than a direct glyph run. This does not classify neighboring HUD text or authorize a global GPU transform. Scope every nested primitive by owner/ancestry and preserve UVs, masks, color/blend, source lifetime and draw order. CPU target holderA39B64 alone does not redirect the device used by40F650.

## Children outside the row: concrete ROI hazard

47ED70 creates **nine25x30 controls per slot** with41F7C0: IDs6A..72, painter47F220, input47ED30, mask0019. Initially hidden8; positions are columns0/25/50 and rows−30/−60/−90 relative to that slot. Widget+50 holds an allocated12-byte data record; +54 forms the parent's auxiliary chain. These are object-related auxiliary controls; a full gameplay role/name is not claimed here.

When visible, they extend the group above its static492x51 rectangle.47F220 uses family-adjusted image resources and40F630/40F650, including hover-dependent color, with original returns47F6B6/47F6C0/47F6D0. Its parent and page-offset logic chooses the associated object. IDs repeat under six parents, so do not identify/deduplicate them by ID alone. Hidden initial state is not proof that they will remain absent in a mission.

A snapshot enlarged to include them copies scene pixels between icons. A before/after pixel difference is not reliable alpha coverage: a drawn pixel can equal the underlay. Prefer scoped sprite transformation or explicit primitive coverage. An inactive-child-only trial must say it is not yet complete-group coverage.

## Input and ordering

47B1A0 and47ED30 are action handlers, with no direct reads of native mouse words5F7E5C/5F7E5E in their bounded bodies. Slots resolve object from page offset; event3 calls419A00 and audio, the other slot path calls438C30. The same47B1A0 also recognizes paging IDs5F/60 and tails to47A900; those controls are not part of the six-slot table, so any paging inclusion needs live ancestry rather than inferred address membership.47ED30 uses its data-record object for selection/focus. Preserve masks, event3 behavior, disabled blocking, original callbacks and release/cleanup; no new drag-coordinate bridge is identified by these bodies. This is not a proof about every downstream helper or other HUD owner.

Constructor479C60 attaches roots in order4E5E90,4E5F20,4E5FB0,4E6040,4E60D0,4E6160.421E80 paints next siblings first, so absent other dynamic heads the effective order is the same: earlier preview/helm roots, captain group, then6040/60D0/6160. Slot paint order is59..5E, including each slot's visible descendants. Later roots, modal heads, tooltip and cursor must retain their original priority.

The best **candidate clean underlay point** is common421ECB with ESI=4E5FB0 and EBP/EDI=parentXY: next siblings have finished, and this root/descendants have not painted. Even with root drawNULL,421ECB runs before the generic421D70 branch at421ED9. A capture here still needs fresh synchronized native storage; it is not evidence of an isolated surface. Match subtree completion at421F60 using entryESP−12, not currentESI (tail traversal can leave a descendant in it). Finish a captain layer there if it must preserve occlusion by later roots. Deferring it to the end of the entire forest would put captain pixels over those later controls.

4023A9's bottom-strip clear precedes native scene/panel/preview rendering and earlier HUD producers; it is not the final clean underlay. Original captain row is below sceneH, but auxiliary icons can overlap the native scene. Do not erase the enlarged bounding box, scale the world behind it or apply a full-target black clear. Scoped primitive placement avoids drawing the original tiny group as well as the scaled one; copied-pixel approaches need separately proven original-footprint retirement/underlay. Whole-frame input/picking and native viewports stay unchanged for the first trial.

Evidence: `scripts/map_captain_group_boundary.py`, `evidence/captain-group-boundary-01/original-windows.json`, existing479C60/421E80/4022A0/42A7B0 exports and the previously recovered image dispatcher contract. Root/slot data and the seven small callback windows are original-byte facts; live bounds, actual auxiliary visibility and complete transformed interaction remain the one mission trial's questions. No further source review is requested.
