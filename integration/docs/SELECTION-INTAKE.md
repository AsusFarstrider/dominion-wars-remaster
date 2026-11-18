# Private mission-selection intake

This intake extends approved 1.15 with the reviewed selection/cursor work,
paused-briefing/confirmation repair and reference-free DWSSEL2 conversion.
It is not Current, a version milestone, a tested player package or a release.

From the repository root, using the verified handoff and previous intake:

```powershell
python -B integration/tools/snapshot_selection_component.py `
  --handoff handoffs/master/rendering-selection-reference-free-20261102-01 `
  --previous-intake integration/components/intake-adaptive-20261030.1 `
  --output NEW_INTAKE_DIRECTORY
python -B -m unittest discover -s integration/tests -p 'test_*.py'
```

The immutable recipe pins specific reviewed input hashes. Another component or
sanitized public-source mapping requires its own explicit review/recipe update;
do not loosen hash checks to import a newer experiment. Output must not exist.

The collected `source-lock.json` records the complete production source, wrapper,
inherited movie/art inputs and five additional remaster data files. New data lives
inside the intake's `runtime-assets`, separately from owner-local movie inputs.
`build_native_remaster.py` consumes these optional `runtime_additions` through its
normal runtime inventory. It refuses paths outside `rs/Remaster`, traversal,
duplicates, existing-target replacement and missing/inconsistent lock entries.
The player still needs no Python/compiler/Git or manual catalog conversion.

Do not use an old DWSSEL1 catalog with the new wrapper. DWSSEL2 has exact original
resource fingerprints/metrics and replacement artwork, not original identity
rasters. The legacy authoring catalog is private conversion input only. Conversion
and validation tools and exact format are retained under `selection-handoff`.

To produce a player candidate later, commit/freeze the reviewed integration and
version decision, then use the existing native builder with this intake, verified
owner game data, and the inherited authorized external media inputs. No new
player build/version decision is selected by creating an intake. Package lifecycle
verification and a suitable combined acceptance are still needed before promotion.

Review: `handoffs/master/rendering-selection-reference-free-20261102-01-MASTER-REVIEW.md`.
Frozen intake: `integration/components/intake-selection-20261102.1`.
Lock SHA256: `6232db997f18194f040740eea10da0c01b18d8926b0460d1e9554436ca872ac2`.
