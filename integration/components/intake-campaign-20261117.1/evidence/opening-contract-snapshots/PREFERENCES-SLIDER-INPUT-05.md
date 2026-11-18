# Preferences slider: concrete coordinate consumer

2026-10-25. Read-only original binary inspection. No engine annotation, code patch, build, game launch or Current change.

Original executable inspected: `E:\Codex\research\dominion-resolution\game-test\ds9dw.exe`, SHA256 checked before disassembly: `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`. Used pefile and Capstone to decode original4217C0 and420FB0; this is raw source-bound instruction evidence, not live input proof.

## Confirmed consumer

The Preferences constructor436150 calls4217C0 for sliders. That factory assigns widget input callback420FB0 at421874 (`c74648b00f4200`), mask0401 at42187B and composite-slider flags2000. Its child controls also use420FB0, with parent links and sub-IDs. Thus a descendant-aware hook can cover the common handler without patching every generated slider.

420FB0 reads global physical coordinates directly:

-420FB4 `0fbf1d5c7e5f00`: EBX=sign-extended mouseX5F7E5C.
-420FBC `0fbf2d5e7e5f00`: EBP=sign-extended mouseY5F7E5E.
-420FC5 loads EDI=callback node from stack.
-420FC9..421003 accumulates original parent X/Y through+77 into stack locals+10/+1C.
-421005 begins dispatch with `8b4704f6c420` (MOV EAX,[EDI+4]; TEST AH,20).

Later paths compare EBX with logical track/child/ancestor positions (e.g.4210F4/42112D), and derive dragged knob X from EBX (421379 onward). The vertical half likewise uses EBP. Generic rectangle inverse at41DB02 cannot change these register values.

## Small adapter boundary

Candidate original instruction boundary421005, after the direct mouse reads and ancestor accumulation, exposes EDI=node and EBX/EBP=physical mouse coordinates. Under a verified active logical Preferences context, checked ancestry under4DB6F8, matching input generation/target/epoch and native input owner, convert only those private registers to logical X/Y. Original ancestor sums are already logical; do not inverse them or add the modal origin twice. Replay original displaced instructions and leave flags/stack/FPU/LastError and all other registers according to the Engine bridge contract.

This is a proposed hook boundary for Engine verification, not authorization to blindly patch six bytes. It avoids changing global mouse variables or replaying callbacks. Retain original value clamping, repeat timing, float increments, change callback and keyboard/event handling.

420F90 is the temporary drag-release/pre-input callback installed by this handler. Its decoded body clears the parent slider's+2D word and5F7ECC, invokes420E60, returnsAL1; it does not read mouseX/Y. Do not inverse coordinates or suppress release there merely because the pointer lies outside the fitted content.

## Drag points outside fitted content

Canvas::inverse_pixel is an integer hit-cell mapping with margin rejection, not a continuous drag inverse. Normal point-inside conversion can reuse it. For an already captured drag, explicitly preserve the original ability to reach/clamp at track endpoints when the physical pointer leaves the content; a private bounded affine/extended coordinate conversion or an Engine-proved endpoint policy is needed. Do not globally clamp ordinary rectangle hit tests onto edge controls, and do not turn an out-of-content release into a skipped event. Shared rectangle admission and captured-drag conversion are different operations.

## Bounded validation

At the known Preferences modal origin, check slider center/end positions and one nested child origin. Compare private register conversion against drawn track coordinates; preserve original min/max and callbacks. Test click, drag across middle, drag beyond each end, release outside, reopen/cancel and keyboard adjustment if supported. Restore changed preferences. No broad menu reconstruction or networking work is required.

This refines SUBMENU-REUSE-MAP-04: the suspected custom coordinate path is now confirmed in original instructions. Rendering retains implementation; Engine verifies actual hook/ABI and event branches.
