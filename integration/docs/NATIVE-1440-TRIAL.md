# Dominion Wars Remastered 1.10 Alpha — internal combined trial

This is an unapproved test candidate, not Current or a public release. Keep your
working game installation. Tested hardware coverage is very limited; no universal
Windows/GPU compatibility is claimed.

This native profile requires the primary display at physical **2560x1440**, desktop
origin (0,0). It refuses other modes before launch and does not change your desktop
resolution. This is not a native1080p/4K release. The earlier compatible-original
installation remains a separate, unchanged fallback.

## Install into a separate folder

1. Extract the entire package ZIP to an ordinary writable folder.
2. Double-click **Install Dominion Wars Remaster.cmd**.
3. Choose your existing original/Modernizer game or supply original North American
   media and the official1.04 patch when prompted. The official patch is not bundled.
4. Use a NEW destination. Do not point it at your working installation.
5. Wait for verification/success, then use the new desktop shortcut or
   **Play Dominion Wars Remaster.cmd** in the installed folder.

Original game inputs are verified. The installer reuses the pinned Modernizer
foundation, applies identified executable changes, reconstructs restored movies
using original soundtrack packets from your owned files, and adds restored
cards/background/buttons. It ships no complete game EXE, original BOLT archive,
original decoder or original soundtrack. The game is still based on official1.04;
**Remastered1.10** is a separate experimental milestone label, not an official patch.

Restored visuals are supplied with the creator's permission for this project.
Original game, logos, characters and trademarks remain the respective owners'
property. DDrawCompat is credited under its included BSD-0-Clause license; the
unchanged Modernizer foundation retains its bundled credits/licenses.

## Combined trial

Confirm Simon & Schuster and the other openings have sound, correct endings and
working skips; distinguish natural playback from skipping. At Main check all six
v7 button idle/hover/activation states and **Remastered1.10** with no missing,
overlapping or clipped text. Check Preferences/Load and returning to Main. Then
test mission loading/HUD/radar/selection/control, save/load, paused Alt-Tab/Win+D
return, screenshot capture and normal quit. Report anything skipped or failed.

The original font/layout/control geometry remain. The new RGBA button banks cache
about231MiB in this32-bit game; report slowdowns, missing artwork or memory failures.
Recording is OFF. A small bounded rendering safety ledger is separate from full
recording; no diagnostic capture is required to play.

## Verify, repair, remove

Keep the extracted package: its Remaster.ps1 supports Verify, Repair, Remove and
Recover through Windows PowerShell5.1. For example, from its folder:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Verify -Destination 'C:\Your Separate Remaster Folder'
powershell -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Repair -Destination 'C:\Your Separate Remaster Folder'
powershell -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Remove -Destination 'C:\Your Separate Remaster Folder'
```

Replace the example destination with your real folder. Repair rebuilds through a
verified staging directory; previous folders/logs are retained. Remove restores a
compatible original in that separate installation and preserves saves. It does not
delete your original/Modernizer installation. Do not mix old/new EXEs or manually
remove HD safety configuration while leaving HD movies installed.

Launcher results are under the installed **.dw-remaster/runs**; lower-level logs
under **.dw-launcher**. The bounded renderer ledger is under
**%LOCALAPPDATA%/Dominion Wars Remastered/Rendering**. Install transactions/staging
logs are retained next to the destination. Report buildID, version, Windows build,
GPU/driver, monitor layout/physical mode, disc edition, exact failure point and logs.
Unsigned scripts/helper tools can attract antivirus warnings; inspect files before
allowing them. Multiplayer is untested and unsupported.

Release/promotion remains held until combined acceptance and lifecycle validation.
