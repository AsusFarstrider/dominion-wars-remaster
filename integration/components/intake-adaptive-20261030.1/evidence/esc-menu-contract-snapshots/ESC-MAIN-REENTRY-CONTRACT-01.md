# Mission Escape → Main: bounded re-entry contract

2026-10-29. The user has now explicitly authorized fixing the accepted baseline defect. Rendering owns a new disposable source/development child from int-20261029.1/source-v2; Current remains immutable. **The retained original code enters the same Main constructor and phase callbacks as fresh startup. The leading hypothesis is adapter admission/provenance failure, not a separate legacy Main implementation.** Engine changed no game/runtime code and performed no build, launch or new Ghidra export.

## Confirmed original lifecycle

Mission3 uses input42AB90/secondary42A2D0/draw42A7B0/scene4022A0. Escape builder4352E0 attaches4DA7C8/id49D, sets7180B6 and normally selects pause2, whose draw42AB00 retains scene4022A0 when saved4E3134=3. The same HUD tree remains attached under the pause overlay.

Escape action4355D0 ids4A0/4A7 removes4DA7C8, clears7180B6, and in the ordinary offline path opens confirmation423760 with callback435510. The retained callback reference and bounded original/native byte window establish: confirmation result5F7F8C==2 plus game mode4D09F8 in1/4 invokes45C710(59F) then sets pending4D0A28=7. The alternate game-mode/network branches select13/14 or a multiplayer return; do not generalize them into this single-player repair. Cancellation preserves the original saved-state behavior; do not replace the confirmation callback or replay its side effects.

429330 finishes original draw/present/limiter work before cleanup429FB0 and installer429980. For current2/pending7, cleanup calls494E30 and mission teardown42B4A0 when713898 is nonzero. The same cleanup dispatcher removes prior Main through46AF70 on departure, releasing group200 and its tree. Teardown42B4A0's full transitive state effects are not established by this bounded review.

Installer429980 clears the scene callback, selects case7, invokes46AED0, then assigns42A210/42A230/42A290 and commits current/pending7 only after construction.46AED0 loads group200, calls41D330 with actual wrapper W/H, attaches Main4E47C0 plus six original children, assigns palette200 and resets animation clocks. Its mode2/3 networking cleanup remains an original side effect.41D330 resets the global0x90-byte root and input-dispatch setup; it does **not** reset the two pre/post pointers immediately before that root. Static child dimensions are zero but41D470 initializes180x100 from the original base images; re-entry must validate initialized nodes, not recreate their geometry from static zeros or restored source density.

Do not infer an unpatched800x600 mode reset from original4292B0's decompile. The approved DE3 native executable already changes4292D9/4292DE's height/width immediates to1440/2560. Constructor/root/callback behavior above remains original.

## Confirmed adapter behavior and targeted hypothesis

CanvasRuntime::screen requires current/pending7, exact menu phases, no scene, correct root/global topology, mode!=15, **pre5F7ED4=0/post5F7ED0=0**, plus six original node geometry/bank/counter/link checks. Main root.previous+6F must also be0. OpeningFlow's Main context checks phases/topology but does not impose the same pre/post-zero guard. On ordinary pure Main its outer pass is only a monitor: completion returns unless a proved modal activation requires that outer owner. It does not automatically rescue an inner Canvas pass that never started.

Original438E40 writes pre/post control hooks: case0 can install47DDC0; cases1..4 install47DCE0/47DE90 after their mode-specific setup. Entry cleanup depends on prior718690.41D330/46AED0 do not establish that those hooks are zero after a mission. **Unproved:** which values survive the actual teardown/re-entry and which exact guard first fails. Do not erase original hooks or admit arbitrary callbacks based on this hypothesis alone.

PublicationState::invalidate preserves mayBeScaled=true while revoking coherence. If Main re-entry never produces a fresh fitted commit, its original800x600 painter remains at the upper left and known menu predicates route to miss. This is an intentional stale-transform quarantine and explains the reported paired symptoms if admission fails. Resetting provenance alone would permit native hitboxes against unresolved output; it would not repair rendering or ownership.

## Smallest next check and repair boundary

Rendering should capture one bounded read-only first-reentry admission snapshot: first failed screen/family guard, current/pending/phase/pre/post values, Main/global links/geometry, native/outer target dimensions, and publication mayBeScaled/coherence/epoch/generation plus capture/blocking state. Existing source supplies the map; a full recorder or broader cleanup reconstruction is unnecessary before this check.

Repair the demonstrated boundary only. Preserve original constructor/confirmation/cleanup callbacks once; retire mission/HUD and old Main publications, then require a fresh owned Main repaint/commit before transformed input resumes. Keep Main/modal and causal departure rules, original bank timing/hit rectangles and native-target identity. A plausible known-callback admission change must be justified by the snapshot and callback semantics, not a global relaxation or pointer clear. Rendering retains implementation and one focused reproduction/retry; Engine will review its concrete hypothesis/delta.

Exact retained bodies, approved-mode instruction differences, bounded confirmation callback window and source pins: `evidence/esc-main-reentry-contract-01/audit.json`. The callback window is labelled as a window, not a newly reconstructed function. The old baseline acceptance/deferral remains historical; this new request authorizes work in the child, not mutation of the approved artifact.
