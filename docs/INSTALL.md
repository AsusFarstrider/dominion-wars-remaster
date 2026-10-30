# Installing an experimental remaster build

These steps describe **Remastered 1.10 Alpha / int-20261029.1**,
not a source checkout. Get the named ZIP from the
[experimental release](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.10-alpha).
Read the separate **READ-ME-FIRST-1.10-ALPHA.md** release attachment. The unchanged
ZIP calls itself an internal trial; the external acceptance record approves those
exact bytes with the known limitations below, rather than silently repackaging it.

## Before starting

- Back up saves and keep your working original/Modernizer installation.
- Set your primary monitor to physical **2560×1440**, at desktop origin **(0,0)**.
  This native pilot refuses other primary-display modes and does not resize your
  desktop. It is not a 1080p or 4K release. Other monitor layouts/DPI are unproven.
- Supply legitimate North American retail media, mounted as a Windows CD/DVD drive,
  and the full North American `ds9dw1to4.exe` updater; or use a supported verified
  original/Modernizer installation. Raw image files are not mounted by the installer.
  The historical [TrekCore downloads page](https://gaming.trekcore.com/dominionwars/downloads.html)
  lists **Official Patch (v1.01 to v1.04)**. Incremental and other-region updates
  are not substitutes. The official updater is not included in our download.
- Use a writable folder and allow several GB of spare space for retained staging
  and backups. No Python, Git or SDK is needed by players.
- Use ordinary local game/source/destination directories. Cloud placeholders,
  junctions and other reparse-point paths are refused; a OneDrive-synced game
  directory may be rejected. Prefer the default local application-data destination.
- Testing is currently limited to one Windows 11/AMD system. Other configurations
  are welcome for future testing, not guaranteed to work.

## Install and play

1. Download **dominion-wars-remastered-1.10-alpha-int-20261029.1.zip** and extract it fully.
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

The package installs restored opening/Main art and reconstructs three restored
movies using original audio packets from your verified files. It does not require
separate private artwork, an encoder or a developer toolchain. Recording is off.

**In a mission, avoid Esc → Main Menu.** That path opens an unresponsive old menu
in the top-left corner. Close and relaunch instead; save beforehand where possible.
If the broken screen is already open, try Alt+F4 or end only the game's ds9dw.exe
in Task Manager. Unsaved progress may be lost. Do not mix old/new EXEs or remove
the HD safety configuration while keeping HD movies installed.

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
The bounded renderer safety ledger is in
`%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`; it is not full gameplay recording.
