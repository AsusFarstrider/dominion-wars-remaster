# Opening cards: bounded original-source contract

2026-10-27. For Rendering's user-authorized replacement-card candidate on frozen HD-video/HUD parent. Existing source plus decoded original BOLT resources only; no implementation, game launch, new export or ordinary-menu reconstruction. Rendering remains implementation/build/install/launch owner.

## Exact identities

| State / root | Actual original artwork | Palette / hold / input |
|---|---|---|
|5;4E31C8/id3E8; constructor46AB90|Resource101: **publisher/developer collage** — Gizmo above Simon & Schuster Interactive, Paramount and mass·media.|Palette100;6seconds;46AC40 queues6 and sets4D0A20=5.|
|6;4E3258/id3EA; constructor46AD30|Resource103: **Dominion Wars title** — starfield/emblem and game title.|Palette102;8seconds;46ADC0 queues7.|

The ordering is publisher first, title second. This is confirmed by decoding and inspecting original001/001 and001/003, not guessed from state names. Both constructors load group100 using49DBB0(global7138EC,100,0);49E1E0(global7138EC,100/102) supplies the respective palette. The shared painter46AC60 resolves the image through49E1E0(global7138EC,root+50). Root data+50 is101/103. The resource archive is original `rs/Bolts/ds9blt.blt`, SHA25640ad59d3…d2a8b. The global is a live resource-context pointer; do not substitute a fixed resource-object address.

Both roots: flags0,XY0,0,logical800x600,event mask4001,draw46AC60,no original child tree. Original resource images are type19,indexed8-bit,800x600,24-byte header plus480000 pixels; on-disk image flags0. Palettes are type15,774bytes,6-byte header plus256 RGB triples. Exact decoded hashes and root bytes are pinned in the audit.

## No original card fade/dissolve

Both palette headers are **steps0,flags0,range0..255**.4A2CF0's selected zero-step path applies colors immediately and disables its interpolation state; its nonzero-step branch supports palette interpolation generally, but these cards do not request it.46AC60 has no time-varying opacity, dissolve or palette-update loop. It normalizes the live image flags to `(old & FFFD) | 1`, forcing the ordinary opaque path. Do not confuse that mutation with a fade.

If image width is smaller than target width, original46ACA5 calls4A1710(target,0) before drawing. CPU source path calls49F050 at46ACEE; hardware bit8000 selects40F630 at46ACE2, with source wrapper+0E and accumulatedXY. The disk flag0 is not a guarantee that every loaded runtime wrapper remains software-only. There is no colored40F650/tint call in this painter.

Consequently a complete immutable native RGB565 replacement can be composed after the original root draw without recreating a card fade envelope. Retain the original constructor/palette side effects, clear and painter once. State5 also calls45C710(59F); preserve that media-related constructor call rather than replacing the constructor with a bitmap show operation. No blanket claim is made about other menu palettes, videos or transitions.

## Timer, input and completed-root boundary

Constructors set float deadline75FEF4 to engine timeA36D9C/A36F2C plus double6.0 at4BD850 or8.0 at4BDBE8.46AC60 draws first, then compares current time against that deadline and queues pending6 for current5, otherwise7. Do not reset the deadline, call the painter twice or delay transition for a fabricated fade.

46AC40 and46ADC0 contain only the state writes listed above: **no argument or mouse-coordinate reads**. They need no new coordinate parameters or callback replacement. Keep original click/key/Escape dispatch and event counts. Pointer admission still needs the declared mapping for the displayed card: if the replacement fills a different physical aspect region, the existing4:3 inverse must not silently leave visible sides unclickable. Keep logical roots800x600 and global native mouse; adjust only the scoped admission/publication policy if required. Keyboard handling is independent of replacement texel dimensions.

The proposed existing completed-root Lock is a suitable candidate seam. Bypass the800x600 snapshot/fit **only for the exact two initialized owners/resources** and compose the full declared native rectangle, respecting pitch/format/storage/lease checks and matching successful Unlock publication. Leave menus, movie history and other46AC60 users outside the exception.

One concrete phase hazard: the original painter itself queues5→6 or6→7 after drawing; input can also queue departure before that final draw. Completed-root validation must allow that exact current-owner/pending-next relationship while the root remains the original active owner. The parent already uses context(false) at completion. A new strict current=pending guard can expose a final low-resolution frame or drop the replacement on departure. Allow only the known departure; do not broaden arbitrary detached/changed-owner admission. Preserve original429330 presentation before cleanup/destination construction; never add a present to compensate.

Native density, aspect-fit/crop policy and the replacement asset's exact format/hash remain Rendering/Resolution's candidate decisions. This contract establishes original identities, effects and callbacks; it does not accept the new artwork or its runtime presentation. One bounded natural-timer/click/Escape check is enough for the next candidate; no further HUD/menu reconstruction is prerequisite.

Evidence: `evidence/opening-card-contract-01/audit.json`, private decoded originals `original-101.png`/`original-103.png`; six existing bodies/352 instructions checked against original and native EXEs. Reproduce with `scripts/audit_opening_card_contract.py`; manifest `validation/opening-card-contract-01-manifest.json`. Protected baseline identities remain unchanged.
