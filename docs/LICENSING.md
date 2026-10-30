# License scope and exceptions

The project owner approved MIT for newly authored integration, build, installation
and release code on October 18, 2026. [LICENSE.txt](../LICENSE.txt) applies only to
material the project owns, not the original game, Star Trek, proprietary assets or
third-party work. This inventory is not blanket legal clearance for every possible
use of reverse-engineering material.

## Project-owned code

New code under integration/tools, integration/installer, integration/runtime,
integration/tests, integration entry points, release-tools and new documentation
is MIT. Newly project-authored launcher/resolution patch logic in the frozen intake
is also MIT only to the extent it is original work. Frozen copies do not change
ownership. Original-game fragments embedded within those files are excepted below.

## Third-party components

- integration/components/intake-20261018.1/modernizer retains its upstream MIT
  license and nested notices. Its inclusion does not relicense its dependencies.
- integration/components/intake-20261018.1/wrapper-source and its pinned binary
  retain DDrawCompat's bundled license, notices and source/build provenance.
- Stock/focus wrappers and i6comp in Modernizer retain their corresponding notices.
  i6comp0.20 credits fOSSiL/Morlac; its preserved source-header notice authorizes use
  with proper credit. Exact hashes are recorded in the foundation/intake.
- pefile, Python and MSVC/SDK/WDK are development dependencies with their own
  licenses. No developer runtime or WDK binary is added to this player package.
- Separate BOLTextract-derived asset-authoring tools are not imported into this
  integration pipeline. Their documented GPL obligations remain separate.

## Game-derived byte fragments: not an MIT grant

Identify these separately from newly authored patch logic:

- integration/components/intake-20261018.1/resolution.patch.json: version-pinned
  COPY/literal transform with changed instruction/data bytes.
- resolution-history/*patch*.json and builder/test files in that directory:
  original machine-code signatures, addresses and small verification fragments.
- launcher/build_input_window_v1.py and launcher/input-window-patch-v2.json:
  original instruction signatures alongside newly authored hooks.
- Generated player patches/engine.json: an executable transform, not a complete
  original executable or an automatic grant of redistribution rights.
- Native intake build_engine.py, qfr-source.json, tests and wrapper modifications
  also contain version-specific addresses/signatures and original-byte evidence.
  Those original fragments are excepted from the project's MIT grant. Changes to
  DDrawCompat remain under its BSD-0-Clause license, not blanket MIT.

MIT does not purport to license original-game fragments because they appear in
our tools/deltas. Rights remain with their owners where applicable. No full game
executable or substantial original source-code listing was identified in the
selected tree; this observation is not a legal guarantee. Future substantial
third-party/game-derived contributions with unclear rights need separate review.

## Restored visuals in 1.10 Alpha

The project owner attested that new/restored visuals were created by the owner or
with ChatGPT and authorized their distribution in this project. The release
contains three converted opening/Main cards, six Main button banks and replacement
video payloads for Paramount, Gizmo/Mass Media and Simon & Schuster. This records
project-distribution permission, not blanket MIT/CC licensing of the art or a claim
to own original logos, Star Trek trademarks or other underlying third-party rights.

Movie deltas copy the original 12-byte audio metadata and every compressed audio
packet from the player's hash-verified files. Independent package inspection
verified 225, 601 and 281 audio packets respectively, with no literal audio payload.
Newly encoded video is supplied in the deltas; a delta is not itself legal clearance.
The original binkw32.dll stays owner-supplied. RAD/Bink encoder, SDK, player and
development-tool binaries are not included, and their terms are not replaced by MIT.

## Exclusions

Original installations, full executables/BOLT archives, media images, official
updater, private Saber art, reference models, portraits, saves and private evidence
are not supplied or licensed here. Possession/submission is not redistribution
clearance. No no-CD or DRM bypass is included; supply legitimate supported data.

Frozen documents may say licensing/public approval is pending. They retain their
creation-time state; this scoped decision and the publication record supplement
them without changing the tested package or frozen input hashes.
