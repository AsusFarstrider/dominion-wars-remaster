# Public source and the approved 1.15 ZIP

Original tested runtime revision:
`8a32bcc36117442fffda89cd42580382f3305216`.
Public mapped counterpart:
`aa8e82ebd511f19aa94e7a3fd7d5ccb48d49aa75`.
Master's final metadata maps from `ba88c517c08578e6060e43f6803ebba3621d7db3`
to `89495847bfde5af2dd9e90043c3fe83ef16a6c56`.

The continuation retains prior public `f7bc25b` ancestry and all released tags.
A complete private original-history bundle was verified first. Personal profile
prefixes and unique monitor IDs in proof were masked; public metadata hash
relationships were refreshed. Production wrapper/launcher/engine source and all
approved player payloads remain unchanged. See
[mapping](../release/int-20261030.1/SOURCE-MAP.json).

Evidence collection helpers contain workspace-specific paths. Their public prefix
substitutions preserve logic/suffixes; they are not a portable player prerequisite.
Snapshot collector constants refer to original private collection-time proof.
Do not weaken their checks to make redacted evidence pretend to be identical.
Use the committed frozen intake for the supported package-build workflow.

## Developer build

Players need none of this; use [installation](INSTALL.md). Developers should use
a **short checkout path** and Git long-path support because nested provenance
filenames can exceed Windows' default limit:

```powershell
git -c core.longpaths=true clone https://github.com/AsusFarstrider/dominion-wars-remaster.git D:/src/dw-remaster
```

See [build prerequisites](../integration/docs/NATIVE-BUILD.md). Verify and extract
the exact approved player ZIP. Use its unchanged original source lock to recover
private asset inputs, then the public intake to build:

```powershell
New-Item -ItemType Directory -Path 'D:/Private/new-1.15-original-lock' -ErrorAction Stop
Copy-Item -LiteralPath 'D:/Extracted-1.15/third_party/native-component-source-lock.json' -Destination 'D:/Private/new-1.15-original-lock/source-lock.json'
python -B integration/tools/materialize_native_inputs.py --intake 'D:/Private/new-1.15-original-lock' --package 'D:/Extracted-1.15' --original-game 'D:/Owned/Verified-1.04' --output 'D:/Private/new-1.15-inputs'
python -B integration/tools/build_native_remaster.py --intake 'integration/components/intake-adaptive-20261030.1' --decision 'versioning/decisions/version-review-20261030-1.15.json' --original-game 'D:/Owned/Verified-1.04' --asset-inputs 'D:/Private/new-1.15-inputs' --output 'D:/Private/new-1.15-build'
```

Paths are examples; output directories must be new and private, outside source.
The original-lock helper intentionally refuses a mismatched public lock. Do not
relax that check: package/private and redacted/public provenance are distinct.
Use a clean committed checkout. Original files are read, not edited.

An independent build from public metadata revision `8949584` passed and matched
the approved game patch, media/art, wrapper, installer and launcher. Its identity,
rendering-config version-hash binding, public lock, manifest and checksums differ.
The exact approved ZIP retains original source/lock identity; do not claim a new
build at redacted HEAD reproduces those provenance bytes. Byte-identical wrapper
recompilation is not claimed; the reviewed binary and matching source are pinned.
Never upload a private verification build in place of the approved download.

---

## Historical 1.10 source record
# Public source and the approved 1.10 ZIP

The player ZIP is the exact, tested original artifact from source
`f025d0cfaacaa1c28964792424ee7b77f37ace97`. It was not rebuilt for publication.
The public counterpart is `4d3ce9ea2dcb97b73672cd9b8303633def904c46`.
Master's later approval metadata is mapped from `4b59a98` to `0065e0d`.
See [full mapping](../release/int-20261029.1/SOURCE-MAP.json).

Five newly committed provenance/log files exposed a personal Windows profile path.
Only that prefix was replaced with an example profile; technical suffixes, findings
and original artifact identities remain. Two public-only source locks were rehashed
to match their redacted metadata. Existing published ancestry/tags and all game
patches, wrapper code/binaries and restored payload hashes are unchanged. Master
and its active worktrees retain original history; a complete private bundle was
verified before the isolated public mapping. No force rewrite of public history
was needed. Private application refs/backups are never pushed.

Registry/package references to original commits, source manifests and locks are
**original tested provenance**, not assertions that redacted metadata has those
hashes. The original lock is retained inside the approved package. This distinction
avoids silently substituting a new runtime or changing the release checksum.

## Developer reproduction

Players need none of this: use [installation](INSTALL.md).

Use the prerequisites in [native build documentation](../integration/docs/NATIVE-BUILD.md).
The public checkout has internally consistent refreshed file locks. The original
materialize_native_inputs.py deliberately requires an identical package/intake
lock and will refuse the original ZIP against a redacted lock. Use the unchanged
lock shipped inside the approved package for input reconstruction, then select
the public intake for the build. After extracting the verified approved ZIP into
`D:/Extracted-1.10`, create a NEW private metadata folder outside source:

```powershell
New-Item -ItemType Directory -Path 'D:/Private/new-1.10-original-lock' -ErrorAction Stop
Copy-Item -LiteralPath 'D:/Extracted-1.10/third_party/native-component-source-lock.json' -Destination 'D:/Private/new-1.10-original-lock/source-lock.json'
python -B integration/tools/materialize_native_inputs.py --intake 'D:/Private/new-1.10-original-lock' --package 'D:/Extracted-1.10' --original-game 'D:/Owned/Verified-1.04' --output 'D:/Private/new-1.10-inputs'
python -B integration/tools/build_native_remaster.py --intake 'integration/components/intake-native-20261029.1' --decision 'versioning/decisions/version-review-20261029-1.10.json' --original-game 'D:/Owned/Verified-1.04' --asset-inputs 'D:/Private/new-1.10-inputs' --output 'D:/Private/new-1.10-build'
```

Paths are examples; outputs must be new private directories outside source. The
helper checks the original package/intake binding and every recovered asset hash.
Verify the ZIP checksum before extracting. It never edits original game files.
Use a clean committed checkout. A new build
embeds its own public Git HEAD and refreshed metadata; its version.json, lock,
manifest, version-hash binding in Remaster.Rendering.json, checksums and ZIP
identity necessarily differ from the approved artifact.
Do not present that reproduction as the tested release or upload it in its place.
Byte-identical wrapper recompilation is not claimed; the reviewed binary is pinned
and matching source/build records are supplied. A new release requires its own
Master handoff and approval.

Independent verification built mapped source `0065e0d` successfully: the game
patch, all restored media/art, wrapper, installer and launcher matched the approved
payloads exactly. Only the identity, its rendering-config hash binding, public
source lock, manifest and checksums differed. This was a private build check, not
a new playtest or a replacement public package.
