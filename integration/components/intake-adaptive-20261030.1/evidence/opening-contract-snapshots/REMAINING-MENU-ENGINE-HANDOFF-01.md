# Remaining Main destinations: bounded engine handoff

2026-10-25. For Rendering's coordinated submenu candidate after the user passed both faction openings. Original executable SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`. Assessment and byte mapping only; no implementation, game launch, networking restoration, new agents or Current edits. This is not a new source-review round.

## Owner and callback map

All addresses below are hexadecimal; state numbers are decimal. Static records are pre-attachment values. Parent/next/child fields are +77/+6B/+73, respectively, not the packed control-specific fields near +10.

| Destination | Owner, constructor, root | Painter / input | Geometry and disposition |
|---|---|---|---|
| Preferences | Main current/pending 7; constructor 436150; root 4DB6F8, id458, flags80 | Root4342D0 / NULL; static child4DB788 id472 input436520; dynamically created sliders use420FB0 | Modal global head over Main. Root400x450. Constructor centers with actual W/H. Logical origin should be200,75. |
| Load | Main current/pending7; constructor44F550 calls434390(root4DF100, child4DF190); root id47D, flags80 | Root4342D0 / NULL; static child4DF190 id485 input44F780; dynamic list47E uses420020/41FC20 | Modal global head over Main. Root600x450. Shared434390 centers with actual W/H. Logical origin should be100,75. |
| Credits | State15; constructor474DF0; root4E57B0 id4C1 flags0 | Draw474A50 / input474980, eventmask4001; cleanup474E80 | Full screen authored800x600 at0,0. No native-size centering in this owner. Background cycles resource300..305 at0,0; scrolling uses600 bounds. Click event0 or Esc event14 queues Main7. Keys96/97 adjust speed. |
| Multiplayer entry | State20; constructor4614E0; root4E05E8 id2007 flags84; static child block4E0678 | Draw461BF0 / root inputNULL; generated controls retain their own handlers; cleanup461BA0 | Full-screen menu owner. Constructor writes800x600 and native-centered XY at461574/461585. Normalize rootXY0,0 for authored canvas. Draw begins full-native zero fill4A1710, title imageE01 at278,27. |
| Multiplayer next owner | State21; constructor4628C0; root4E0828 id2011 flags84; children4E08B8 | Draw462BD0 / root inputNULL; cleanup462B60 | Full-screen menu owner with static zero W/H and no constructor W/H assignment in the checked body. Do not reuse an800x600-root-size guard blindly. Painter begins full-native zero fill, imageF01 at265,34, further content parent-relative. |
| Multiplayer setup owner | State22; constructor463E20; root4E1908 id2026 flags4; children4E1998 | Draw464BB0 / root inputNULL; cleanup464AD0 | Full-screen owner with static zero W/H, no constructor W/H assignment in the checked body. Painter begins resource1216 at root+parent XY, then1217 at parent205,13. Root-specific tree validation and the authored ROI are more useful than assuming every root has nonzero bounds. |

The existing state dispatcher429980 assigns states15/20/21/22 normal menu input42A210, secondary42A230 and draw42A290 on normal menu entry. State20+ constructors can fail, display confirmation/error roots or transition onward. Preserve these original branches; this map enables visual sizing, not multiplayer networking. Later states23+ are not generalized by this handoff.

## Modal composition boundary

Main46B440 dispatches animation completion event9999 through46AFA0 during a child paint. Load and Preferences attach a new global head while current/pending remain7. Constructor centering is therefore not a separate display state, and pending-state-only admission remains insufficient.

Normalize only the identified modal roots before their first paint. Use the same authored origin throughout input and paint, including custom drag/row consumers. Do not temporarily spoof global screen dimensions. These dialogs share painter4342D0: frame423A80 followed by title drawing using root+parent coordinates. Keep flags80 and original modal input ordering.

Traversal421E80 paints a node's next siblings first, then its own callback and descendants. Thus a modal head's underlying Main sibling is painted first. Disable the pure-Main publication/fit while that foreign head is present; let original Main rebuild the unscaled background, let the modal and children paint at their normalized origins, then snapshot/fit that completed composition once. An outer traversal boundary after the whole admitted forest is the cleanest shared boundary, before tooltip422030 and native cursor43B140. A stale previously fitted Main image is not a valid popup background. A matching callback return alone is insufficient if remaining siblings/children have not completed. Do not replay Main or dialog callbacks to manufacture a frame.

