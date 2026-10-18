# Installation/build architecture

Selected: **B user-facing unified remaster installer, implemented by composing A
internally**. The pinned, unmodified Modernizer installs a clean original1.04
compatibility foundation in staging; the remaster applies its own identified
engine delta/runtime/profile and verifies the whole result before commit.

Why this fits current code: Modernizer0.6 already exposes Install with SourcePath
or MediaPath/PatchPath, no-shortcut mode, progress/logging, exact media/exe hashes,
interactive official updater handling, and cabinet extraction. A second PUBLIC
installation step would make players coordinate two products unnecessarily.
Extracting a new shared installer library(C) would require a cross-project refactor
before it provided value; it is deferred. No Modernizer source changes or remaster
dependency are introduced. Its compatible-original workflow remains usable alone.

## Developer versus player

Developer build reads frozen, versioned source/intake, exact original-file inputs,
optional private art delta and pefile2024.8.26. It composes executable changes,
verifies scope, generates COPY/literal deltas, collects exact wrapper/foundation
files, generates a manifest and checksums, and creates a deterministic ZIP. A
development/uncommitted package is explicitly non-promotable; normal builds require
the source revision to be committed and clean. Another environment needs the same
identified source/dependencies and legitimate originals, not live Codex chats.

Players run one .cmd entry point under ordinary Windows PowerShell5.1. No Git,
Python, debugger, compiler, Codex, or manual executable patching. The package uses
verified delta COPY/literal operations and post-output hashes, without distributing
complete commercial executables/archives. A private optional asset component uses
the same patch interface; future approved portraits/models/art can be supplied as
versioned components without changing media extraction or launcher ownership.

## Safety/transactions

Existing original/Modernizer installs are read-only sources. Only canonical
hash-verified game data and save files are copied into a CLEAN source; inherited
Modernizer state/shortcut references are not imported. Media path uses its exact
cabinet hashes and the owner-supplied exact official updater. Unsupported editions
and altered immutable originals are refused.

Fresh destination must not exist or nest inside/outside its source; reparse paths
are refused. Remaster files are applied only in a unique sibling stage. Complete
original base backups and an installed manifest support repair/update/removal.
Repair/removal retain the previous complete directory before switching in the
verified replacement. Commit failure attempts rollback; a retained transaction
journal supports explicit recovery after interruption. No broad recursive deletion
or cleanup of user data is used. Extra staging disk usage is documented.

New shortcut names do not overwrite the compatible original's shortcut. Removal
restores a compatible original in the remaster directory and retains the former
remaster/owned shortcut in a sibling backup. Unknown shortcut edits remain intact.
Updates install into a NEW destination from canonical local backups and saves.

## Deliberate limitations

One tested regional input family; fixed logical960x60016:10, not arbitrary16:9 UI;
native input patch assumes primary origin(0,0); no automatic pause; guarded focus
recovery has the branch's32-cycle bound; screenshot hosts are heuristic exceptions.
Native versus D3D9On12 backend choice is capability-driven, not GPU-vendor guessing,
but remaster validation on those alternate systems remains pending.

No silent background downloads, system-wide DLLs, registry compatibility flags,
DRM changes, new display modes or forced CPU-affinity settings. Wrapper may itself
manage display presentation as before; 'no startup resize' means the private
launcher does not apply a low physical mode, not that all Windows/driver display
events are impossible.
