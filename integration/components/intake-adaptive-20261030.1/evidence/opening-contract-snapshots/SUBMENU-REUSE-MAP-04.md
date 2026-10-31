# Main-menu destinations: existing Resolution reuse map

2026-10-25. Read-only source/reference handoff. No implementation, game launch, networking restoration or Current modification.

## Modal heads under unchanged state7

Preferences4DB6F8 and Load4DF100 are not new full-screen states simply because their controls are visible. Preserve actual global modal-head order, underlying Main, disabled/consumed/capture behavior and original callbacks. Do not treat the old pure Main root whitelist as an already working modal composite.

### Preferences: exact centering owner

Original436150 computes centered rootX/Y against native q_screen_width/height before attaching4DB6F8. Its packed X/Y are4DB700/4DB702; packed width/height are4DB704/706. The historical UI builder only replaced width load43615D with800; it did NOT normalize native-height centering and is not a complete native1440 popup fix.

Small scoped normalization: after the original constructor has set up this exact modal, but before its first logical painting/input, set root local origin to((800-root.w)/2,(600-root.h)/2) using original signed-division semantics and original dimensions. Keep local child geometry unchanged. This is a persistent modal-context origin, not render-only temporary coordinates. Recheck/restore context on detach/reinitialization; do not change native display globals or the global5F7ED8 root. Root dimensions exceeding the declared canvas need an explicit alternate policy, not silent clipping.

Source inspected: Engine exports/main-menu-contract-03/targets/00436150/decompiled.c. Constructor creates slider controls through4217C0, at logical(32,50/100/150/200/250), optional gamma atY300. It installs drawing436020 and change callback436520. Difficulty/speed/music/SFX/voice/gamma are therefore real drag/slider workflows, not merely button rectangle clicks. Rectangle inverse admits a hit but does not prove drag updates consume logical coordinates. Ask Engine for the slider's actual coordinate read at4217C0's installed input/drag path; apply the same private owner inverse there if it bypasses the rectangle locals. Do not rewrite global physical mouse values or disable sliders to claim compatibility.

### Load: same modal policy, distinct constructor

Use supplied head4DF100 and Engine's exact constructor attachment map, then normalize its root localX/Y from its own dimensions under the same800x600 context. Resolution does not have a verified Load-specific centering instruction pair to claim. Existing xref brief-art-family-xrefs-v1.json contains this root, including44F5E0; that is a reference lead, not a proven centering hook. Prefer exact post-constructor root normalization to another sweep of literal offsets. Lists, scrollbar, text entry, selection/double-click and modal cancel must retain original event dispatch; generic rectangle fit alone does not establish those custom coordinate consumers.

## Reusable generic centering helper and historical sites

Original434390(paramWidget,paramChild) writes widget+8/+A using(nativeW-widget.w)/2 and(nativeH-widget.h)/2, attaches widget via41D590 and optional child via41D470, then sets widget+8C from49DFB0. +8C is palette/transition state, not geometry.

The old12-site width-only allowlist is:
4343A0,434445,4344D5,435319,435388,4359DF,435B4A,435BB6,435C8B,435CF7,43615D,4367AE.

Source: integrated resolution-history/build_ui_center_probe.py. Those sites read5FF670 intoEAX and historically substituted800. They are useful centering leads, NOT identities for every submenu and NOT a blanket native policy. Native-height reads require corresponding owner review or post-construction root normalization; never globally spoof5FF670/674.

Shared confirmation builder423760 reads surface width intoDI at4237F9, halves it at4237FD and writes widgetX4CC670. Historical build_dialog_center_v1.py only replaced that width with800; Y was unchanged. Use a separately verified logical-modal context for confirmations, not an unconditional global patch to a working native mission/Escape dialog.

## Rendering Main plus modal once

Reusing the completed-target ROI route is reasonable: retain/restore an UNscaled Main history or freshly repaint original Main at logical0,0; paint the modal at its normalized local origin in original order; snapshot one complete800x600 region BEFORE native tooltip/cursor; fit exactly once. Do not snapshot a prior fitted Main as the modal background or fit Main and then refit it with the modal. The passing Main shadow controller rejects altered modal topology by design; coordinate ownership explicitly with it so only one compositor owns that frame.

