# Integration intake — October18,2026

Inspected latest conversations plus source, patch reports, game images, profiles,
test results and source repository state. Conversation status is corroborating
evidence, not the sole source of component identity. Do not use earlier Master
summaries as current status.

| Area/chat | Latest inspected work | Integration decision |
|---|---|---|
| Dominion Wars Resolution | selection-loading-v3, exe EA1EA1B8...C29139B; visually passed menu repaint, fleet origin, objectives and loading panel; logical960x60016:10 with display-fit rendering | Include exact reviewed resolution parent, preserve its hooks. Remaining screens/long sessions still need checking. |
| Dominion Wars Launcher | input-window-v2 exe FB972D8F...A9E91CD7 plus no-resize-focus-v2 helper; user pass with logged stale-Windows-bit repair on two taskbar returns | Port native input hook after resolution's occupied code, use no-switch launch and guarded helper. No four-minute observer/automatic game shutdown. |
| Dominion Wars Compatibility | Modernizer0.6.0-alpha, committed foundation4f889dea81385c3a604d09eb82452a1b48fafa25 (later README commit; runtime same release); established retail extraction/official updater/compatibility install | Reuse immutable committed foundation behind remaster entry point. Do not add remaster code to Modernizer. |
| Dominion Wars Textures | all-state Saber8BC7AC0F...81A1DDA0; initial natural-combat pass in20261017T160412094Z; detachment user-reported, not pictured | Optional PRIVATE owner-media-derived art delta. No public clearance inferred. Original geometry retained. |
| Native geometry | restrained/balanced cohesive concept;21 tests; bounding and light-coordinate work pending; most recent concept not packed/launched | Exclude from runtime. Preserve branch files. Earlier isolated ridge was judged too pronounced. |
| Dominion Wars Captains | Bilecki100x115,35x40,20x28 reference set, shared palette constraints; no replacement runtime pass | Exclude portrait replacements. Keep findings as asset-interface constraints. |
| Dominion Wars Release | updated MASTER-RELEASE-HANDOFF.md; existing shared Git initialized/no commits/no remote; proposed separate remaster repository is a future assignment | Use existing workspace history for integration source only. No remote creation, push or public package. Supply immutable handoff under handoffs/master. |

## Actual overlap and reconciliation

Both executable branches descend from layout-trace-v4(827D3183...D377357).
They append to .dwui starting at the same old address0xABD200. Latest resolution
uses that region for fleet/menu/loading adapters; launcher v2 uses it for recenter
logic. Bytewise overlay is unsafe. The integrated emitter applies the exact
resolution transformation, then retargets only the input builder's base hash and
appends its unchanged native body after the used resolution region. Existing
hook/import/zero-capacity guards remain. Tests compare every changed byte and
confirm no resolution bytes outside that scoped port differ.

Recovery-read race fix is already present in the latest resolution modules,
unlike the established Modernizer runtime. Include these pinned modules in the
remaster layer, not by overwriting the compatibility project's newer files.
The new launcher is distinct from Modernizer's normal/optional focus entry points.

Wrapper0E4790D2...16CB566 is the exact resolution diagnostic1 build based on
DDrawCompat0.7.1/focusv3b. Corresponding source, license and build report are
captured. Bounded telemetry is opt-in; the normal integrated launcher clears
experimental telemetry flags. Any uncertainty about disabled-telemetry/combined
runtime behavior requires a real combined smoke test, not a wrapper-name assumption.

## State and provenance

No prior authoritative Current Build pointer or approved integrated revision was
found. Established original/Modernizer installations and independent research
copies remain the fallback; none is renamed as an approved remaster baseline.
Source intake locks532 files by size/hash, plus a source-lock hash. It includes
Modernizer files fetched from its commit, the exact native port source, a minimal
resolution delta, matching wrapper source and an original-game HASH inventory.
No original game binary/archive enters source. Private optional Saber delta stays
under E:/Codex/remaster-private-inputs, outside the source repository.

New packages are integration candidates until combined tests and user acceptance.
Existing branch passes do not automatically satisfy promotion criteria.
