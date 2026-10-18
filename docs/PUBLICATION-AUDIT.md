# Initial public-source audit

Baseline source:921826392c5b6f5acdc523f4db2429cbad234782, one commit,557 files.
Publication documentation/license commits retain it as an ancestor. No competing
history or history rewrite is used.

## Paths and privacy

The initial scan identified26 files with absolute environment references; handling
JSON-escaped paths added two metadata files, for28 reviewed files. They contain
historical E:/Codex fixture/build/provenance locations and standard C:/Program Files
software paths. No personal profile path, private contact detail or credential
pattern match was found in the selected source revision. Ordinary examples such
as C:/Games and path-validation examples remain useful and are retained.

Master confirmed the nonpersonal path scope and no concurrent history changes.
Frozen paths are retained as technical provenance, not silently rewritten: no
genuine privacy-sensitive path was identified. Historical standalone research
scripts are not all portable. Use the supported integration build recipe and
explicit owned-original/new-output paths with pinned dependencies.

A complete private Git --all bundle was created and verified before publication,
including private application refs. Only reviewed master history is pushed; the
bundle and private Codex refs are not published. No original/sanitized commit
mapping is needed because the approved commit is unchanged.

## Contents and licensing

No full game installation/executable/archive/media, updater, private art/model,
save or personal runtime evidence was found in the selected history or ZIP.
Pattern scanning is an aid, not proof of absence. Four expected tracked native
binaries are pinned DDrawCompat variants and i6comp, with corresponding notices;
they are not original game executables. See [license scope](LICENSING.md), including
the separate treatment of original instruction signatures and delta bytes.

Player ZIP:489 members, all488 checksum entries/487 manifest files and CRC checked;
asset=null, private Saber excluded. Exact package and manifest hashes match Master.
Later acceptance/license/publication records supplement frozen pending fields.

## Reproduction

The release verification build from detached approved commit9218263 reproduced
the original ZIP byte-for-byte:7633f657e8c5845d083c2d6cf6a871b8360155b7f8d7bc1733e839134392c864,
4,101,191bytes. The package manifest reproduced as
ee9c6cff67be3ccf7c607cddb41fccb398aaff20ee945c4ab359a896640adcb4.
No runtime or package bytes changed.

Rebuild from that source commit, not the later documentation HEAD: the builder
embeds Git HEAD and therefore a documentation-only commit changes a new ZIP's
provenance. The private detached verification worktree is not a new baseline.
Private audit/backup/reproduction records stay outside public history.

## Publication receipt

The original-assets experimental release was published October18,2026. The public
tag int-20261018.1 points to documentation/license commit9b7e9d8, whose integration
tree is byte-identical to approved source9218263. Twelve Python and fifteen PS5
core checks were rerun successfully. A cloud/reparse fixture location was refused
on the first core-test attempt; ordinary local fixtures passed without code edits.
GitHub's uploaded ZIP digest and a downloaded round-trip both match the approved
7633f657... SHA256. See the [machine-readable receipt](../release/int-20261018.1/PUBLICATION.json).
