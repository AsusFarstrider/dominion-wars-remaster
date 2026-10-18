# Distribution inventory and exclusions

No public repository, remote, tag, release or upload is created by this pipeline.
The user separately approves public publication of an exact package. Release owns
that audit and publication, using Master's immutable recipe/revision.

- Modernizer foundation: unmodified committed0.6.0-alpha runtime at4f889dea...;
  source MIT, preserve its LICENSE.txt and all existing third-party notices.
- DDrawCompat0.7.1 diagnostic1/focus derivative: BSD-0-Clause, exact binary
  0E4790D2...16CB566. Full matching collected source and build provenance accompany
  the modified DLL. Collector build pins that binary; byte-identical recompilation
  is not asserted. MSVC/SDK/WDK header dependencies and local build paths are in
  the original build record/project and require environment configuration to rebuild.
- i6comp: exact existing bundled foundation tool and its original notices. No
  assumption that Modernizer's MIT license licenses it. Release must confirm the
  carried-through notices and proposed remaster rebundling before public packaging.
- Game executable transform: only COPY/literal delta against verified owner
  input, not a complete original/modified exe. The delta includes generated hooks
  and small changed instruction fragments. **Not automatically legally cleared.**
- Default package has no replacement game assets. Optional PRIVATE Saber delta
  contains owner-media-derived atlas/palette data and metadata, not a complete
  BOLT archive. Remains private; submission/ownership/hash is not redistribution
  clearance. Builder refuses a different optional asset payload fingerprint.
- No original game data, updater, disc images, complete game exe/archives,
  reference OBJ or portrait art is included. No no-CD/DRM bypass.
- Historical Jiri Dvorak wrapper and BOLT-family research are provenance, not
  incorporated runtime dependencies. New outer-directory BOLT delta mapping does
  not redistribute/import BOLTextract codecs. The separate existing asset-authoring
  pipeline remains GPL-3.0-or-later where documented; it is not silently relicensed.
- New integration scripts are project-authored; no blanket license is imposed on
  third-party research, owner artwork or game-derived content. Release/user must
  settle a top-level remaster source license before public source publication.

Private source intake/history includes local research records and absolute build
paths; it requires review/sanitization decisions before any public push. A checksum
is identity/integrity evidence, not signature/security review or legal permission.
Audit the actual Git history and exact selected package, not merely .gitignore.

Private artifacts are stored under E:/Codex/remaster-builds and optional art inputs
under E:/Codex/remaster-private-inputs. Shared-workspace source must not contain
full proprietary game outputs. Initial authoring drafts/portraits and personal
runtime screenshots/logs are excluded from the source history. Do not delete local
research just to make a publication audit clean.
