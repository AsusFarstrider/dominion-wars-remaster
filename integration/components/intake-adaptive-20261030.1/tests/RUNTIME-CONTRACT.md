# Production runtime contract — isolated candidate, October28,2026

Master owns package identity, installer and launch preflight. Rendering owns a new
derivative of exact gizmo-main-source-v5. Current and parents stay unchanged.

Root `version.json` is the exact existing versioning/identity.schema.json record.
Root `Remaster.Rendering.json` is a UTF-8/ASCII flat JSON object with exactly:

```json
{
  "schema": 1,
  "profile": "native-1440-v1",
  "engine_sha256": "de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6",
  "version_sha256": "REPLACE_WITH_SHA256_OF_EXACT_INSTALLED_VERSION_JSON_BYTES",
  "recording": "off",
  "paramount_hd": true,
  "gizmo_hd": true,
  "ss_hd": true,
  "publisher_card": true,
  "title_card": true,
  "main_background": true,
  "main_buttons": true
}
```

Both files are immutable manifest-hashed runtime files. Version hash is calculated
after serializing version.json; no wrapper/config self-hash cycle. Functional
hooks depend on this validated profile, not QFR environment switches. All asset
booleans false is the original-assets core profile; all true admits the exact
reviewed local replacements. A true movie option admits either its known original
or exact approved HD replacement. A false option refuses an installed oversized
replacement. Missing optional card/background payloads retain original artwork.
The object has12required fields; duplicate/unknown keys, nesting and malformed
types are refused. Metadata values are bounded ASCII, including JSON ASCII
unicode escapes. The implementation does not accept arbitrary paths or asset IDs.

`ss_hd` admits the exact281/30:1 source-density5C7 asset with original audio.
`main_buttons` admits the user v7 six35-state RGBA banks only, in
`rs/Remaster/MainButtons/{new_game,load_game,multiplayer,preferences,credits,quit}.dwbank`.
Every bank has a compiled SHA/resource layout pin. Original button descriptors,
180x100initialization/hitboxes, engine-selected resources and timers are retained.
Actual postlookup resource receipt selects a720x400 frame; ordered mixed native
RGBA/raw foreground keeps modal fills/text above sprites and exposes fresh
background through new alpha holes. Missing/malformed/unallocatable banks retain
the original family and record that fallback explicitly; installer pins and live
acceptance must verify the intended v7 family actually loaded.
Unknown/mismatched HD media fail closed. No hardware/mode generalization is implied.

Recording off suppresses recorder startup even with inherited QFR_DIR. An explicit
diagnostic package may choose recording compact and supply QFR_DIR as output;
recording does not control functional features. A separate small production
startup/readiness/label/safety ledger remains independent of the recorder at
`%LOCALAPPDATA%/Dominion Wars Remastered/Rendering/rendering-PID-FILETIME.ndjson`.
It needs no write permission in the game installation. Each selected major event
is written and flushed immediately; routine frame drawing is not recorded there.
It is bounded128events, not a recorder-complete frame log or exit receipt. The
managed launcher remains responsible for process exit/candidate acceptance logs.
The functional transition hooks also remain active with recording off: frame
preparation and state-install ownership invalidation are required for HUD/UI.
Readiness requires all six functional components including those transitions.
Master should preflight actual selected display/client/target2560x1440 before game
launch; runtime mode/output gates also remain active.

Candidate label is `Remastered 1.10`; alpha/build/source/flavor stay in metadata.
The provider validates the complete identity relationship and substitutes only
the verified Main font-call stack argument, using original glyphs/palette once.
Actual metrics must fit the original logical canvas; the80-character schema
storage bound is not assumed to fit the screen.
