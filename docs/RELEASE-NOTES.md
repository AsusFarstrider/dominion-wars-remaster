# Remastered 1.10 Alpha — int-20261029.1

**Early experimental release. Native 2560×1440 primary display at origin (0,0)
only; one known Windows 11 / RX 6800 XT / three-monitor system.** Other primary
display modes are refused. No universal GPU/Windows, campaign or finished-remaster
claim. The original official game version remains 1.04.

> **Avoid Esc → Main Menu during a mission.** It opens the legacy Main menu in
> the top-left and mouse clicks stop working. Close and relaunch instead. This
> session-blocking path is explicitly deferred, not fixed or passed.

## What changed

- Native-1440 scene/UI composition and coordinated HUD assembly, retaining original
  HUD-density artwork and original fonts/control geometry.
- Restored opening/title cards, Main background, six Main button banks and three
  restored opening movies (Paramount, Gizmo/Mass Media v5, Simon & Schuster).
- Production launcher/version provider showing **Remastered 1.10**, with recording
  off and functional GPU hooks installed independently of recording.
- Original-audio reconstruction from verified local movies; no complete game EXE,
  original BOLT archive, original soundtrack, decoder or official updater supplied.

## Testing and limits

The exact packaged installer cloned verified original inputs into a new test
directory. Master records a short recording-off mission playtest and the owner's
explicit release approval with the menu bug deferred. The latest launcher returned
**1**, not a clean shutdown pass. Focus, media extraction and save/load evidence
are explicitly inherited from unchanged components/earlier tests, not fresh final
package regression passes. This is not independent per-movie, every-screen or
campaign acceptance. Button images cache about 231 MiB in the 32-bit process.
Multiplayer remains unsupported. See [known issues](KNOWN-ISSUES.md).

## Exact approved download

Download **dominion-wars-remastered-1.10-alpha-int-20261029.1.zip** (182,134,278
bytes) from [v1.10-alpha](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.10-alpha).

SHA256: `fdd3f3ee6bc619bcc841324ec7d3841fc6188379904301590293714de4a007ee`.

Original integration source: `f025d0cfaacaa1c28964792424ee7b77f37ace97`.
Manifest SHA256: `b0319fd0a248d8862e2641ddc8eb03218632fbfd4d63a2384ff1203827bed43d`.
The release does not rebuild or modify the tested ZIP. Its embedded pending/held
trial wording is historical; the separate **READ-ME-FIRST-1.10-ALPHA.md** and
[approval supplement](../release/int-20261029.1/RELEASE-RECORD.json) document later
approval of these exact bytes. Download the player ZIP, not GitHub's source archive.
See [installation](INSTALL.md), [testing](TESTING.md), [license scope](LICENSING.md)
and [publication audit](PUBLICATION-AUDIT.md).

Public source history has targeted privacy redactions and refreshed metadata
locks. Original-to-public revision mapping is recorded separately; a build from
redacted source is not claimed to reproduce the original ZIP's provenance bytes.

---

# First experimental alpha — int-20261018.1 (unchanged historical release)

Release basis: **int-20261018.1, original-assets**, source
`921826392c5b6f5acdc523f4db2429cbad234782`. The integration ID is the prerelease
tag; no unrelated alpha number is assigned. See [publication audit](PUBLICATION-AUDIT.md)
and [license scope](LICENSING.md). Public documentation is a later, non-runtime commit.

This first snapshot focuses on integrated resolution/UI, startup/input and focus
behavior while preserving original game assets. It is not an art-restored campaign
or a finished remaster. No private Saber artwork, new models or portraits included.

## Included

- Experimental 960×600 logical 16:10 UI and display-fit rendering, with reviewed
  selection/loading/menu/fleet adaptations.
- Integrated no-startup-resize launcher, relocated native input hook, bounded
  focus-return helper and guarded recovery-read fix.
- Hash-pinned DDrawCompat 0.7.1 resolution-diag1/focus wrapper and matching source.
- Installer using the pinned Modernizer 0.6.0-alpha foundation, from legitimate
  original media plus the full official updater or verified supported source data.
- Separate new-destination installs, verification, staged repair, return to the
  compatible original and explicit interrupted-transaction recovery.

## Evidence

Both variants were reported byte-reproducible; only the accepted original-assets
variant is the public release basis. Master records 12 Python tests, 15 PS5 core
checks, all packaged PowerShell parses, original-media installation, cloning and
baseline adoption. The private flavor additionally had 29 installer lifecycle
checks; those are not combined visual tests of private artwork.

Owner-observed combined first-mission/menu/input/focus/capture smoke and a separate
save/load run passed on one Windows 11/RX 6800 XT/three-monitor system. Both game
sessions exited0, managed files verified and desktop recovery completed. Normal
shortcut creation/target/hash and exact-owned removal were checked. Successful
loading is owner-observed; an output save corroborates saving, not loading by itself.
No long-campaign, other-hardware or combined render-target measurement is claimed.

See [known issues](KNOWN-ISSUES.md), [installation](INSTALL.md) and
[testing](TESTING.md). The advanced Repair -NoDesktopShortcut ownership edge,
32-cycle focus helper, primary-origin restriction and unsupported multiplayer
remain disclosed. Unsigned scripts/helpers may trigger security warnings.

## Exact player artifact

`dominion-wars-remaster-int-20261018.1.zip`, 4,101,191 bytes.

SHA256: `7633f657e8c5845d083c2d6cf6a871b8360155b7f8d7bc1733e839134392c864`.

The ZIP has been checked against its manifest/checksums and CRC. Its creation-time
approval/test fields remain historical; later acceptance/publication evidence is
separate. No ZIP bytes have been changed by release preparation. Do not distribute
the private Saber ZIP or automatic source download as this player package.
