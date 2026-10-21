# Dominion Wars Remastered versioning policy

Owner: **Master Integration**. Established October 21, 2026.
Machine-readable authority: [versioning/registry.json](../versioning/registry.json).

## Standing goal: Version 2.00

**2.00 means Dominion Wars has been successfully remastered.** It is the point at
which the core restoration objectives have been substantially achieved and the
result is a reasonably polished game ordinary owners can install and enjoy.
It is not a deadline, a completion percentage, or a promise about the current build.
Everything afterward is continued development.

Master decides when that milestone is earned from validated integrated progress.
The assessment must address:

- Practical modern compatibility, with tested coverage and honest fallback limits.
- Reliable gameplay and saves, supported by representative play rather than only
  compilation, one menu, or one successful mission load.
- Modern-resolution presentation with appropriate layout, input, focus and display
  behavior across a justified range of outputs.
- Appropriately restored graphics and interface elements, with the original game's
  identity and recognizable presentation preserved.
- Ordinary installation, verification, updating/recovery and launch/shutdown that
  are understandable and reasonably dependable for players.
- A coherent, reasonably polished experience, with serious known problems resolved
  or bounded and accurately documented.
- Faithfulness to Dominion Wars' original engine, gameplay, campaigns and identity.

These are evidence categories, not equal-weight percentages or a requirement to
replace every asset. Satisfactory original artwork may remain. Complete QUANTUM
source reconstruction, every hypothetical graphical improvement, speculative new
features, perfect hardware coverage and zero bugs are **not** prerequisites.
The first-Federation-mission restoration remains a useful earlier project milestone,
not by itself proof that the broader 2.00 goal has been earned.

## Three identities, never conflated

| Identity | Meaning | Current recorded value |
|---|---|---|
| Official original-game version | Supported original game's official patch/input identity | North American retail **1.04** |
| Remaster version | Master-approved player-facing restoration milestone | **Not separately assigned to the legacy first release** |
| Integration build ID | Exact experimental assembly, with source revision, flavor and hashes | **int-20261018.1**, original-assets |

An altered executable derived from 1.04 is not the unmodified official executable.
Preserve both its original-input SHA256 and generated-output SHA256. A PE version,
wrapper version, recorder version, reconstructed-source revision, date or Git count
must never be used as the remaster version.

The published first experimental release retains its historical integration-ID tag,
filename, approved ZIP and receipts. Its menu still displays the original label
`Version 1.04`; that is **not** an approved numeric remaster release. Do not rename,
retag, patch, or retroactively repackage it just to introduce this policy.
Later publication authorization supplements earlier immutable approval snapshots;
it does not require rewriting their historical contents.

## Numbering and advancement

Remaster numbering **continues after the official 1.04**, with **1.05** as the
first available remaster label. Player-facing versions use
**major.two-digit-milestone** strings: `1.05`, `1.06`, `1.07`, `1.08`, `1.09`,
`1.10`, `1.11`, potentially `1.25`, `1.54`, `1.75`, `1.90`, and ultimately `2.00`.
These examples describe the allowed scheme, not assigned versions, reserved
feature assignments, schedules or promised increments. This clarification does
not itself assign1.05 to the already-published historical package.
Keep values as strings: `1.10` must not become floating-point `1.1`. This is a
project milestone convention, not a claim of strict semantic versioning.
Always preserve two digits in game, launcher and player-facing releases; shortened
labels such as `1.1` or `1.2` are not permitted. Continuing the sequence does not
make a remaster release an official original-game patch: label that distinction.

Versions may skip numbers when Master's validated milestone assessment warrants
it. `1.25` is **not**25% complete, `1.75` is **not**75%, and no numeric span creates
a requirement to invent intervening releases. `2.00` remains reserved for a game
that can fairly be called a completed remaster, with further development expected.

Keep integration IDs and tooling-specific package versions compatible with their
actual tools. If a tool requires a different numeric representation, record an
explicit mapping in separate tooling metadata (or a reviewed schema revision),
without changing the canonical player-facing string. For example, a tool requiring
three integer components could map display`1.05` to internal`1.5.0`, and display
`2.00` to internal`2.0.0`. These are illustrative adapters, not selected package
versions or shortened labels to show players. Do not silently add fields to the
strict shared identity schema. Compare canonical versions as integer pairs, never
as floating-point values or a tooling-formatted display string.

Pre-2.00 remaster versions are above the official 1.04 base and below 2.00.
Experimental/alpha/beta/release-candidate status is a **separate field** and must be
visible in the appropriate player materials. Numeric advancement alone does not
mean stable, approved for distribution, or safe on every system.

Master may advance the number only after reviewing a meaningful **integrated,
validated** improvement or coherent milestone. Possible grounds include a major
presentation capability, a substantial restored-content slice, or a material
compatibility/reliability milestone supported by actual tests. No fixed step size
or one-feature-one-number rule is prescribed.

Routine fixes, small asset revisions, refactoring, documentation, dependency
rebuilds and diagnostic/research experiments do not automatically advance it.
Accumulate routine improvements under the existing remaster version, using new
integration IDs/revisions and checksums to distinguish changed artifacts. A major
repair may justify an explicit milestone review, but never an automatic bump.

