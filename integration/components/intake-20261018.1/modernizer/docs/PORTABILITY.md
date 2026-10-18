# Broader-hardware compatibility work

Current scope: compatibility and stability, not remastered assets, image-quality
upgrades or performance experiments. The optional visual presets below are
historical candidate additions, not part of the current validation sequence.

## Candidate 0.4.0-alpha additions

The 0.5.0-alpha candidate packages the separate, opt-in focus mode while retaining
stock baseline behavior. Its prototype passed a short minimize/return test on the
known AMD system; different-mode transitions and long focus sessions remain
unvalidated. Packaged baseline and optional focus regressions also passed on
2026-10-16 on that AMD system. Windows 10 / Intel Core i5-3570K / NVIDIA GTX 670
now has original-media installation and brief first-mission evidence, plus a
separate installer click-guard pass; optional focus mode is not validated there.
See [the dated testing record](../TESTING.md) for the distinction. The current clean-install sequence is in
[new-system testing](NEW-SYSTEM-TESTING.md). The 0.4 findings below are historical.

Optional per-run Sharper, Enhanced 3D and Resource-saving presets now isolate
presentation filtering, 2× render scaling and a conditional 60 FPS limit. The
normal launcher keeps the original visual profile. Exact-byte backups, normal/
error cleanup and next-launch recovery have automatic coverage; gameplay/image
quality/resource use have not been validated. See [graphics experiments](GRAPHICS.md).
These optional experiments do not replace any compatibility safeguard below.

Stability hardening on 2026-10-10 gives ordinary shutdown and independent
recovery the same monitor-identity and mode/position guards. An already-restored
desktop is left alone, later display adjustments are preserved, and unknown
recovery phases cannot authorize a display change. Controlled parent termination
and actual restoration of a synthetic orphaned mode passed on the known AMD
system. These checks do not establish gameplay crash resistance on other hardware.

## Candidate 0.3.0-alpha

The original working configuration remains the gameplay baseline. These changes
remove avoidable launcher assumptions; they do not establish universal support.

| Change | Reason | Evidence so far |
|---|---|---|
| Selectable system-default D3D9 path | Forced D3D9On12 is experimental and requires Windows 10+ and a DX12-capable GPU | AMD retest reached a mission but faulted in AMDXN32.DLL; translated path retained on this system; other hardware pending |
| Enumerated, driver-tested display modes | Not every monitor/driver offers 1280×960 or the same refresh rates at each size | Real local mode capability tests plus synthetic missing-mode/refresh-rate tests |
| 1024×768 / 800×600 / 640×480 fallbacks | Preserve a narrow desktop when the preferred size is unavailable | Selection unit tests; explicitly selected 1024×768 and 800×600 passed brief mission/start/exit checks on AMD; 640×480 and automatic failover gameplay pending |
| Session-wide launcher lock | Simultaneous launchers could restore each other's temporary modes | Separate-process lock rejection test |
| Launch diagnostics and real exit codes | A black-screen report alone cannot distinguish driver, input, display, or executable failures | Real game startup/clean exit, simulated failures, and read-only hardware diagnostics |
| .NET hashing instead of module-discovered commands | The actual startup environment lacked `Get-FileHash` even though development shells had it | First real launch exposed the issue; repaired candidate launched successfully; restricted-module regression |
| Payload preflight and DLL hash check | Missing or substituted components should not be silently installed | Disposable install/repair/removal tests; exact 0.7.1 DLL remains bundled |
| Additional narrow desktop modes | Some laptops/TVs expose no 4:3 desktop modes | Synthetic missing-4:3/current-mode tests; widescreen fallback gameplay pending |
| Optional 60 Hz preference | A high-refresh environment may expose timing/driver problems | Supported-rate ordering tests; not a fixed 60 Hz guarantee or proven gameplay fix |
| Independent display recovery | Parent-process death can bypass launcher cleanup | Recovery policy/handshake tests; controlled termination and real synthetic-orphan restoration passed on the known AMD system |
| Reset/undo game-local configuration | Leftover generic or overlay settings can defeat the recipe | Byte-exact backup/restore, repeated-reset refusal, and later-edit protection tests |
| Pin enabled/borderless/app-aspect defaults | Keep an explicit recipe despite lower-priority shared settings | Packaged/reset profile equality check; these are existing DDrawCompat defaults |
| Inventory legacy Windows flags | Old compatibility flags can impose conflicting display/DPI behavior | Read-only registry/inherited-environment inventory; no flags are removed automatically |
| Active-game and locked-file protection | Repairing a running game can leave partially replaced components | Target-process guard and exclusive locked-file preflight rejection tests |
| Owned shortcut backups | Installation should not permanently lose an existing same-name shortcut | Fake-desktop tests verify original bytes are restored and edited shortcuts preserved |
| One normal launcher with capability-based backend adjustment | Non-DX12 hardware should not need a vendor-specific package or be forced through D3D9On12 | Pure selection and dummy-process orchestration checks; multi-vendor gameplay pending |

