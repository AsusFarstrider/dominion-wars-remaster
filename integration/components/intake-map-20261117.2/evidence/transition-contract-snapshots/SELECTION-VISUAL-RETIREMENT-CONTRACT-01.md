# Selection visual retirement: original phase boundary and bounded Back contract

2026-10-24. Source assessment and observation contract only. No implementation, game launch, original/candidate executable change, frozen-v5 change or canonical Ghidra save. All VAs refer to original NA1.04 SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`.

**Recommendation: investigate Back10->7 first, using the original destination menu's existing full-target clear.** Do not begin by adding a global clear. The likely smallest correction is to finish one last valid scaled selection frame after Back queues state7, then let the original destination painter establish native visual ownership. First collect unsampled frame/state/target/clear receipts: the existing Back observation does not establish that complete runtime join.

## 1. Exact original frame order and useful boundaries

Frame routine429330 executes in this order, subject to its original activity gates:

| Site | Operation | Consequence |
|---|---|---|
|429330|Load input713834; save EBX/ESI|Frame-entry metadata; before input callbacks|
|42934F|CALL input phase|Input can queue pending state or attach UI|
|**429351**|**MOV EAX,[713840]**, bytes `a140387100`|After input, before the scene phase is loaded/dispatched|
|42936C|CALL scene phase|Potential3D destination painting|
|429389|CALL secondary713838|Secondary update; do not assume every state is paint-free|
|4293AB|CALL UI/draw71383C|Current state's UI and cursor|
|4293AD|Next instruction after draw returns|Post-draw, before presentation|
|4293C3|CALL4A79B0|Original presentation; target storage may rotate|
|4294C6 /4294CC|Cleanup429FB0 /install429980|State transition occurs **after** that presentation|
|4294D6|Normal frame RET|End owner frame|

429351 is the primary post-input/pre-scene boundary for observation and a later guarded retirement action. Its5-byte window is identical in original, native1440 and approved executables. Branches42933B and42934D can also reach it without an input call. It must not be labeled an unconditional input completion. At that site originalESP=entryESP-8; the frame's transition-enabled byte is at[siteESP+C]. Preserve original register/flags/FPU/LastError/stack state if instrumented. The displaced MOV must still load the scene pointer after observation.

Match frame entry/thread/stack and nesting explicitly. Original callers include modal loops422990/422EB0 as well as the main loop; they are not interchangeable with a top-level transition-enabled frame. Apply the original active-frame condition `(A396D8==0) || (screenFlags&0x40)` before considering writes. A paused/inactive visit is not permission to clear.

**Do not put a general clear at421F70.** In a scene-bearing state it follows the scene callback and would erase newly rendered3D. Installer429980 alone is also too late to repair the departure frame, because the old state has already drawn and presented. The phase-order proof does not prove that arbitrary input callbacks, constructors or reentrant modal loops never paint; that stronger claim requires a bounded producer contract.

## 2. Back has a specific pending-state frame

Original4729F0 case446 at4E53B0 handles the Back control. Pointer event0 queues pending4D0A28=7; keyboard P/Escape is another original route, but first admit the captured pointer case only. Current4D0A24 remains10 until the end-of-frame install.

Therefore, after Back activation,429330 still draws selection42A290 and presents it with current10/pending7. V5's render admission requires pending10, so it does not compose that departure frame. Its original800x600 menu writes can overwrite only part of the previous scaled image. This is distinct from the now-closed hitbox and input-epoch issues.

Cleanup429FB0 case10 calls472600, which unloads resource family800 and detaches selection. Installation429980 case7 calls46AED0, attaches root4E47C0/id405 and its child table, then installs42A210/42A230/42A290 with scene713840NULL. Root4E47C0 has flags84, not4, origin0,0, original size800x600 and painter46ADD0. Under the captured selection setup4D09F8=1, constructor46AED0 skips its optional2/3 branch; it has no explicit clear/present pair.

## 3. The original destination painter already supplies the native clear

Original46ADD0 obtains its background from root+50 (resource201). It compares background.width with the current wrapper's width at+10. **If the background is narrower, it calls4A1710(currentWrapper,0) before drawing the background or text.** Original resource201 is800x600; a native2560-wide wrapper satisfies the condition.

4A1710 resolves the actual native image and constructs a full-native-dimension, origin-matched solid-fill source: flags0,8bits,and_mask0,xor0, callXY0,0, then49F050. Source clipping flag1 is absent, so this path does not inherit a smaller global clip as its only bound. Its dimensions come from the destination native image, not the background's800x600 dimensions. It then continues original background and version-label drawing. No blanket frame clear needs to be invented to reproduce this state's existing policy.

This is static proof of the path and its dimensions, not a successful runtime full-write receipt. Confirm actual wrapper/native descriptor, format, Lock/Unlock results, full fill span and presentation. A4A1710 entry alone does not prove successful completion. Do not interpret49F050's return value as a uniform HRESULT.

The existing capture records Back event0 atseq3088442 and later root4E47C0/id405/draw46ADD0, callback serial50234 starting3088460. Rendering's bounded extraction retains only two font producer samples and no new state snapshot after the trigger. Repeat sampling can omit an already-seen clear/background. **Do not infer either a missing clear or a completed state/target join from that absence.**

## 4. Smallest plausible next correction, after observation

Keep strict input admission unchanged. Consider a separate **render-only departure lease** for exactly one armed Back frame: previously coherent selection, captured pointer Back event0 in this owner frame, current10/pending7, sceneNULL and the expected phase pointers, unchanged selection root/tree/resource generation/target, no foreign producer, reentry or coverage gap. Finish the normal selection subtree and compose its complete logical image, then allow the original cursor and original presentation. Resources remain present until cleanup after presentation.

Do not relax a shared screen/family predicate so that pending7 also enables input. Remaining selection predicates must retain miss/cleanup and original release behavior. A departure visual commit is not renewed interactive selection ownership. Do not skip original input, draw, cleanup, install or presentation calls; do not add a Flip.

For the first state7 native frame, observe46ADD0's existing full-target fill before background/text and its original presentation. Native nodes keep physical coordinates. On returning to selection later, require a fresh successful scaled composition before transformed selection input resumes. There is no need to reset v5's conservative may-contain-scaled flag merely to let unrelated native controls work.

If a final departure composition fails, retain quarantine and report that frame as unrepaired; do not claim native visual retirement. A pre-paint full-clear alternative at429351 would need its own concrete review, target generation checks, successful clear and original repaint/presentation proof. The source identifies a legal phase position, not approval for unconditional writes there.

## 5. Target lifetime, storage and reset constraints

- Reacquire the current holderA39B64, outer wrapper, native+20 descriptor and COM surface at every relevant boundary. Join the UI surface with screenA38770+44 (A387B4) and actual operation resource generation. Keep logical canvas, draw target, front/present destination and returned Lock pointer distinct.
- AddRef protects an interface lifetime, not its backing storage or device generation. Original presentation4A79B0 can Flip, BltFast or Blt. A stable interface address across5896 selection commits does not prove one immutable pixel allocation or retire every flip member. Do not directly clear the primary/front buffer, infer a two-buffer cycle from timing, or treat one clear as global native ownership.
- In this Back case, original46ADD0's full-target fill can protect each destination frame as it is painted, including rotated storage. Require per-frame proof before treating a presented native frame as repaired; do not clear a global dirty ledger merely because one member was observed. Unknown chain members remain unknown until overwritten or genuinely destroyed.
- Wrapper destruction4A10A0 frees descriptors/holder and setsA39B64NULL; creation4A0E50/429030 makes new records. Mission initializer42B040 destroys/recreates wrappers. Discard borrowed descriptors/pixels and invalidate all leases across these boundaries.
- Screen release4A6790 releases devices, draw/depth surfaces and aliases. Focus restore4A6720/4A8C70 can Restore existing interface objects; mode changer4A7B50 can recreate them. Same pointers after Restore or reuse are not fresh proof. Revalidate dimensions, format, lost state, aliases, generation and producer coverage. Do not operate on a merely retained old COM pointer after a screen-generation change.

## 6. Next unsampled observation contract

The next instrumentation should record once per relevant owner frame/generation, without repeat suppression:

1.429330 entry and429351: frame ID/thread/ESP/depth/transition flag/activity, current/pending/actual previous state, four phase pointers, global and relevant subtree identity, target tuple, source/resource/device epochs and any prior producer in that frame.
2. Back pointer callback and subsequent pending state; do not treat callback eventE on every control as an activated shortcut. At4293AD, record which phase calls actually ran and the completed selection/native draw owner.
3.4294C6 before cleanup,429980 entry/commit, and the first matching state7 pre-paint frame. Constructors can draw/present outside the normal phase pipeline; log them, rather than assume callbacks describe all painting.
4. First46ADD0 native background width vs wrapper width,4A1710 entry, generated solid source/destination at49F050, its actual surface Lock/Unlock and full-span write evidence, then original presentation result/target generation. Existing sampled metadata is insufficient for this join.

Reject continuity on gaps, nested/unexpected painting, changed epoch, lost/failed surface or changed topology. Preserve physical globals. A complete observed clear/repaint/present for the same native owner is the useful success criterion; pointer equality or callback ancestry alone is not.

Federation441 requests state9 (video), not direct mission3D. Its constructor472420 already performs clear->present->clear before movie setup; those constructor operations need separate result/storage receipts. Actual mission initialization42B040 also calls progress/setup functions while installing a state, before the next normal scene callback. Do not extrapolate this Back contract into a mission-entry guarantee. Native overlays likewise need a separate topology/producer contract; pure-root publication remains unchanged.

The result is a concrete next investigation, not a general retirement implementation: observe the missing Back join, review the render-only final-frame lease, and reuse the original destination clear where its operation is proven.
