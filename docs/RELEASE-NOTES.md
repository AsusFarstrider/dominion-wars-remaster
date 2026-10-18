# First experimental alpha — int-20261018.1

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
