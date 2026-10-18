# Known limitations — int-20261018.1 original-assets

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