A transparent/modal partial repaint needs correct underlying logical pixels each frame. Restoring logical Main underneath is not permission to replay stateful activation callbacks. Preserve original sibling ordering/disabled blocking; ancestry-private input belongs to the exact admitted modal tree and underlying Main must remain blocked as original. On close require a fresh coherent Main publication, not reuse the modal generation.

## Full-screen destinations: Credits15, Multiplayer20+

These are separate state/owner contexts, not Preferences/Load modal descendants merely because Main launched them. Resolution has no additional validated Credits/MP-specific constructor/centering/shape patches to transplant. Use Engine's supplied state/root/callback map with the existing800x600 completed-target strategy, exact fit and scoped input. If their actual roots already sit at0,0 with800x600 content, no centering patch is justified. Normalize only identified nativeW/H placement consumers if actually present; do not apply all12 historical popup edits.

Separate view/layout support from network services. No GameSpy/backend restoration or connection attempts are implied. Scope Multiplayer initially to opening/closing its local UI and original error handling. Credits scrolling may consume nativeH or draw through a custom callback rather than widgets; retain original logical bounds and scroll rate where established, do not infer them from state15 alone.

## Known shape and input boundary

Original Main's six controls were confirmed rectangles, despite visual artwork. No Resolution evidence identifies a particular Preferences/Load/Credits/MP control as a pixel-mask or circle predicate. Thus do not invent a shape exception list. Retain existing unsupported flags2/400000/custom dispatch policy until actual consumers are identified. Sliders, scrollbars, list selection, drag/drop or text fields can still read coordinates outside41DB02 even when their initial hitbox is rectangular. This is a targeted custom-input check, not a need to reconstruct every ordinary widget.

Reuse41DADC for eligible hit/disabled blocking and41DC62 for scoped miss/cleanup; physical globals stay physical. Use checked ancestry+parent accumulation, sticky uncertainty invalidation, prewrite scaled provenance and fresh frame repair from the passing opening controller. Do not relax those safety properties for convenience.

## Small coordinated implementation sequence

1. Preferences and Load: exact modal attachment registry, constructor-time logical root normalization on BOTH axes, single Main+modal logical composite, original modal blocking and known custom-input adapters only as needed.
2. Credits and local Multiplayer destinations: Engine's exact root/state registry and same fit; no unrelated modal or mission changes.
3. One candidate, one user flow: open/cancel each; move a slider and restore value, scroll/select an existing save without overwriting it, return from Credits, enter/exit local Multiplayer UI without networking, and confirm Main still works. Record actual unsupported/custom coordinate cases rather than claiming every destination complete from an opening screenshot.

Recording reliability fixes remain independent. No new global offsets, no fixed80, no old executable/cave transplant, no HUD changes and no broad test campaign required by this map.

## Confirmed slider refinement; source lookup sufficient

Original-hash-gated disassembly confirms4217C0 installs input420FB0 (store421874, mask0401 at42187B) on the composite slider and its generated children.420FB4 loads EBX from signed mouseX5F7E5C;420FBC loads EBP from signed mouseY5F7E5E.420FC5 loads EDI=callback widget;420FC9..421003 accumulates logical parent positions. Candidate private-register adapter boundary421005 has six original bytes `8b4704f6c420` (MOV EAX,[EDI+4]; TEST AH,20). Under checked Preferences ancestry/generation only, map private EBX/EBP to logical coordinates, leaving ancestor locals and physical globals unchanged. Engine verifies bridge/ABI before implementation.4210F4/42112D compare EBX to logical track positions;421379 onward derives knob X from it.420F90 release helper reads no mouse coordinates and must retain original cleanup.

Captured drags outside content require an explicit private extended/affine coordinate or endpoint policy; inverse_pixel's margin rejection is not a continuous drag inverse. Preserve release outside and original min/max rather than clamping ordinary hit tests onto controls. Detailed evidence: `E:\Codex\research\dominion-resolution\PREFERENCES-SLIDER-INPUT-05.md`.

No additional Load custom-input function or coordinate-read site has been confirmed by Resolution. Existing root/reference and list/scroll/text-entry checks above are leads, not invented hook addresses. This source lookup is sufficient for the requested bounded implementation; no additional review campaign or game launch is requested here.
