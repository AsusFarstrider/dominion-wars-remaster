# Installing an experimental remaster build

These steps describe the approved **int-20261018.1 original-assets** player package,
not a source checkout. Get the named ZIP from the
[experimental release](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/int-20261018.1).

## Before starting

- Back up saves and keep your working original/Modernizer installation.
- Supply legitimate North American retail media, mounted as a Windows CD/DVD drive,
  and the full North American `ds9dw1to4.exe` updater; or use a supported verified
  original/Modernizer installation. Raw image files are not mounted by the installer.
- Use a writable folder and allow several GB of spare space for retained staging
  and backups. No Python, Git or SDK is needed by players.
- Use ordinary local game/source/destination directories. Cloud placeholders,
  junctions and other reparse-point paths are refused; a OneDrive-synced game
  directory may be rejected. Prefer the default local application-data destination.
- Testing is currently limited to one Windows 11/AMD system. Other configurations
  are welcome for future testing, not guaranteed to work.

## Install and play

1. Download **dominion-wars-remaster-int-20261018.1.zip** and extract it fully.
   Do not run inside the ZIP or download GitHub's source archive as an installer.
2. Close other Dominion Wars sessions. Run **Install Dominion Wars Remaster.cmd**.
3. If your usual Modernizer installation is detected, choose whether to clone it or
   use original media. With media, supply the official updater when requested and
   complete its normal wizard. Drive letters are not fixed.
4. Use a NEW destination. The default is `%LOCALAPPDATA%\Dominion Wars Remaster`.
   Source files are verified and copied; the original installation is not modified.
5. Wait for successful completion, then use the build-labelled **Dominion Wars
   Remaster** desktop shortcut or **Play Dominion Wars Remaster.cmd** in the new game.

Recognized `*.sav` files outside internal `.dw-*` folders are copied, not moved.
Other mods and custom-file formats are not automatically imported. Unsupported
versions, modified immutable input files and reparse-point sources are refused.
Keep legitimate media available where the original game requires it.

## Verify, repair or return to the compatible original

Run **Verify Dominion Wars Remaster.cmd** inside the installed game to verify it.
Advanced package commands, run from the extracted package in Windows PowerShell:

```powershell
.\Remaster.ps1 -Action Repair -Destination 'D:\Games\Dominion Wars Remaster'
.\Remaster.ps1 -Action Remove -Destination 'D:\Games\Dominion Wars Remaster'
```

The example path is not required. Repair stages a verified replacement and retains
the previous installation/saves. Remove rebuilds a compatible original without the
remaster layer; launch that with **Play Dominion Wars.cmd**. It is not a recursive
deletion of the game. The previous remaster and saves remain in a sibling backup.
Only an owned, unchanged remaster shortcut is moved into that backup; modified
shortcuts are preserved. Do not use standalone Modernizer repair/removal on remaster
files. Avoid the advanced `Repair -NoDesktopShortcut` combination in this release.

Update by installing from the supported remaster source into a NEW destination,
not by overwriting an existing experiment. Staging/backups are not automatically
deleted. Failed/interrupted operations retain diagnostics and may require explicit
journal recovery; see the [exact advanced procedure](../integration/docs/INSTALL.md).

Session/focus logs: `.dw-remaster\runs` in the installed game. Startup/recovery
diagnostics: `.dw-launcher`. Installer foundation logs are in the retained staging
path printed by the operation. Review personal paths before sharing logs.
