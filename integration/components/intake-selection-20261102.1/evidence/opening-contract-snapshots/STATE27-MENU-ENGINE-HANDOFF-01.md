# State27: bounded Skirmish setup handoff

2026-10-25. Rendering observed state27 after Selection10 in the first submenu run and requested the smallest existing-engine map. No new Ghidra export, implementation review, game launch or Current edit. Source bytes are original SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`; background assets are from original BOLT SHA256 `40ad59d38a6424558f80837735a47b5b9083b35c298ba4e49e8198a6f7bd2a8b`.

**A small same-canvas extension is plausible.** This owner already paints an authored800x600 background. Its zero-sized, id-zero root must be recognized by its exact address, flags, callback and live tree rather than rejected by a generic nonzero-id/size test. Reuse the corrected completed outer traversal once; do not change the scene renderer or add another projection path.

| Field | Original finding |
|---|---|
| Entry | Selection input4729F0, button443 or keyS, queues decimal27 (hex1B). |
| Constructor | 487D40; attaches root at487F30 and child block4E9C08 at487F5B. Loads group1200; its checked body contains no native-size centering writes. |
| Root | **4E9B78, id0, flags4, XY0,0, W/H0, inputNULL, eventmask0, draw488560.** ID0 is valid here; it is not a child-table terminator at this known root address. |
| Phases | Existing429980 state27 branch uses normal42A210 input,42A230 secondary,42A290 draw on ordinary menu entry, as observed by Rendering; no scene callback in that observed owner. |
| Cleanup | 4884A0 through48855D, destroys generated controls and detaches root. |
| Complete painter | Bounded488560..488AEF contains original image/font/palette operations and helper469520. No direct present in this window; fit the completed root/child forest before tooltip/cursor through the corrected shared boundary. |

Painter488560 fetches resource1218, draws it at rootXY+parentXY, then resource1219 at parent205,13. Independent asset decoding gives:

| Resource | Bounds | Format |
|---|---|---|
|1218|800x600|8bpp, flags1, AND mask255, XOR0; full authored background|
|1219|379x21|8bpp, flags3, AND mask255, XOR0; overlay|

The remaining painter writes font positions in the authored space (for example258,323;266,361;258,388;260,444 and headings at287/497/611/682,60). Its eight player rows use authored positions through X767. Helper469520 draws small color rectangles as repeated49EA30 span writes to the same screen wrapper; its function ends4695B3. It is not a 3D preview. Custom generated child painter489860 uses41EC90/image49F050 or40F630 and41EA00 text. The native target completion snapshot accommodates both original raw/CPU and tagged/GPU image alternatives; it must occur after descendants, not immediately after the root callback.

Static child block4E9C08 has these pre-attachment records; original attachment resolves image-derived zero dimensions:

| Address / ID | XY | Flags / input |
|---|---|---|
|4E9C08 /4|20,13|200 /488AF0, eventmask1|
|4E9C98 /1|19,556|1 /488AF0, eventmask1|
|4E9D28 /2|663,13|1 /488AF0, eventmask1|
|4E9DB8 /3|247,556|200 /488AF0, eventmask1|
|4E9E48 /9|268,489|1 /488AF0, eventmask1|

Constructor487D40 also creates selection/combo controls with41F140 and handler488AF0. Its call list contains neither the slider4217C0 nor list4206C0 constructor. The checked488AF0..48965F byte window contains no direct read of native mouse words5F7E5C/5F7E5E. No additional private-coordinate bridge is identified by this bounded check; retain the existing ancestry/private predicate and original generated-control behavior. This is not a proof about every helper or popup reached subsequently.

One concrete overlay must be included: **488FD5 calls Preferences436150 without first leaving state27**, after saving current owner at4E3134. The same400x450 Preferences root therefore needs the already-mapped modal composition/slider handling over this owner, not just over Main7. Do not fit state27 independently under a later popup fit. Preserve the original pending-state changes: handler branches queue18,7,29 and11; constructor has an onward28 branch. This map does not claim that28/29 or arbitrary confirmation roots are covered.

Rendering reports the first submenu candidate's shared completion token had a12-byte saved-stack mismatch, which it has corrected while preserving the failed run/source. This handoff does not review that fix or recast the first run as passed. The next practical test should distinguish successful full-screen fitting, state27 control alignment, Preferences roundtrip and original onward/back behavior.

Reproduction: `scripts/map_state27_menu_boundary.py` uses the existing function catalog and a few labelled original byte windows, plus decoding just1218/1219. Evidence is `evidence/state27-menu-boundary-01/original-windows.json`. No broader reconstruction is required to admit this specific owner for a bounded test.
