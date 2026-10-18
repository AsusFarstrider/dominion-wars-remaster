# Experimental remaster installation prototype

This is an integration candidate, not an approved internal baseline or public
release. Keep the working original/Modernizer installation. NA retail1.04 only.

Extract the complete package to a normal writable directory. Run **Install Dominion
Wars Remaster.cmd**. If the usual Modernizer installation is found, choose whether
to clone it or use original media. Otherwise the pinned foundation looks for the
mounted retail disc. Provide the official full1.01-to1.04 ds9dw1to4.exe updater
when requested, and complete its normal wizard prompts. No disc letter is assumed.
The package contains no original game executable, original game archives, official
updater, or DRM/no-CD bypass. Keep legitimate original media available for play.

The default new destination is %LOCALAPPDATA%\Dominion Wars Remaster. The original
installation is not changed. Source data are hash-verified; a clean base is staged
without copying old Modernizer shortcut/state files. Save files (*.sav outside
internal .dw-* directories) are copied, not moved. Other custom mods/user file
formats are not silently imported. Source installations must not contain junctions
or other reparse entries. Input versions/modified game data outside the supported
hashes are refused, not patched blindly.

Launch using the build-labelled Dominion Wars Remaster desktop shortcut or **Play
Dominion Wars Remaster.cmd** inside the new installation. This candidate combines
the latest selected 960x600 logical16:10 UI/rendering work with no-startup-resize
input/focus handling. On the known2560x1440 desktop the resolution branch measured
2304x1440 3D drawing. This does not establish true16:9, rebuilt high-resolution UI
art, or broad monitor/GPU support. Pause manually before switching away; automatic
pause is not supported. Screenshot tools are not universally validated.

## Advanced reproducible calls

No Git or Python is needed by players. Windows PowerShell5.1 is sufficient:

```powershell
.\Remaster.ps1 -SourcePath 'D:\Games\Dominion Wars' -Destination 'D:\Games\Dominion Wars Remaster'
.\Remaster.ps1 -MediaPath 'G:\' -PatchPath 'D:\Downloads\ds9dw1to4.exe' -Destination 'D:\Games\Dominion Wars Remaster'
.\Remaster.ps1 -Action Verify -Destination 'D:\Games\Dominion Wars Remaster'
.\Remaster.ps1 -Action Repair -Destination 'D:\Games\Dominion Wars Remaster'
.\Remaster.ps1 -Action Remove -Destination 'D:\Games\Dominion Wars Remaster'
```

Media drive letters above are examples, not requirements. Use -NoDesktopShortcut
to omit a shortcut. Backend Auto probes the default adapter's D3D12 capability;
Native or D3D9On12 may be selected explicitly at installation/repair. The installer
records the exact chosen profile. Other backend/hardware combinations still need
combined runtime tests; the successful Modernizer tests are not remaster tests.

Repair rebuilds in separate staging from verified original backups and unchanged
game data, retains saves, then preserves the previous directory before committing
the replacement. Unknown/damaged immutable originals require a clean source/media
installation, not guessed repairs. Updating to another package uses Install with
the old remaster as SourcePath and a NEW destination; original backups supply
canonical input bytes. Existing experimental directories are never updated in place
merely because a new build exists.

Remove performs the same guarded rebuild, but leaves a compatible original game
without the remaster changes. Play it with the foundation's Play Dominion Wars.cmd.
The former remaster and saves remain in a named sibling backup; an owned, unchanged
remaster shortcut is moved into that backup. Modified shortcuts are preserved.
No existing normal-game shortcut is removed or replaced. Do not run the standalone
Modernizer Repair/Uninstall against remaster files; use this package's operations.

Failed staging and logs are retained, never reported as success. No large temporary
source copies/backups are automatically deleted in this prototype, so allow several
GB of extra space. Each operation prints its staging, backup and transaction JSON
locations. If interruption leaves the destination absent after preserving its old
version, use Remaster.ps1 -Action Recover -Destination '<original destination>'
-RecoveryPath '<printed sibling transaction JSON>'. Recovery restores the verified
previous remaster; it does not silently promote an unfinished candidate.

Installed verification: Verify Dominion Wars Remaster.cmd. Session/focus logs are
.dw-remaster\runs; underlying startup/recovery diagnostics are .dw-launcher.
Installer foundation logs live in the printed retained staging directory. Include
build ID, Windows/GPU/driver, primary monitor/layout/scaling, exact failure stage,
screenshots and relevant logs when reporting problems. Review logs for personal
paths before sharing. Unsigned scripts/legacy helper software may trigger warnings;
do not disable Windows security to install this prototype.

An optional PRIVATE Saber-art package additionally contains an owner-media-derived
texture delta. Its public redistribution is not cleared. Default packages omit it;
unfinished geometry/portrait assets are excluded from both variants.
