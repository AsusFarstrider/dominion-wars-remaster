# Testing on another computer

This is an early alpha, not a promise of compatibility. Windows 11 with an AMD
processor / RX 6800 XT has the longer gameplay record. Windows 10 with an Intel
i5-3570K / NVIDIA GTX 670 has original-media installation and brief first-mission
evidence, plus a separate installer click-guard pass. Other GPUs, Intel integrated
graphics, hybrid laptops and longer sessions still need testing. These are not
equivalent feature-by-feature passes; see [TESTING.md](../TESTING.md).
Back up any existing saves before starting.

## What to download and supply

1. Open the [0.6.0-alpha release](https://github.com/AsusFarstrider/dominion-wars-modern/releases/tag/v0.6.0-alpha)
   and download its attached **dominion-wars-modernizer-0.6.0-alpha.zip**, not the
   automatically generated **Source code** ZIP. No Git, PowerShell knowledge or
   build tools are needed to use an assembled release.
2. Right-click the downloaded ZIP, choose **Extract All**, then open the extracted
   folder. Do not run the installer from inside the ZIP.
3. Insert your legitimate North American retail CD **or mount an image made from
   your own disc as a Windows CD/DVD drive** (for example ISO or BIN/CUE with a
   suitable mounting tool). The installer reads the mounted drive, not the raw
   image file. Confirm its disc files are visible in File Explorer, and
   obtain the official North American 1.04 patch yourself. The package includes
   neither the game nor that patch, and does not bypass disc checks.
4. Double-click **Install Dominion Wars.cmd**. Follow its prompts to select the
   patch and complete the patch wizard. Drive letters and usernames need not
   match the developer's machine. If Windows or antivirus blocks a component,
   record the exact warning; do not disable protection to continue blindly.

The default installation is `%LOCALAPPDATA%\Dominion Wars` (your user's local
application-data folder). It creates a **Dominion Wars** desktop shortcut. The
installer does not require a full modernizer uninstall/reinstall between tests.

## Test A: normal launcher first

Use the **Dominion Wars** desktop shortcut. Confirm the menu, a mission, mouse
selection, one save/load and normal quit. Keep graphics/visual presets unchanged.
The launcher briefly initializes a compatible desktop mode; DDrawCompat may then
return the physical display to your normal resolution. That alone is not failure.
The normal mode still has a known Alt+Tab limitation; leave Alt+Tab for Test B.

The default graphics selection probes capability rather than choosing by GPU brand.
If startup fails, preserve logs **before another run overwrites them**. Then try
**Play Dominion Wars - Native Graphics.cmd** in the installed game folder. It
changes the saved backend selection; record which launcher was used and do not
treat a change in backend as an identical test. Hardware selection on hybrid
systems remains a separate uncertainty. Do not change drivers or Windows graphics
preferences mid-test without recording it.

## Test B: optional focus mode

After a successful normal run, open the installed game folder and double-click
**Play Dominion Wars - Experimental Alt-Tab.cmd**. This is per-run opt-in, currently
requires a driver-tested 1280x960 initialization mode and the verified NA 1.04
executable, and does not combine with visual presets. If that mode is unavailable,
continue testing the normal launcher and report the limitation.

1. Load a mission and **pause manually**. Automatic pause when unfocused does not
   work; simulation/audio may continue in the background.
2. Alt+Tab away. Check the game minimizes and your desktop is usable.
3. Alt+Tab back. Check image redraw, mouse selection and resuming gameplay.
4. Repeat, returning by clicking its taskbar icon.
5. With the game focused, take a screenshot using your usual Windows shortcut.
   Tell us the exact shortcut; if using Win+Shift+S, check both the captured image
   and input after dismissing the overlay.
6. Quit and check your original display settings. If everything works, extend to
   a longer mission/save-load session, changing only one condition at a time.

The optional launcher temporarily swaps its verified wrapper/profile and restores
them afterward. If interrupted, the next ordinary launch attempts recovery. If
it reports a conflict or changed backup, stop and retain the logs/backups; use
**Repair Dominion Wars.cmd** from the extracted package to restore packaged
compatibility files. If repair refuses a changed/missing backup too, keep the
evidence and report the exact error rather than deleting recovery records.
Repair resets those files; keep copies of any intentional
customizations first. It does not patch or delete saves.

## Reporting and undo

Before another launch, copy the installed folder's `.dw-launcher\launcher.log`,
`.dw-launcher\diagnostics.json` and `DDrawCompat-ds9dw.log` into a separate folder.
**Diagnose Dominion Wars.cmd** updates diagnostics without launching or changing
the display/profile, but may replace the previous report: copy it first. Review
logs for personal paths and hardware identifiers before sharing.

For installation failures, also copy the relevant run and referenced helper logs
from `%LOCALAPPDATA%\Dominion Wars Modernizer\InstallLogs`; these do not require
a successful game installation. Review them for personal paths before sharing.

Include Windows version/build, CPU and GPU model(s), driver version(s),
monitor count/layout/resolutions/scaling and primary display, disc edition,
candidate version, launcher used, exact point of failure and error text. Include
the executable hash if available (diagnostics normally records it). State whether
the alternate backend worked and whether the failure happened before or after
switching focus. Do not upload game executables, assets, saves or disc images
unless their sharing is separately appropriate; ordinary bug reports need logs.

To undo the modernizer, close the game and run **Remove Dominion Wars Modernizer.cmd**
from the extracted package. It removes unchanged managed components, restores its
recorded originals and preserves modified files according to the README. Game
assets, saves and diagnostic evidence remain. This is not a full game uninstall.

The packaged baseline and optional focus-mode regressions passed on the known
AMD system using an existing verified 1.04 installation as source. The later
Windows 10 / Intel / NVIDIA original-CD install and first-mission check provide
additional but shorter coverage; its final click guard was tested separately.
New hardware and longer sessions remain compatibility tests, not guaranteed
successes. Follow the release notes for the exact validation boundaries.
