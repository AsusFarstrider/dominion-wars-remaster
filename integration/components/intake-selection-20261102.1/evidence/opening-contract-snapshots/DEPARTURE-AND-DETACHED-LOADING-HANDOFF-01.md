# Departing Main and detached loading updates

2026-10-25. Bounded follow-up to Rendering's run03 findings. Existing exports and previously mapped original constructor bytes only; no new export, game launch, implementation review or Current edit. These findings explain why the previous eligibility tests can reject real presentation paths; they do not constitute a live pass for a correction.

## Main to Multiplayer

Original frame429330 calls ordinary present4A79B0 at **4293C3** (return4293C8), then later cleanup429FB0 at4294C6 and destination constructor dispatcher429980 at4294CC. Thus the departing UI is presented **before** destination initialization.

Main handler46AFA0's Multiplayer completion event9999 sets pending20 and calls45C6C0 (audio teardown); it does not synchronously call Multiplayer constructor4614E0. Constructor4614E0 is called later from429980 at429D7A. There is no direct4A79B0 call in the checked bodies46AFA0,45C6C0,4614E0 or429980. This is a direct-body statement, not a proof about arbitrary reentry from operating-system/network calls.

Inference from this ordering: if the final Main fit is rejected for the proven pending20 departure,4293C3 can present the original-size image. It then remains visible while4614E0 initializes the network stack. A new synchronous constructor present is not necessary to explain that symptom. Preserve a fitted visual-only departing composition through the existing present for the known causal transition; keep input eligibility separate. Load/Preferences change topology while current/pending remain7 and need their once-only combined modal composition rather than a pending-state exception alone. Preserve original callbacks and presentation count.

## Loading detaches during the constructor

Existing42B040 instructions establish this exact sequence:

| Call site | Operation |
|---|---|
|42B052|4344B0 attaches/configures loading widget4DA738 and sets active7180B5=1|
|42B0D9|434560 initial progress draw/present|
|42B122|434560 progress0.2 draw/present|
|**42B15C**|**41D330 resets the UI tree**|
|42B16B|434560 progress0.3 draw/present, now after the reset|
|42B192,42B1D1,42B1F5,42B310,42B32F,42B393,42B48D|Additional direct434560 loading draws/presents|

41D330 walks the old global child chain, unlinks nodes, zeroes parent/next/previous/child fields and reinitializes global root5F7ED8. It does not clear active7180B5. The static loading widget remains readable, and434560 draws from its data without requiring attachment or a particular global head/sibling topology. Consequently an attached-global-head guard admits the two early updates and rejects later progress calls. Rendering's two commits under old state11 are consistent with this source ordering.

Dispatcher429980 writes current/pending to the new destination only after its constructor returns (normal completion writes at429DD6/429DDC). Therefore direct progress during42B040 can still see old current11. Stable current1 and ordinary menu phase guards are not valid eligibility requirements for this synchronous route.

Use the bounded actual434560 invocation, exact static widget id493/draw434700/geometry, progress value and a fresh native target/epoch as ownership evidence. Detached topology is expected here; allow it specifically for this direct route, without admitting arbitrary detached callbacks or presents. Keep the ordinary widget traversal adapter separately scoped. Preserve the mapped clip restoration4346ED and fit before original direct present4346F3 (caller4346F8), exactly once. Do not fit after updater return.

The fresh surround and authored widget origin must also apply to these later **detached** direct draws. If only early attached updates are fitted, later original-size400x110 panel writes can coexist with pixels from a preceding enlarged panel. That is a plausible explanation of the reported doubled bars; it is not a pixel-level diagnosis of run03. The direct updater still does not define pixels outside its small panel, so the previously identified fresh-surround requirement remains.

Evidence: existing429330/429980/42B040/41D330 exports,46AFA0 and45C6C0 exports, and the already captured original4614E0 body. `evidence/departure-detached-loading-01/audit.json` pins these and byte-checks instruction witnesses against original SHA256 `03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a`. No broader reconstruction is needed for these two corrections.
