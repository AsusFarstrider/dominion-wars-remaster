# Captain trial: Escape graphics and right-strip cleanup

2026-10-25. Bounded source follow-up to Rendering's first captain trial. The user reported successful enlargement/selection/camera/focus/quit, with right-strip cursor trails and a small row while Escape was open. No new exports, agents, implementation, build, game launch or Current edit. Corrections below are source-supported proposals, not a live pass.

## Escape redraws the same HUD through another phase

Builder4352E0 creates native Escape root4DA7C8/id49D, flags80, painter4342D0, inputNULL. It attaches400x462 or400x318 at native-centered coordinates, adds child table4DA858/4DAD68, sets active7180B6 and requests state2 for the single-player path. Child action callback4355D0 owns Resume/quit/Load/Save/Preferences/confirmation actions. Resume removes the root with41D770, clears7180B6 and requests the saved return state4E3134 (mission3 in this case). Other actions can replace Escape with another modal;7180B6 alone is not a complete modal-input guard.

Existing state installer429980 confirms:

| Context | Input | Secondary | Draw | Scene |
|---|---|---|---|---|
|Mission3|42AB90|42A2D0|42A7B0|4022A0|
|Pause/Escape2|42AB90|42AA20|**42AB00**|4022A0 **when saved4E3134=3**|

42AB00 begins UI graphics, draws any mode-dependent pause text, then calls421F70 at**42AB7C**, native cursor43B140 at42AB81 and ends UI graphics. It does not call42A7B0, but the same attached captain subtree still traverses421F70. Rendering reports575 normal-phase and337 paused-phase prepaints, with575 commits and no adapter errors. This matches the trial's `root()` requirement71383C==42A7B0 rejecting the paused redraw.

**Small correction:** separate graphics eligibility from input eligibility. Admit this verified mission-pause render context, unchanged captain root/visibility/target, and existing subtree scope; preserve all original rendering order under Escape. Keep input blocked by the actual Escape/Preferences/confirmation tree and original flag80 priority. Do not broadly permit all paused states, all42AA00 drawing or all scene-less menus. The input callback pointer42AB90 is the same in both states and cannot distinguish interactive mission from modal ownership. Preserve visual continuity through known Resume2→3 while keeping publication/epoch checks; a pending state change is not permission for another captain activation.

## The original strip rectangle excludes the right margin

The original4022A0 instructions are unambiguous:

-4023A9 loads `L=[5FF6D0]` intoEAX.
-4023AE/B4 loads `W=[5FF670]` intoEDX and subtractsEAX.
-The stackD3DRECT is **[L,H−stripHeight,W−L,H]**, with stripHeight from5FF694 (original154). These are rectangle edges, not left/width.
-Clear2 at4023EF, caller return**4023F2**, uses count1,target flag1,black0,Z1,stencil0 on the viewport objectA38C88. The full-screen viewport record4F022C has been installed before this operation; scene/panel/preview and UI paint follow.

Thus L>0 leaves a rightmost margin of widthL uncleared by this operation. Rendering's screenshot and receipts locate trails there. This supports the proposed **private rectangle copy with only right=W**, scoped to exact caller4023F2, actual viewport/target, fresh native dimensions and matching original rectangle/arguments. Keep leftL,top/bottom,other arguments,HRESULT and the single original call. Do not add a Present or a full-frame fill. Preserve original scene reserve/projection/picking.

Resolution's historical4023A9 MOV-EAX-zero change made the existing arithmetic clear the full-width strip[0,H−154,W,H]; its user explicitly passed cursor/old-art cleanup. The proposed right-edge-only change is narrower and addresses the demonstrated right margin while retaining the left/minimap boundary. Historical success supports this repair location, not a new runtime claim for the narrower correction. Use actualL/W/H, not old960/243 literals. This refines any earlier shorthand describing the original clear as simply “from map-right to W.”

## Cursor boundary and what is not proved

43B140 draws the native cursor after HUD/tooltip through40F990/40F650 or software49F050, using mouse5F7E5C/5E minus hotspot5F7E60/62. It indexes cursor bookkeeping by the current wrapper's WORD+8 active-image index: rectangle words at718628+16*i and valid flag71869C+4*i. Preserve physical coordinates and cursor scope; do not include it in captain scaling.

This is cursor/dirty-rectangle bookkeeping, not evidence of a universal underlay-save/restore routine covering untouched pixels. No new cursor-buffer rewrite is justified while the known strip clear demonstrably excludes the affected region. Retain fresh storage/target identity rather than treating one COM pointer as every backing image. If trails persist after the scoped correction, inspect that actual failed path; do not reconstruct all cursor machinery preemptively.

Evidence: existing Escape contract,429980/43B140/4022A0 exports, Resolution `STRIP-CLEAR-v1.md`, frozen strip builder, trialHudCaptain.cpp and small original42AA00/42AB00/4355D0 windows. `evidence/hud-escape-strip-boundary-01/original-windows.json` and its manifest pin these. Rendering's counts and visual report are relayed here; this chat did not reimport the run or validate the proposed fix in-game.