## Custom input consumers and private hook witnesses

Rectangle admission alone does not convert the coordinates used by sliders, list rows or scrollbar drags. Preserve the original global mouse words5F7E5C/X and5F7E5E/Y; convert only private registers/locals for a validated committed owner.

| Consumer | Exact bounded witness | Available private state and continuation |
|---|---|---|
| Preferences slider input420FB0 | Loads signed native X intoEBX at420FB4 and Y intoEBP at420FBC; loads widget intoEDI at420FC5 | Candidate bridge420FC9, original bytes `8b4f778bc1` (movECX,[EDI+77]; movEAX,ECX), resume420FCE. At bridge: EDI=actual slider/control, EBX=X, EBP=Y. Replace only EBX/EBP with the inverse-authored point under the admitted Preferences ancestry; replay overwritten instructions and preserve other registers/flags/stack. |
| Load list/scroll input420020 | Loads signed native Y intoEBX at420024; widget intoEDI at42002E | Candidate bridge420032, bytes `8b77778b4642`, resume420038. EDI=actual list/control, EBX=Y; overwritten instructions load ESI=parent and EAX=[ESI+42]. Inverse-map private EBX for admitted Load ancestry. This does not cover row helper's independent X/Y reads. |
| Load row hit helper41FAE0 | Independently loads native X intoECX at41FAE3 and Y intoEAX at41FAEA; caches them at41FB09/41FB0D | Candidate bridge41FB09, bytes `89442414894c2418`, resume41FB11. ESI=actual row/list widget, ECX=X, EAX=Y; EBP/EDI and parent offsets already belong to the original geometry. Convert ECX/EAX, replay both cache stores. Original list input calls helper at42007E, return420083; restrict to that known path and Load ancestry for this candidate. |

These are original-byte hook witnesses, not drop-in C function signatures or a tested detour implementation. Each bridge still needs the established register/flags/stack-preserving machinery, a matching committed transform and owner epoch. On uncertain scaled-owner state, quarantine activation/drag consistently with the passed controller rather than processing native pixels as authored coordinates. An unrelated control must retain original behavior.

Preferences436150 creates slider ids459/45D/465/461/469 (difficulty, game speed, music/SFX/voice), plus optional47C gamma. It installs shared420FB0 on generated parts. Shared slider release callback420F90 reads capture owner5F7F98, gets its parent, clears temporary field+2D and callback5F7ECC, then calls420E60; it does not reread native mouse X/Y. Retain original capture/release, timer and value notification behavior.

Load44F550 creates list47E at142,62, with visible-row argument11, data/count7359D0/735A24, resources403/406/40D/402/409; sets font field+62=24 and margin+64=3. Shared4206C0 installs painter41FC20 and input420020 on generated list parts. Row helper41FAE0 tests both cached coordinates, accounts for parent offsets and visible-row image geometry; the input path combines returned row with first-visible field+2E and stores selected field+30. Scrollbar and page controls also enter420020 and use its native-Y-derived EBX. Shared release41FF30 uses capture owner5F7F98 and parent geometry, clears temporary field+32/callback5F7ECC, and updates knob position from first-visible index; it does not reread native mouse coordinates. This is why a private list-Y bridge plus the separate row-helper bridge is needed. Original file loading/deletion/cancellation behavior in44F780 remains unchanged.

## Evidence and practical next step

`scripts/map_remaining_menu_boundaries.py` reads the original PE using the existing `selection-phase-policy-01/functions.jsonl` catalog and a small explicit set of callback byte windows. The windows are labelled as windows, not inferred function extents. It records root/child data and original instructions in `evidence/remaining-menu-boundaries-01/original-boundaries.json`. Existing exports supply436150/44F550/434390, state dispatcher429980, traversal421E80/421F70 and attachment41D590/41D470. No new Ghidra export was needed.

Rendering can now build one coordinated submenu candidate with Credits/full-screen MP ownership, the two normalized modal roots, one completed-composition fit and the scoped custom coordinate consumers. Test real Preferences dragging/release, Load row selection/scroll/cancel, Credits exit and reachable Multiplayer UI/back behavior. Keep networking failure/success as original behavior, HUD separate, and preserve the passed opening source for comparison. This handoff adds no requirement for another review round before that bounded work.