These additions are implemented and included in the 0.4 candidate. Its short
baseline run worked except Alt+Tab; see [testing evidence](../TESTING.md).
That run does not validate fallback sizes, alternative GPUs, longer-session
stability, or optional visual presets.

Upstream describes `ForceD3D9On12` as experimental and requiring a DX12 GPU. We
retain it by default because it avoids a reproduced AMD exit fault on the known
configuration, but expose `off` as a deliberate alternative. We do not guess a
preferred backend from the GPU brand. `off` permits the Windows default, not
necessarily a native implementation; `forceoff` is deliberately not selected.
See the [DDrawCompat configuration reference](https://github.com/narzoul/DDrawCompat/wiki/Configuration#forced3d9on12).

The normal shortcut uses `Auto`: retain the profile when usable; if it forces
D3D9On12 but default-adapter DX12 creation fails, save `ForceD3D9On12 = off`
and continue through system-default D3D9 with the same texture fixes. Existing
`off` selections remain respected. Manual graphics launchers remain overrides;
an explicit D3D9On12 request still fails before switching the display when that
probe fails. `-GraphicsBackend Profile` opts out of automatic adjustment.
This is capability-based selection, not proof that either path will run the game
on every driver. There is no crash-triggered retry or performance-based selection.

Display modes come from the named primary device and are checked with
[`EnumDisplaySettingsEx`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumdisplaysettingsexw)
and [`ChangeDisplaySettingsEx`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-changedisplaysettingsexw).
`CDS_TEST` is not a display change. A successful test still cannot guarantee that
the actual switch will succeed; the launcher also handles switch failure.

The DX12 probe attempts `D3D12CreateDevice` on the default adapter at feature
level 11.0. It is a capability check, not proof that D3D9On12 or Dominion Wars
will render correctly, and not a complete inventory of laptop/hybrid adapters.

## Test priorities

Keep the North American 1.04 executable fixed while changing one condition at a
time. Do not upload disc images, executable files, or other copyrighted assets.

Compatibility and stability take priority over visual/performance experiments.
Testing has begun; the remaining sequence is:

1. Extend the passed controlled termination/orphan recovery and fixture lifecycle
   checks with installed manual recovery and real shortcut repair/removal.
2. Investigate the unresolved baseline AutoSave crash before claiming longer-
   session stability. Fresh-process, repeated-load, new-slot and AutoSave-overwrite
   controls passed; the original dump now provides a UI-callback/timing lead.
   Confirm repeated startup/quit and mission transitions without treating those
   controls as a fix. The baseline Alt+Tab failure remains; separately test the
   optional 0.5 focus mode without treating one AMD pass as broad validation.
3. Extend the brief GTX 670 result to more NVIDIA GPUs and Intel integrated
   graphics with the packaged path, then the alternate graphics
   launcher if needed. Record results for both rather than replacing logs.
4. Extend Windows 10 testing beyond startup and the installer checks, and test
   other Windows 11 builds, single-monitor configurations,
   125%/150% scaling, differing refresh rates, and hybrid-GPU laptops.
5. Test a display without 1280×960, confirming fallback gameplay and restoration.
6. Explore portrait primary monitors, HDR, remote/virtual displays, and GPU
   updates only after the simpler configurations have results.

For each run, record installation success, chosen display size/refresh rate,
backend, videos, menu input, mission loading, 15+ minutes of gameplay, save/load,
Quit, and restored display state. Include `.dw-launcher/diagnostics.json`,
`.dw-launcher/launcher.log`, and `DDrawCompat-ds9dw.log` after reviewing privacy.
Back up logs between attempts; diagnostics JSON is replaced on the next run.

## Remaining boundaries

- Only known North American media/patch/executable hashes are accepted. Regional
  support requires verified media and a separate investigation, not relaxed hashes.
- Single-player remains the target; no multiplayer changes are made.
- The game still requests 800×600 internally. Desktop fallback is an input/display
  environment workaround, not a higher-resolution renderer.
- Recovery waits for both parent and game to exit, checks the saved primary
  monitor identity, and changes only the still-matching temporary mode/position.
  It cannot guarantee recovery if the helper also dies and does not start after
  reboot. Full monitor-topology, HDR, scaling, and window/icon-position restoration
  are not implemented.
- User/backend-modified profiles may remain after removal under the existing
  preservation rule. Repair intentionally restores the packaged profile.
- Most automated tests mock display changes. The opt-in real-display test covers
  controlled worker termination and synthetic orphan recovery on one system;
  neither kind simulates all GPU drivers or substitutes for actual playtesting.

## Changes deliberately not made

- No automatic backend choice by GPU brand: drivers and hybrid selections vary,
  and the known-good AMD workaround remains the default until other evidence exists.
- No forced exclusive fullscreen: upstream's borderless default is retained;
  exclusive mode has additional D3D9On12/driver limitations.
- No universal CPU-affinity, DPI, or performance tweaks. DDrawCompat already has
  affinity/DPI policies and newer NVIDIA workarounds; changing them without a
  reproduced game-specific fault would add variables, not demonstrated wins.
- No emulated-resolution replacement for the real desktop switch: earlier
  investigation found that it did not solve the separate mouse-input bug.
- No relaxed regional hashes, engine rewrite, higher native rendering resolution,
  multiplayer work, or speculative wrapper upgrade.

The rationale for keeping wrapper defaults is documented in the
[upstream configuration reference](https://github.com/narzoul/DDrawCompat/wiki/Configuration)
and [release notes](https://github.com/narzoul/DDrawCompat/releases). These are
constraints and research leads, not evidence of broader Dominion Wars validation.

## Developer checks

Run from the repository directory in Windows PowerShell 5.1:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-Launcher.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-LaunchOrchestration.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-RecoveryAndProfiles.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-Shortcuts.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-VisualPresets.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-Lifecycle.ps1 -VerifiedGame 'C:\Games\Clean Dominion Wars 1.04'
```

The first five tests need no game files and do not change the desktop. The recovery
test starts a helper with a disposable `Prepared` record and completes it without
arming/applying a mode; the shortcut test uses a fake desktop inside the fixture.
The orchestration suite disables the real watchdog and uses fake display operations;
it also invokes the restricted-module-environment regression. The lifecycle
test needs the owner's verified executable; it copies that file into an ignored,
non-playable structural fixture and uses no desktop shortcut. Test artifacts are
left in `.test-*` directories for inspection. No tests alter the source game tree.

The following separate, **opt-in** check temporarily changes the real primary
desktop. Close the game and all launchers first; do not run during other display
experiments or valuable fullscreen work:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\Test-RealDesktopRecovery.ps1 -ExerciseRealDisplay
```

It arms the shipped helper and kills only its disposable worker. If the desktop
has already returned when the worker dies, it separately establishes a synthetic
orphaned temporary mode with the same dead parent identity to exercise the
helper's actual restoration call. A controller cleanup attempt does not count
as a helper pass. Original/temporary snapshots and results stay in
`.test-real-recovery-*`; the installed game's recovery record is not touched.