Every materially different candidate needs an identifiable build/source/flavor and
hashes even when its remaster number stays unchanged. Never silently replace a
published package with changed bytes under the same artifact identity. If an older
build's numbering practice was less strict, preserve its history and disambiguate
with recorded source/package hashes rather than rewriting it.

For each checkpoint Master records: advance/hold/defer; integrated changes; test
evidence and coverage; unresolved limitations; the reason the milestone is or is
not significant; selected version and stage if approved; and exact baseline/build
provenance. Candidate preparation may propose a value; it is not an approved player
version until Master accepts the evidence and records the decision. User acceptance
of an internal baseline and user authorization for public publication remain
separate decisions. Neither this policy nor a version number grants publication.

## One value throughout a versioned package

For a future versioned candidate, generate a single package version record from
the selected Master decision. It should identify official base version, remaster
version, release stage, integration build ID, source revision and decision ID.
Use that record for installer/launcher labels and diagnostics, the game's version
label, package manifest and accompanying documentation/release materials.

Select the immutable version metadata before packaging; after validation, bind the
approval to the resulting manifest and ZIP hashes in an **external acceptance or
publication record**. Do not embed the final ZIP's own hash inside its contents,
creating a circular checksum dependency. Historical approved packages stay frozen.

The future in-game label should clearly identify the remaster, for example
`Remastered 1.10` (illustrative, not approved). The official base `1.04` and exact
build ID remain separately available in diagnostics/about/support information.
Where room is limited, do not squeeze all metadata into the menu label.

Missing or unapproved version metadata must never advertise `2.00` or an invented
release number. Research builds should be plainly identified by their experimental
build ID. For the existing legacy release, retain its known original label and
build-ID documentation rather than pretending a new provider is already installed.

The legacy int-20261018.1 builder does not yet consume this registry or generate a
shared game/launcher version provider. This policy does not claim otherwise. That
binding belongs in a separately identified, verified future integration candidate.
Do not bypass normal integrity checks to add it to an existing installation.

The prospective packaged identity contract is
[versioning/identity.schema.json](../versioning/identity.schema.json). It is one
immutable, manifest-hashed `version.json`, not a second mutable runtime registry.
`version_decision_id` identifies Master's selection; that field alone is not proof
of baseline or publication approval. A trial label can be tested in an explicitly
experimental candidate without declaring the version earned. Final acceptance
binds the selected identity and tested package externally, without changing bytes.
Invalid present identity metadata/hash/build disagreement must fail managed
verification, not silently fall back. Only the identified legacy absence/null path
retains the known original label and build-ID information. Python/schema checks
are developer tooling, not a new player dependency.

## Implementation responsibilities and safety

- **Master:** owns policy, significance assessment, selected version/stage, immutable
  decision-to-build mapping, combined validation and eventual 2.00 decision.
- **QUANTUM Source Reconstruction (QUANTUM Engine):** supplies the exact original
  label/font/measurement/draw contract and safe string ownership/lifetime boundary.
  No broad replacement of every `1.04` string or in-place overflow of a longer label.
- **Rendering:** binds the approved label to the presentation path, preserving
  measurement, placement, clipping, draw order, fallbacks and resource lifetimes.
  Existing font probes retain their original known label until a separate candidate
  deliberately changes selectors/contracts. Sharper-text research is not itself a
  remaster version increment.
- **Launcher:** consumes shared approved metadata for relevant startup/about/log/
  shortcut information; does not derive numbers from PE/wrapper versions or dates.
- **Documentation & Releases:** maps approved remaster version/stage to integration,
  source and exact package; checks all distributed labels/materials agree. It does
  not select a new milestone or silently rebuild/relabel runtime artifacts.

The version-display candidate must test the actual approved string, font glyph
coverage, measured bounds, alignment/clipping, menu repaint, focus transitions,
fallback and normal shutdown through the ordinary verified launcher. No claim of
in-game consistency follows merely from changing JSON, a screenshot mockup, or a
successful string patch. If existing selectors rely on the old label/context,
coordinate explicit versioned changes rather than disabling the identity guards.

Read-only consistency checks (not a replacement for visual/runtime testing):

```powershell
python -B versioning/check_policy.py --manifest '<selected package manifest.json>' --current-record integration/current.json
python -B -m unittest discover -s versioning/tests -v
```

For a future selected package, add `--identity '<package version.json>'`. The
registry deliberately records the legacy release's absence of a separate number;
it does not modify its original 1.04 executable, generated engine or old manifest.

## Initial review, October 21, 2026

**Decision: hold existing release identity; do not assign or increment a number.**
The October18 resolution/launcher integration is an accepted technical foundation,
but no subsequent component has been promoted into Current. DWUI has an intake
review with integration/runtime/asset-lifecycle gates remaining. QFR and QUANTUM
knowledge/source work remain research infrastructure, not new player-facing
integrated milestones. The standing 2.00 goal alone does not change those facts.

Next review: when a significant new candidate is integrated and validated, or when
new evidence materially changes the assessment. Review does not require a bump.
Maintain the registry and append decision evidence; do not let the policy become
dependent on a chat remembering what a number meant.
