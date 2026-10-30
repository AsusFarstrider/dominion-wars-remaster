# Known limitations — 1.10 Alpha / int-20261029.1

## Mission → Main Menu is broken

**Do not choose Esc → Main Menu during a mission.** It opens the legacy Main menu
in the top-left of the screen and mouse clicks do not work. The owner approved
publication with this bug deferred. Close and relaunch instead; save first where
possible. If stuck, try Alt+F4 or end the game's ds9dw.exe in Task Manager; unsaved
progress may be lost. No fix is included in this release.

## Current native-1440p pilot

- Physical **2560×1440 primary display at origin (0,0) only**. Other modes are
  refused before launch; the launcher does not change your desktop resolution.
  Arbitrary resolutions, 1080p/4K and other DPI/monitor layouts are not validated.
- One known Windows 11 / AMD Radeon RX 6800 XT / three-monitor system. No universal
  Windows 10/11, NVIDIA, Intel or hybrid-GPU compatibility claim.
- North American retail official 1.04 inputs only. Multiplayer is unsupported and
  untested, even though the restored Main menu includes its button. Avoid it.
- Single-player is the test target, not full-campaign validation. The release
  session's launcher returned 1; do not read approval as a clean-exit test pass.
- HUD assembly still uses original-density art and original fonts/control geometry.
  Ship textures, models and captain portraits are not restored in this package.
- Restored button banks cache about 231 MiB in a 32-bit process. Report memory
  failures, slowdowns, missing art and driver/Windows-update regressions.
- Recording is off. A bounded renderer safety ledger remains enabled. Focus,
  screenshots, skips, save/load and longer play need broader testing; do not assume
  every requested check was completed merely because the owner approved release.
- Unsigned scripts and legacy helpers may trigger antivirus warnings. Do not
  disable security as an installation step. Keep separate originals and saves.
- Retained staging/backups need several GB of disk space. Repair/Remove are guarded
  rebuilds, not recursive deletion. See [installation](INSTALL.md).
- Package creation-time pending/held fields remain immutable historical records.
  The separate release acceptance record identifies the later approval exactly.

## Earlier int-20261018.1 original-assets experiment

The following limitations describe the unchanged earlier download, **not 1.10**:

- Very early development, with a short combined first-mission smoke on one
  Windows 11/AMD/three-monitor system. Long campaigns and other systems unverified.
- North American retail 1.04 inputs only. Other regional editions are not supported.
- 960×600 logical 16:10 canvas, not full 16:9 or arbitrary native UI dimensions.
  Original low-resolution UI artwork remains; no remastered textures/models or
  captain portraits are included.
- Primary display origin (0,0) is required by this candidate. Other layouts, DPI,
  hybrid GPUs and graphics drivers have not been established to work.
- No forced startup desktop resize; guarded recovery remains. Focus-return helper
  is bounded to 32 cycles per run. Automatic pause is not supported. Capture tools
  and repeated long-session focus switching are not universally validated.
- Native backend can be selected, but combined gameplay was not validated on it.
- Multiplayer is unsupported and untested.
- Unsigned scripts and legacy helpers may trigger warnings. Do not disable Windows
  security as an installation step.
- Several GB of spare disk may be needed; staging/failed operations/backups are
  deliberately retained rather than recursively cleaned up.
- Advanced `Repair -NoDesktopShortcut` does not carry prior shortcut ownership
  metadata forward. Use ordinary repair when retaining a managed shortcut. Normal
  shortcut creation/target/hash and exact-owned-shortcut removal were checked.

The private Saber flavor is not this release. Its art-distribution permissions and
combined visual testing remain outside this approved original-assets baseline.
