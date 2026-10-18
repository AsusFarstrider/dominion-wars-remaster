# Optional graphics and resource-saving experiments

Candidate: 0.4.0-alpha, 2026-10-09. No gameplay comparison has been performed
with these presets. The compatibility baseline remains the default.

## Presets

| Setting | Baseline values | Sharper | Enhanced 3D | Resource-saving |
|---|---|---|---|---|
| DisplayFilter | bilinear(0) | bicubic(0) | bilinear(0) | bilinear(0) |
| ResolutionScale | app(1) | app(1) | app(2) | app(1) |
| Antialiasing | off | off | off | off |
| FpsLimiter | off | off | off | flipstart(60) |

Baseline launches do not rewrite a customized profile to these values. The
table lists the upstream defaults used to isolate the experimental runs. Each
experimental preset pins all four controls temporarily and varies only one
from those defaults. Backend selection and required game fixes are untouched
by the preset code.

The installed command files are `Play Dominion Wars - Sharper.cmd`,
`Play Dominion Wars - Enhanced 3D.cmd`, and
`Play Dominion Wars - Resource-saving.cmd`. The normal shortcut and
`Play Dominion Wars - Baseline.cmd` use the original profile.

Advanced combinations use the same launcher, for example:

```powershell
.\Launch-DominionWars.ps1 -VisualPreset Enhanced3D -GraphicsBackend Native
```

The explicit backend is persistent; the visual preset is not. Presets cannot
be combined with `-ConfigureOnly`. `-DiagnoseOnly -VisualPreset Sharper` records
the requested experiment but does not apply it or recover an interrupted preset.

## What to expect, and what not to claim

- Sharper changes presentation filtering, not the game's assets. It may improve
  perceived sharpness, but text/edge ringing must be compared rather than assumed.
- Enhanced 3D uses integer 2× scaling: the engine still sees 800×600 while the
  wrapper targets 1600×1200 3D rendering. The required temporary desktop size is
  unchanged. Scaling may expose sprite/HUD glitches or add significant GPU work.
- Resource-saving attempts to cap page-flipped frames at 60. It may reduce
  excess CPU/GPU work, but is not a speed improvement, may add input latency,
  and will not limit paths that do not use the relevant page-flipping mechanism.
  It does not force a 60 Hz display mode.

The existing AMD test log reports support for 2×/4×/8× MSAA. That is capability
evidence, not proof of correct game rendering. MSAA, texture-filter overrides,
high-polling mouse limits, and reduced presentation delay are deliberately not
combined into these presets. They remain separate possible experiments.

32-bit rendering and low-blur bilinear presentation were already enabled by
DDrawCompat defaults in the known-good log; they are not new improvements here.
No CPU-affinity, exclusive-fullscreen, driver-control-panel, or system-wide
performance changes are made.

Settings and cautions are checked against the bundled 0.7.1 source and the
[DDrawCompat configuration reference](https://github.com/narzoul/DDrawCompat/wiki/Configuration).

## Recovery and user changes

Before applying a preset, the launcher preserves the exact original INI in
`.dw-launcher\visual-presets\<unique ID>\original.ini` and writes a pending
`.dw-launcher\visual-preset.json` record with original/applied hashes.

Normal exit or startup failure restores the original bytes if the current
profile still matches the applied preset. A killed launcher leaves the record
for the next ordinary launch; recovery runs only after the existing session/game
checks, before any new profile changes or game startup. The independent desktop
helper restores display state, not visual profiles.

If the profile was edited or deleted after application, recovery keeps the
later state, retains the original backup, and archives `conflict.json` beside
it. The launcher warns that preset values may remain. Repair returns managed
files to the packaged baseline; custom originals remain in the retained backups.
A missing or altered backup is an error, not permission to overwrite a profile.
No global configuration or overlay file is edited by these experiments, and
a higher-priority overlay may defeat the requested settings.

## Later gameplay comparisons

Use a separate disposable installation and copied save. Keep the same mission,
camera, backend, desktop mode and refresh rate for each comparison:

1. Baseline: capture the HUD/text, ship silhouettes, effects and menu input.
2. Sharper: compare text readability, ringing, videos and fine edges.
3. Enhanced 3D: compare ships and effects; check menus, HUD, transparency,
   selection outlines, targeting, loading and screenshot/video behavior.
4. Resource-saving: compare responsiveness and CPU/GPU load during an identical
   scene. Verify that a real frame cap occurs instead of inferring it from the
   monitor refresh rate.
5. After each run, verify original profile bytes and desktop restoration.

DDrawCompat's Shift+F12 statistics overlay reports presentation/flip rates, not
necessarily the game simulation rate. Use those counters as supporting evidence,
not a claim of measured gameplay FPS. Inspect the final graphics-log configuration
and `.dw-launcher\diagnostics.json`, which records the requested preset and the
actual process-profile contents used for that launch. Copy logs between runs.

## Automatic checks

`tests/Test-VisualPresets.ps1` covers isolated settings, repeated-preset refusal,
preservation of core/backend/unrelated settings, exact-byte recovery, conflicts,
already-restored records and tampered-backup rejection. The orchestration suite
covers a real dummy child process, simulated failures, interrupted-preset recovery,
diagnostics without edits and incompatible configure-only requests. No automated
test proves image quality, reduced resource use, or real GPU-driver correctness.
