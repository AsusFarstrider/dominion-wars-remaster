# Main HD button banks and S&S startup movie

2026-10-28. Bounded existing-source handoff for Rendering. **Keep the original animation/resource choice and substitute artwork for the exact draw it selects. SSLogo1 is the existing5C7 startup lifetime, not a new screen or sequence.** Textures owns the supplied asset catalog/audio conversion; Rendering owns runtime/build/testing. Engine modified no executable/runtime and launched nothing.

## Six original button banks

Main root4E47C0/state7 uses painter46ADD0; all six buttons use painter46B440 and input46AFA0. Positions and hit rectangles remain logical; each initialized button is180x100. Static dimensions are zero: original41D470 derives them from the original base image. Do not substitute720x400 into those original descriptors or initialization queries.

|Button / node / id|XY|Idle|Hover indices0..24|Activation indices0..8|Hover wrap start|
|---|---|---|---|---|---|
|New Game /4E4850 /3F3|320,50|202|203..21B|21C..224|12|
|Load /4E48E0 /3F4|320,115|26B|26C..284|285..28D|12|
|Multiplayer /4E4970 /3F7|320,185|225|226..23E|23F..247|11|
|Preferences /4E4A00 /3F6|320,255|28E|28F..2A7|2A8..2B0|12|
|Credits /4E4A90 /3F8|320,325|248|249..261|262..26A|12|
|Quit /4E4B20 /3F5|320,395|2B1|2B2..2CA|2CB..2D3|12|

Resource numbers are hexadecimal; counters/wrap starts are decimal. Node+50 contains0x000C1909, except Multiplayer0x000B1909: byte0=9 activation frames, byte1=25 hover frames, byte2=12/11 hover wrap start. The supplied v3 is reported as35 RGBA720x400 frames per button, exactly4x the original180x100 canvas. Map catalog ordinals explicitly: idle→base, hover ordinal1→counter0, activation ordinal1→counter0. Do not infer bank identity from directory or lexical filename order alone.

46B440 reads the counter's low byte at+54, then writes the result as a DWORD. Without activation100000 or hover20000 it selects idle and resets counter0. Hover selects`base+1+counter`; activation takes precedence and selects`base+1+25+counter`. Advancement occurs only when shared deadline7606DC is strictly less than engine tickA36DB0; it sets deadline=tick+5 and advances once, with no catch-up. Hover wraps25→12 (Multiplayer11), not25→0. There is no reverse/ping-pong branch. Preserve the original shared clock rather than imposing a new playback FPS. The painter determines which frames actually appear.

Activation clamps to8 and calls46AFA0(node,9999) at46B4AC/return46B4B1 **before** storing counter8 and before the final resource lookup/draw. The callback resets+54 on entry, restores sibling disabled flags and clears activation; the painter subsequently writes8 and draws the final activation image anyway. Event0/keyboard shortcuts play1517, set selected activation100000 and other-five disabled10000.9999 requests NewGame10, Quit18, Multiplayer20, Credits15 or attaches Load/Preferences while pending remains7. Keep these operations once, including the existing causal modal/departure handoff; postcallback flags alone cannot identify the final activation image.

Lookup is49E1E0(7138EC, selected resource), then CPU49F050 call46B579/return46B57E. The original8000 alternative calls40F630 at46B56B; it is not covered by raw-P8 admission. All210 original bank images are rawP8 flags3, AND255/XOR0,180x100; zero source index is transparent before palette lookup. Bind replacement to the admitted owner/node, actual resource/source and exact producer, not image dimensions alone.

41D470 inserts children at the head: live root.child=Quit and next-links descend toward NewGame.421E80 paints next siblings first, so original drawing is NewGame→Load→Multiplayer→Preferences→Credits→Quit. Input tests the opposite order and retains overlapping rectangular hitboxes. A modal's next=Main paints underlying Main before its own controls. HD sprites must retain that order relative to modal fills, text and other primitives; a final batch painted over the completed modal is incorrect. The existing source contract already records these mechanisms; this handoff makes their HD-asset implications explicit.

**Density/coverage hazard:** retain180x100 layout/input and use720x400 solely as replacement source density. The selected original P8 branch provides binary keying, not a demonstrated RGBA alpha compositor. Use the asset's explicit alpha in the new provider; never infer transparency from RGB black or reuse old indexed coverage as the new shape. Old sprite pixels must not survive beneath transparent replacement pixels in the shared foreground. Admit replacement only when the complete ordered composition/fallback path is ready; reuse the existing single-publication, target-lifetime and coverage-gap safeguards.

## SSLogo1 / external5C7

Verified original `rs/Video/SSLogo1.bik`: SHA256`e444e02185d1fd7d7c7be5edf3bd2842cd43cfe9a89e715a2542ce59c0f7249e`,3,885,492bytes,BIKi640x480,281frames,rate30/divisor1, nominal281/30=9.366666667seconds. One44100Hz audio track; original12-byte metadata`001c020044ac00e000000000`. Preserve its own compressed packets/metadata/timeline; supplied1440MP4 properties remain Textures' catalog/conversion responsibility until the immutable encoded asset is supplied.

453910 registers SSLogo1 under external5C7 via454640. Resolve its runtime internal index through DWORD[749020+4*5C7]; do not put external5C7 directly into the internal slot identity. Owner remains startup state4/root4E3138/id3E9, slot index4E47B8.46AA40 starts5C5 after455F20(80,60). On completion46AAE0 consumes flag4E47BC, clears it and queues5C7 **without changingXY**: normal S&S placement inherits80,60. Next optional5C6 sets0,0;5C9 sets0,118; exhausted flags lead to pending5. Do not force all logos or reorder them.

Skip46AAC0 clears presentation bitA38776&2, invokes454A00 to stop media, and clears only4E47BC. A skip during Paramount can therefore suppress the still-pending S&S clip; a skip during S&S leaves the subsequent Gizmo/Intro flags intact. Retain the original callback and close/lifetime progression rather than treating a skip as “exit all opening videos.” Open failure/negative slot follows the existing done progression; slot/handle reuse must start a new lifetime.

Extend the existing explicit HD profile/readiness and installed-asset refusal checks to5C7. Original public CopyXY80,60 is distinct from private2560x1440 historyXY0,0/pitch5120. Preserve one original Wait/Do/Copy/Next, original clears/presents, full initialization until conversion0 and subsequent selective history. Reuse source-native pixel/Unlock/lifetime admission; do not substitute Paramount's225/15 or Gizmo's601/2997:100 guards. HD5C7 must never reach legacy Copy merely because its profile or startup readiness is missing. No new engine timer is necessary.

Exact original/native byte checks, static bank fields,210 original frame headers and original movie-header/audio pins are recorded in`evidence/main-buttons-sslogo-contract-01/audit.json`. Existing MAIN-MENU-CANVAS-CONTRACT-01, MAIN-FLAGS13-CONTRACT-01 and BINK-NATIVE-FIT-CONTRACT-01 remain controlling for input, clip and decoder behavior. No broader reconstruction is requested; release remains held for the new assets and tested implementation.
