# Dominion Wars Modernizer

Bring your original *Star Trek: Deep Space Nine – Dominion Wars* back to a
modern Windows PC. The Modernizer installs the game from your own disc, helps you
apply the official patch, and adds a launcher for its compatibility fixes. It
keeps the original game and assets intact; this is not a remaster.

> [!WARNING]
> **Early alpha:** tested on a limited number of systems, not a guarantee that
> every computer will work. Back up your saves and expect some unresolved issues.

[Install](#how-to-install) · [Troubleshooting](#troubleshooting) · [Technical information](#technical-information)

## What you'll need

- A **64-bit Windows 10 or Windows 11** computer.
- An original **North American retail Dominion Wars CD**, or a mounted image
  made from your own CD.
- The official North American **1.04 patch**, `ds9dw1to4.exe`.
- The [Modernizer 0.6.0-alpha release ZIP](https://github.com/AsusFarstrider/dominion-wars-modern/releases/tag/v0.6.0-alpha).

The official patch is available on the historical
[TrekCore Dominion Wars downloads page](https://gaming.trekcore.com/dominionwars/downloads.html).
Choose **Official Patch (v1.01 to v1.04)**. The incremental, European, German and
missions patches on that page are not the file this installer accepts.

**The game and official patch are not included.** You do not need Git,
development tools, or PowerShell knowledge; the command files handle the scripts.

### Using your own disc image

A mounted **ISO** can replace a physical CD drive, provided Windows shows it as a
CD/DVD drive containing the original disc files. BIN/CUE images can also be used
with a suitable mounting tool; a BIN may need its matching CUE file. Any drive
letter is fine.

The Modernizer does not create or mount images, or read an unmounted image file.
After mounting, check that the new CD/DVD drive opens in File Explorer. Keep the
CD inserted or image mounted during play wherever the original game requires it.

## How to install

1. Open the [0.6.0-alpha release page](https://github.com/AsusFarstrider/dominion-wars-modern/releases/tag/v0.6.0-alpha).
   Under **Assets**, download **`dominion-wars-modernizer-0.6.0-alpha.zip`**.
   Do not choose **Source code (zip)** or **Code → Download ZIP**.
2. Right-click the downloaded ZIP and choose **Extract All**. Open the extracted
   folder; do not run anything from inside the ZIP.
3. Insert your original CD or mount your own disc image. Confirm that its CD/DVD
   drive and files are visible in File Explorer. Close the old disc setup program
   if it opens automatically.
4. Put **`ds9dw1to4.exe`** in the extracted Modernizer folder, beside the installer.
   Your Downloads folder also works. If it is elsewhere, the installer can ask
   you to select it.
5. Double-click **`Install Dominion Wars.cmd`**.
6. Follow the prompts. The Modernizer checks and copies your disc files, then
   opens the official 1.04 updater. Complete that updater's prompts too.
7. Wait for **`Installation completed successfully`**, then press a key to close
   the installer window. Keep the extracted Modernizer folder for repair or removal.

Disc checks and copying can take several minutes. The installer shows its current
stage, elapsed time and measurable progress during longer operations. A warning
about no recent progress does not necessarily mean it has frozen.
**Do not press Ctrl+C to make it advance**; that requests cancellation.

The default game folder is:

```text
%LOCALAPPDATA%\Dominion Wars
```

A **Dominion Wars** shortcut is created on your desktop. The Modernizer normally
runs as your usual Windows user; it does not normally need administrator rights.
For security warnings, see [troubleshooting](#windows-shows-a-security-warning).

## How to play

Double-click the **Dominion Wars** desktop shortcut. If it is missing, press
**Win+R**, paste `%LOCALAPPDATA%\Dominion Wars`, press Enter, and double-click
**`Play Dominion Wars.cmd`**.

Your screen may briefly change resolution during startup. This works around the
original game's mouse/display problems. The launcher attempts to restore your
desktop when the game exits; Windows may move other windows or desktop icons
during the switch.

Use this shortcut or launcher rather than opening `ds9dw.exe` directly, so the
display preparation, recovery checks and cleanup run. Only one game/launcher
should run at a time.

The normal launcher still has a known **Alt+Tab return problem**. If switching
away is important, see the separate experimental option below.

## Optional features

Start with the normal desktop shortcut. Once it works, alternatives are in the
**installed game folder**, not the extracted download. Open it with **Win+R** →
`%LOCALAPPDATA%\Dominion Wars` → Enter.

| Command file | What it changes |
|---|---|
| `Play Dominion Wars - Experimental Alt-Tab.cmd` | Experimental minimizing and return through Alt+Tab or the taskbar. |
| `Play Dominion Wars - Native Graphics.cmd` | Lets Windows choose its default graphics route. |
| `Play Dominion Wars - D3D9On12 Graphics.cmd` | Selects the graphics option used on the tested RX 6800 XT; requires DirectX 12 support. |
| `Play Dominion Wars - Sharper.cmd` | Experimental sharper image scaling; does not replace textures or assets. |
| `Play Dominion Wars - Enhanced 3D.cmd` | Tries a higher-resolution 3D image; may cause glitches or run slower. |
| `Play Dominion Wars - Resource-saving.cmd` | Attempts a 60 FPS limit to reduce unnecessary work; not a speed boost. |
| `Play Dominion Wars - 60 Hz.cmd` | Prefers a supported display refresh rate near 60 Hz, not necessarily exactly 60 Hz. |

**Experimental Alt+Tab does not automatically pause the game. Pause manually
before switching away.** It requires a supported 1280×960 startup mode and cannot
be combined with the visual presets. It passed a short test on the AMD system;
longer sessions, other GPUs and screenshot-overlay behavior still need testing.

Graphics-path choices are remembered by the normal shortcut. Visual presets apply
to one run and attempt to restore your previous settings afterward. Use only one
experiment at a time. The alternative graphics path has caused exit crashes on
the tested AMD system, and visual presets have not received gameplay comparisons;
none is a universally better setting.

See [graphics experiments](docs/GRAPHICS.md) and
[advanced graphics and focus behavior](#graphics-paths-and-experimental-recovery)
before combining or troubleshooting these options.

## Troubleshooting

### The installer cannot find my disc

Open the disc in File Explorer. It must appear as a **CD/DVD drive**, with
`data1.hdr`, `data1.cab`, `data2.cab` and an `rs` folder. A raw image file or a
folder copied from the CD is not enough for automatic detection. Check that your
image is mounted and that you have the North American retail edition.

### The installer cannot find or validate the patch

Use the full North American `ds9dw1to4.exe` from the link above. Place it beside
`Install Dominion Wars.cmd` or select it when asked. Renaming a different patch
will not work: the installer checks its contents. If the file is rejected,
preserve the error and check that you chose the v1.01-to-v1.04 download.

### The installer looks stuck

Read the current stage and elapsed time. Copying from an optical disc can be
slow, and the official updater may be waiting for you in another window.
Version 0.6.0-alpha includes progress reporting and a safeguard against accidental
mouse-selection pauses. If text is selected and output stops, press **Esc** once.
Do not disable security software or press Ctrl+C to force progress. For a
persistent wait, retain the [installation logs](#diagnostics-and-logs).

### It says the destination already exists

The CD installation path does not overwrite an existing game folder. If the
Modernizer is already installed, use **Repair** instead. If this is an older or
partial installation, back up your saves and read the
[existing-installation instructions](#advanced-powershell-usage) or report the
error. Do not delete an unfamiliar game folder just to bypass the check.

### The game will not start

Use the Modernizer shortcut, keep your disc available, and copy the exact error
before closing its window. Close any other game/launcher instance. If Modernizer
files are missing or changed, use **Repair** below. If an error specifically
says a DirectX 12 device cannot be created, use
`Play Dominion Wars - Native Graphics.cmd`. Other startup failures need
[diagnostics](#diagnostics-and-logs), not a guaranteed one-size-fits-all tweak.

### My screen changes resolution

A temporary change at launch is expected. It is not a new native game resolution
or an asset upgrade. Use the launcher and exit normally so it can restore the
desktop. See [display handling](#display-selection-and-restoration) for details.

### The game will not return properly after Alt+Tab

The normal launcher has a known return failure, sometimes showing
`DDERR_NOEXCLUSIVEMODE`. The [experimental Alt+Tab launcher](#optional-features)
is a separate, limited-tested option for future runs, not a guaranteed recovery
for an already-broken session. Pause manually before switching away.

### My desktop stayed at the wrong resolution

Close the game and its launcher, and allow the automatic recovery helper to finish.
If needed, run **`Recover Dominion Wars Desktop.cmd`** in the installed game
folder. It attempts a guarded restore; it is not guaranteed. You can also restore
your preferred resolution in **Windows Settings → System → Display**.

### Windows shows a security warning

The scripts and legacy helper are unsigned and may trigger SmartScreen or
antivirus warnings. Check that you obtained the ZIP from this repository's
release page and retain the exact warning. Do not turn off antivirus or Windows
security protections to continue blindly; stop and report anything you cannot
verify.

### It worked before, but now fails

Copy the [logs](#diagnostics-and-logs) before another attempt replaces them,
then try **Repair** and the normal shortcut. Windows/GPU-driver updates and
changed settings can affect compatibility. Repair restores Modernizer files,
not damaged game assets or saves; if it still fails, report what changed.

## Repair or remove the Modernizer

Close the game first. Keep backups of saves and any deliberate configuration
changes, since repair restores the packaged compatibility settings.

From the **extracted Modernizer download folder**:

- Double-click **`Repair Dominion Wars.cmd`** to restore its compatibility files
  and launcher, and recreate a missing desktop shortcut without replacing one
  you have edited.
- Double-click **`Remove Dominion Wars Modernizer.cmd`** and confirm the prompt
  to remove its managed compatibility layer and unmodified shortcut, restoring
  recorded original files/shortcuts where applicable.

**Removal does not uninstall the original game or delete saved games.** Modified
files and diagnostic logs may remain. Do not delete the `.dw-modernizer` state
folder first if you want removal to restore recorded originals.

These command files target the default installation. Custom locations need the
[advanced commands](#advanced-powershell-usage).

## Compatibility and known limitations

Dominion Wars Modernizer has been tested on a limited number of Windows 10 and
Windows 11 systems, including **AMD and Intel processors, and AMD and NVIDIA
graphics cards**. Compatibility with all hardware configurations is not guaranteed.

| System | What has been exercised |
|---|---|
| Windows 11; AMD Ryzen 7 5800X; AMD Radeon RX 6800 XT; three monitors | Several single-player missions, save/load controls, desktop recovery checks, and a short experimental Alt+Tab regression. |
| Windows 10 Home build 19045; Intel Core i5-3570K; NVIDIA GeForce GTX 670 | Original-CD installation and brief first-mission gameplay; the updated installer click/focus guard passed a separate test. |

Coverage varies by version and feature. The Windows 10 installation needed Esc
to release console selection before the guard was added; the guard was then
validated separately, not through another full installation. Extended gameplay,
experimental settings, save reliability and the full campaign have **not** all
been validated on both machines. See [TESTING.md](TESTING.md) for dated evidence.

Other important limits:

- Only the known **North American retail media and official 1.04 patch** are
  accepted. Other regional editions are not currently supported.
- **Single-player is the target. Multiplayer, including LAN/direct-IP, is
  untested and unsupported.**
- A baseline AutoSave reload crash was observed. Later repeated-load and
  save/overwrite controls passed, but the original fault remains unresolved.
  Keep backups; successful individual tests are not a save-reliability guarantee.
- Normal Alt+Tab remains unreliable. The optional focus mode and screenshot
  handling are not universally validated; automatic pause does not work.
- Intel integrated graphics, hybrid-GPU laptops, other GPU/driver combinations,
  Windows on ARM, unusual monitor layouts/scaling, HDR and remote/virtual
  displays need testing. Intel **CPU** testing is not Intel **GPU** testing.
- Visual presets, fallback display modes and different-resolution Alt+Tab returns
  have more limited evidence than the default AMD gameplay configuration.
- Desktop recovery cannot guarantee success after a forced closure, power loss,
  reboot, monitor replacement or later display changes. It does not restore the
  entire monitor layout, window positions or desktop icons.

## Technical information

You do not need this section to install or play. It preserves the compatibility
recipe and advanced controls for testers and developers.

The original InstallShield setup is unreliable on modern 64-bit Windows. The
Modernizer instead reconstructs the game from verified original disc cabinets,
runs the owner-supplied official patch, and checks the resulting executable
before adding its compatibility files.

### Original renderer and texture fix

The original QUANTUM renderer is retained. Tracing shows the engine requesting
**800×600×16 internally**; the launcher's 1280×960 startup environment is not an
increase in the game's native resolution. Higher internal rendering presets are
wrapper experiments, not new assets or an engine rewrite.

The game takes a broken non-power-of-two texture path on modern hardware,
leaving dimensions uninitialized. This produced invalid allocation requests and
a later null-pointer access. Re-advertising the old `D3DPTEXTURECAPS_POW2`
limitation sends it through its working power-of-two path.

The bundled stock **DDrawCompat 0.7.1** profile is:

```ini
EnableDDrawCompat = on
FullscreenMode = borderless
DisplayAspectRatio = app
PalettizedTextures = on
CompatFixes = singlemonitor
CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps|=0x2
ForceD3D9On12 = on
```

Palettized emulation supplies the expected 8-bit texture format; `singlemonitor`
avoids the game's old secondary-monitor selection behavior. The translated
graphics path avoids reproduced AMD legacy-driver exit faults on the RX 6800 XT.
See [compatibility findings](docs/COMPATIBILITY.md) for crash evidence, the
historical wrapper discovery, input limitations and activation-order analysis.

### Graphics paths and experimental recovery

The normal launcher's `Auto` selection is capability-based, not GPU-brand-based.
It preserves the saved profile unless that profile forces D3D9On12 and the
default adapter cannot create a DirectX 12 device; then it saves
`ForceD3D9On12 = off` while retaining the texture fixes. There is no automatic
retry after a crash. Hybrid-GPU selection remains a separate uncertainty.

`Native` means Windows' **system-default D3D9 path**, not `forceoff`; on some
systems Windows may still translate it. `D3D9On12` explicitly selects `on`.
Graphics selections persist. Repair restores the packaged `on` setting, after
which the normal capability check can choose `off` again if needed.
`-GraphicsBackend Profile` skips that automatic adjustment.

Visual presets temporarily pin filtering, scaling, MSAA and frame-limit controls,
then restore the exact previous INI when safe. Baseline launches, including
`Play Dominion Wars - Baseline.cmd`, do not rewrite customized visual values.
See [preset settings, limitations and recovery](docs/GRAPHICS.md).

Experimental focus mode swaps in the separately hash-pinned DDrawCompat revision
3b plus its activation profile for one run. It verifies the NA 1.04 executable
and wrapper identities, backs up originals, and restores them after exit.
A later ordinary launch attempts interrupted focus/preset recovery before starting.
Changed/deleted files or damaged focus backups can block recovery; keep the
evidence and use repair rather than deleting records to bypass checks.
See [focus source, gates and build provenance](third_party/DDrawCompat-focus/README.md).

### Display selection and restoration

The launcher captures the primary display's complete `DEVMODE`, enumerates real
driver modes and tests candidates with `CDS_TEST` before switching. It prefers
1280×960, then 1024×768, 800×600 and 640×480. If necessary it considers other
available modes at least 800×600, with width below the reported 1600-pixel input
failure threshold. Color depth and orientation must match; refresh rates are
ranked near the original rate or the explicit preference. These are capability
tests, not proof of correct gameplay in every mode.

The temporary change does not update registry display defaults. DDrawCompat may
restore the physical desktop resolution during game startup, so the initialization
mode need not persist throughout play. The launcher waits for the game and
attempts cleanup on normal exit or failure.

An independent helper is armed before switching. Both normal and orphan recovery
check monitor identity, primary-display selection and the expected temporary mode/
position. They leave an already-restored desktop alone and avoid overwriting
later user changes. The helper waits until both launcher and game have exited;
it is not restarted after reboot. Controlled recovery tests do not establish
universal recovery or game stability.

### Diagnostics and logs

Press **Win+R**, paste a folder path below and press Enter. Nothing is uploaded
automatically; review local paths and hardware IDs before sharing.

**Installation, repair and removal:**

```text
%LOCALAPPDATA%\Dominion Wars Modernizer\InstallLogs
```

Each run has a timestamped log and references its extractor `.stdout.log` and
`.stderr.log` files. Logs survive staging cleanup and removal; they are not
automatically rotated. Include the relevant run and helper logs in an installer
report. Long operations report about every five seconds; sixty seconds without
measured byte changes produces a warning, not an automatic timeout.

The extractor runs without the interactive console, with stdin closed and both
output streams drained concurrently. The installer temporarily disables classic
console QuickEdit where supported, preserves Ctrl+C handling, and restores the
original mode unless something else changed it. Other terminal hosts and explicit
keyboard Mark mode may behave differently. Cancellation waits for owned I/O/
helper termination before staging cleanup; an outstanding optical read can delay
it. Close the interactive official updater to cancel that stage. Forced console
closure bypasses normal cleanup. A late compatibility-setup failure can leave a
verified game directory; only the final completion result indicates success.

**Game and launcher:**

```text
%LOCALAPPDATA%\Dominion Wars\.dw-launcher\launcher.log
%LOCALAPPDATA%\Dominion Wars\.dw-launcher\diagnostics.json
%LOCALAPPDATA%\Dominion Wars\DDrawCompat-ds9dw.log
```

Run **`Diagnose Dominion Wars.cmd`** in the installed folder for a report without
launching the game, changing displays or editing the compatibility profile.
The JSON is replaced on each run; copy evidence first. The launcher log keeps
one `launcher.log.previous` when it exceeds about 1 MiB at the next launch.

Diagnostics include Windows/GPU/driver and monitor information, executable/wrapper
hashes, requested and tested modes, graphics choices, profile contents, additional
DDrawCompat configuration paths and legacy Windows compatibility flags. Inventory
failures are recorded rather than blocking launch. Existing overlay configuration
can override the selected profile; check the graphics log's final configuration.
Legacy Windows flags are inventoried, not automatically removed.

Display recovery state is stored under:

```text
%LOCALAPPDATA%\Dominion Wars Modernizer\DisplayRecovery\session-<Windows session ID>.json
```

Errors from that helper go into the adjacent `.json.log`. Original compatibility
files/state live in the game's `.dw-modernizer` directory. Other retained backups
and pending records are under `.dw-launcher`: `focus-backups` / `focus-session.json`,
`visual-presets` / `visual-preset.json`, and `profile-backups` / `profile-reset.json`.
Keep these if recovery reports a conflict.

For a bug report, include Windows edition/build, CPU, GPU and driver, monitor
layout/resolutions/scaling/primary display, disc edition, Modernizer version,
launcher used, exact failure stage/error and whether the desktop restored.
Include the executable hash if available; diagnostics normally records it.
Do not upload game assets, executables, disc images or the official patch.

### Advanced PowerShell usage

Use Windows PowerShell from the **extracted Modernizer folder**. These examples
are for an existing, unmodified and structurally complete North American 1.04
installation; the ordinary CD workflow above does not need commands.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\DominionWarsModernizer.ps1 Status -Destination 'C:\Games\Dominion Wars'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\DominionWarsModernizer.ps1 Install -SourcePath 'C:\Games\Dominion Wars' -Destination 'C:\Games\Dominion Wars'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\DominionWarsModernizer.ps1 Repair -Destination 'C:\Games\Dominion Wars'
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\DominionWarsModernizer.ps1 Uninstall -Destination 'C:\Games\Dominion Wars'
```

Use your actual path. An installed 1.01 tree is not accepted directly: update it
with the official patch first or use the mounted-media workflow.
Repair requires the known 1.04 executable and Modernizer state; it cannot repair
arbitrary game assets. Removal preserves later changes without an original
backup, so a backend-modified INI may remain. `-NoDesktopShortcut` opts out of
shortcut changes, and `Get-Help .\DominionWarsModernizer.ps1 -Detailed` lists parameters.

From the **installed game folder**, advanced launcher examples include:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Launch-DominionWars.ps1 -GraphicsBackend Profile
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Launch-DominionWars.ps1 -Width 1024 -Height 768 -NoResolutionFallback
```

`Reset Dominion Wars Compatibility.cmd` backs up the game-specific INI, local
`DDrawCompat.ini` and game overlay, then applies the clean recipe and removes
those local overrides. `Undo Dominion Wars Compatibility Reset.cmd` restores
originals but refuses to overwrite newer file edits. Only one reset can be pending;
global DDrawCompat settings and Windows flags are untouched. Neither command
launches the game or changes the display. Repair is not an undo of custom settings.

### Development and testing

- [Dated validation and unresolved investigations](TESTING.md)
- [Technical compatibility findings](docs/COMPATIBILITY.md)
- [New-system testing guide](docs/NEW-SYSTEM-TESTING.md)
- [Broader-hardware work and developer checks](docs/PORTABILITY.md)
- [Release notes](RELEASE-NOTES.md)
- [Published package manifest](MANIFEST.json)

The accepted NA 1.04 executable SHA-256 is:

```text
03C20A3535D0209111FF9F3B90EBA403E059DF2BE7E784D2746ABF1535C1BE6A
```

## Legal information and credits

This is an independent fan compatibility project, not affiliated with or endorsed
by the game's developers, publishers or rights holders. It distributes **no game
assets, official patch, modified game executable, no-CD patch or DRM bypass**.
You must supply legitimate original media and the required patch.

- **DDrawCompat 0.7.1:** Narzoul's compatibility wrapper. The bundled license is
  [included here](payload/DDrawCompat-LICENSE.txt);
  [upstream source](https://github.com/narzoul/DDrawCompat).
- **Optional DDrawCompat focus build:** game-specific revision 3b, separately
  hash-pinned, with its BSD Zero Clause license, source patch and
  [build/provenance record](third_party/DDrawCompat-focus/README.md).
- **i6comp v0.20:** created by fOSSiL and updated by Morlac; the bundled helper is
  hash-verified. Its headers authorize use with proper credit; the
  [attribution and authorization text](payload/tools/I6COMP-LICENSE.txt) accompany it.
- **Jiri Dvorak's historical wrapper/source:** research provenance for the broken
  texture path and POW2 workaround. That wrapper binary, source and game code
  are not shipped in this project.
- **Modernizer source:** [MIT License](LICENSE.txt).
