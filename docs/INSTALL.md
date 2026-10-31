# Installing 1.15 Alpha

Get **dominion-wars-remastered-1.15-alpha-int-20261030.1.zip** from
[the 1.15 Alpha release](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.15-alpha).
GitHub's source ZIP is not an installer. Read the separate
**READ-ME-FIRST-1.15-ALPHA.md** release notice: later exact-package approval
supplements the ZIP's historical candidate/pending metadata without changing it.

## Before starting

- Keep a working original/Modernizer game and back up saves.
- Supply legitimate North American retail media mounted as a Windows drive plus
  the full official `ds9dw1to4.exe` updater, or a supported verified original,
  Modernizer or remaster installation. The updater is not included. Raw disc
  images are not mounted by this installer; drive letters are not fixed.
  [TrekCore's download archive](https://gaming.trekcore.com/dominionwars/downloads.html)
  lists the full official v1.01-to-v1.04 patch; incremental/other-region patches
  are not substitutes.
- Use ordinary writable local folders. Cloud placeholders, junctions and reparse
  paths may be refused. Allow several GB for game copies, staging and backups.
- Make the intended display primary at desktop origin (0,0) before launching.
  Actual 1080p and 1440p component tests cover one Windows 11 / RX 6800 XT computer
  at 96 DPI. Final packaged manual acceptance was 1440p; other modes/hardware/DPI
  remain unverified. The launcher does not change the desktop resolution.

## Install and launch

1. Extract the entire player ZIP into a normal writable folder.
2. Close other Dominion Wars sessions. Double-click **Install Dominion Wars Remaster.cmd**.
3. Select a supported existing installation or original media. Supply the official
   updater if prompted and complete its normal wizard.
4. Choose a **NEW destination**; the default is
   `%LOCALAPPDATA%\Dominion Wars Remaster`. If that already contains an experiment,
   choose another folder rather than overwriting it.
5. After successful verification, use the new build-labelled desktop shortcut or
   **Play Dominion Wars Remaster.cmd** inside the installed game.

No PowerShell knowledge, Git, Python or encoder is needed to install and play.
Original inputs are verified and copied, not modified. Recognized `*.sav` files
outside internal `.dw-*` folders are copied, not moved. Other mods and custom
formats are not silently imported. Unsupported inputs/modified immutable originals
are refused. Keep legitimate media available where the original game requires it.

Nine authorized replacement visual payloads and three movie video deltas are
included. Original compressed audio is reconstructed from verified local movies.
The embedded Read Me's phrase "No game assets" means no complete original retail
game assets; it must not be read as saying replacement visuals are absent.

A single size is selected at startup from the primary physical display. Do not
resize, change display modes or hotplug monitors while playing. Pause before
Alt-Tab. Do not mix this adaptive engine/wrapper/launcher/profile with 1.10's
fixed-1440p files, or remove HD safety configuration while retaining HD movies.
Multiplayer remains unsupported.

## Verify, repair or undo

Use **Verify Dominion Wars Remaster.cmd** inside the installed game. Keep the
extracted package. For Repair/Remove, open Windows PowerShell in its folder:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Repair -Destination 'D:\Games\Dominion Wars Remaster 1.15'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Remove -Destination 'D:\Games\Dominion Wars Remaster 1.15'
```

Replace the example destination with your actual installed folder. Repair stages a
verified replacement and retains the previous installation/saves. Remove rebuilds
a compatible original in that separate destination; use **Play Dominion Wars.cmd**
afterward. The previous remaster/saves remain in a sibling backup. This is not a
recursive deletion and does not delete your original game. Managed shortcut removal
is ownership-checked; modified shortcuts are preserved. Use ordinary Repair;
the earlier advanced `Repair -NoDesktopShortcut` ownership edge is not independently
cleared by this release.

Update by installing from a supported source into a NEW destination. Backups and
failed staging are retained, not automatically deleted. Use this package's lifecycle
operations, not standalone Modernizer repair/removal on remaster files.
See the [advanced recovery procedure](../integration/docs/INSTALL.md) if an
interrupted operation needs its printed transaction journal.

## Troubleshooting

Session logs are in the installed `.dw-remaster\runs`; startup/recovery diagnostics
in `.dw-launcher`; the bounded renderer ledger in
`%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`. Installer/staging/transaction
paths are printed by the operation. Retain relevant errors; review personal paths
and monitor identifiers before sharing logs.

Unsigned scripts/legacy helpers may attract antivirus warnings. Do not disable
Windows security as an installation step. Report version/build, Windows,
GPU/driver, monitor layout/physical mode/DPI, disc edition and exactly where the
failure occurred. See [testing](TESTING.md) and [known issues](KNOWN-ISSUES.md).
