# Technical compatibility findings

Original validation date: 2026-07-19; later dated checks are recorded below.

This document records the evidence behind the current Dominion Wars Modernizer
profile. The detailed rendering investigations below primarily concern the
Windows 11 / AMD Radeon configuration. A Windows 10 / Intel Core i5-3570K /
NVIDIA GTX 670 system has also received original-media installation and brief
first-mission testing, plus a separate installer click/focus-guard test. Those
results do not establish equivalent long-session or experimental-feature coverage;
see [TESTING.md](../TESTING.md). General Windows 10/11 compatibility is not proven.

## Validated result

The North American retail build was reconstructed from the original CD cabinets,
updated from 1.01 to the official 1.04 release, and run on Windows 11 build
10.0.26200 with an AMD Radeon RX 6800 XT driver 32.0.21043.19003 and a
three-monitor desktop.

Observed working behavior included all introduction videos, menu mouse input,
the first two Federation missions, the beginning of the first Dominion mission,
and a later continuous session covering the first four missions (the first three
completed and the fourth played until defeat). Windows recorded no application
fault on the final packaged launcher or longer gameplay tests, DDrawCompat
detached successfully, and the original 2560×1440 desktop mode was restored.

## Known-good profile

DDrawCompat v0.7.1 is installed as `ddraw.dll` beside `ds9dw.exe` with:

```ini
PalettizedTextures = on
CompatFixes = singlemonitor
CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps|=0x2
ForceD3D9On12 = on
```

The launcher temporarily changes the primary desktop to 1280×960, starts the
game, waits for it to exit, and restores the captured display mode in a `finally`
block. Runtime tracing shows the engine still requests 800×600×16 internally.

## Native failure

The unwrapped 1.04 executable changed display mode, displayed a black screen, and
then raised an access violation:

- exception: `0xC0000005`;
- fault: `ds9dw.exe+0x8A08` (`0x00408A08`);
- invalid operation: write to address `0x00000054`; and
- immediate cause: a failed internal creation routine returned null and the game
  wrote to offset `+0x54` without checking it.

DDrawCompat's release log showed managed texture creation failing with
`E_INVALIDARG`. A debug build exposed malformed texture dimensions of roughly
13.8 million by 13.9 million pixels.

## Texture root cause

The recovered source for Jiri Dvorak's historical Dominion Wars wrapper documents
that the game's non-power-of-two texture path does not initialize texture
dimensions correctly. The wrapper reintroduced `D3DPTEXTURECAPS_POW2`, causing
the game to select its working texture path.

DDrawCompat reproduces that capability advertisement with:

```ini
CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps|=0x2
```

After adding it, the managed `P8` and `R5G6B5` allocations that previously failed
succeeded and the game remained running. `PalettizedTextures = on` provides the
8-bit palettized format expected by the game. `CompatFixes = singlemonitor`
prevents the legacy display enumerator from selecting a secondary monitor.

## Mouse and display behavior

The historical wrapper author documented a mouse failure when the real primary
desktop width is at least 1600 pixels. DDrawCompat's virtual display-resolution
setting did not correct this independent input bug. Temporarily setting the real
primary desktop to a 4:3 mode does.

The initial 640×480 Windows compatibility flag proved the diagnosis and allowed
successful gameplay. The final launcher instead uses 1280×960, preserving the
intended 4:3 output while remaining below the mouse bug's width threshold.

## AMD exit failure and D3D9On12

At 1280×960 without D3D9On12, gameplay worked but some state transitions and
exits reproducibly faulted in AMD's legacy 32-bit Direct3D module,
`AMDXN32.DLL+0xBECCFE`. With `ForceD3D9On12 = on`, the first Dominion mission ran
normally and Quit produced a successful DDrawCompat detach with no Windows crash
event. This setting is validated for the tested RX 6800 XT; its behavior on other
vendors and drivers still needs evidence.

The 2026-10-13 retest on Windows 11 Home build 26300 / RX 6800 XT driver
32.0.21045.5002 again reached a rendered mission with `ForceD3D9On12 = off`, but
ended with access violation `0xC0000005`. Windows event 1000 names
`AMDXN32.DLL+0x00BECC8E` (version 32.0.21045.5002). The launcher restored the
desktop and the test harness restored the exact baseline profile. This reinforces
retaining the translated path on this tested AMD configuration; it does not
establish a preferred backend for every AMD driver, NVIDIA or Intel system.

## Save-load failure investigation

On 2026-10-13, a newly written AutoSave crashed on reload with the exact baseline
profile and translated backend still enabled. Windows reports
`ds9dw.exe+0x1DCE8`, exception `0xC0000005`; the dump shows an invalid read through
ESI at `test byte ptr [esi+4], 0x80`. This is a game-code fault, distinct from the
native-backend AMD driver fault. The underlying reason for the invalid pointer is
not established, and successful wrapper detach does not make this a clean exit.

The save and crash dump are preserved locally. The unchanged save loaded cleanly
from a fresh process's main menu and that process exited with code 0. Matching
file hashes argue against a consistently unreadable save; preceding runtime
state or the save/reload sequence remains under investigation. A second process
also loaded the save both from the main menu and from inside the mission without
an intervening save, then exited with code 0. Repeated loading alone did not
reproduce the fault. Writing a new slot and immediately reloading it is the next
control; an overwrite-specific cause is not established. That new-slot control
and a subsequent new-mission / AutoSave-overwrite / immediate-reload repeat both
passed, with exit code 0 and unchanged baseline settings. The original failure
therefore remains unresolved and unreproduced by the controls, not fixed.

The original dump's window-procedure arguments identify message `0x0202`, which
is [WM_LBUTTONUP](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttonup),
with wParam 0 and client coordinates (640,480). Stack returns `0x00433991` and
`0x0041DF9E` link the fault to an indirect handler and recursive object traversal;
the code tests rectangles/flags and invokes callbacks before using the object
pointer again. Without symbols, this is an inferred UI-hit-test/callback path,
not a proven function name or heap-lifetime diagnosis. UI lifetime/reentrancy or
timing is a lead to investigate, not justification to suppress mouse messages
or assume the save file is corrupt. No save repair, game-executable patch or
speculative compatibility setting has been applied.

## Alt+Tab failure investigation

On 2026-10-10, the 0.4 candidate baseline failed to return from Alt+Tab on Windows
11 Home build 26300 with RX 6800 XT driver 32.0.21045.5002 and three displays.
The game displayed `Error with Screen RestoreSurfaces() on primary:
DDERR_NOEXCLUSIVEMODE`. The single-player rendering recipe still worked before
switching away. This is a separate known limitation, not a corrected condition.

Three isolated settings in the disposable installation failed:

- `noactivateapp(1)` and `noactivateapp(0)` avoided the original error but left
  an apparently transparent/non-redrawing window and upper-left mouse confinement.
- `keepvidmem(1)` reproduced the original primary-surface restore error.

None was promoted to the packaged profile. A short run with the unchanged release
DLL and baseline settings plus `LogLevel = debug` produced call-level evidence:

1. On deactivation, the game called `RestoreDisplayMode`; its main window then
   became minimized, with coordinates around -32000 and style `0xb4000000`.
2. On return, it received `WM_ACTIVATEAPP` while still minimized.
3. It then received `WM_ACTIVATE` with wParam `0x00200001`: active status in the
   low word, nonzero minimized state in the high word. Microsoft's
   [WM_ACTIVATE reference](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-activate)
   defines these fields.
4. While handling that message, it called `SetDisplayMode(800,600,16)` (success),
   followed by `IDirectDrawSurface4::Restore` on its primary surface. The wrapper
   entered `RealPrimarySurface::restore`, returned `0x887600e1`
   (`DDERR_NOEXCLUSIVEMODE`), and the game immediately displayed the error dialog.
   Its main window was still minimized at the dialog call.

Read-only disassembly of the hash-verified North American 1.04 executable identifies
its window procedure at VA `0x00433620`. The WM_ACTIVATE jump-table entry dispatches
to `0x004336A8`, shared with WM_ACTIVATEAPP. The active branch checks the low word
of wParam and invokes an activation callback without first checking the minimized
high word. No executable bytes were changed.

This establishes that a restore attempt occurred before the window had been
restored in this reproduction. It does not yet prove which component should
correct the sequence or that a particular message hook will fix all return paths.
The next investigation should address activation/restoration ordering rather than
add filtering, CPU, texture-capability or driver-performance settings. It must
check keyboard Alt+Tab, taskbar return, menu and mission input, pause behavior and
normal exit. The packaged profile still has no validated focus-loss correction.

The diagnostic harness restored the byte-identical packaged profile and disabled
debug logging after exit. The launcher restored the original desktop mode. The
game's zero exit code did not mean that the focus-loss test succeeded.

On 2026-10-14, a separate private build of DDrawCompat 0.7.1 combined
`noactivateapp(0)` with suppression of the verified NA 1.04 game's `WM_ACTIVATE`
handler and retention of its borderless window instead of automatic minimization.
The user confirmed focus could leave the game and returning restored normal
rendering and mouse input. A keyboard screenshot taken while the game was focused
captured gameplay. However, the visible fullscreen image flickered while
unfocused. This is partial progress, not an accepted fix; background pause/input
and other return paths are not yet validated. The run exited with code 0 and
the launcher reported the original desktop mode restored (`AlreadyRestored`).

The second private revision skipped presentation while unfocused. The user
confirmed that this stopped the flicker and that automatic pause does not work.
The run exited cleanly with original desktop restoration reported. This is still
not a packaged or broadly validated fix. A third opt-in revision adds minimizing,
original-desktop restoration and reapplying the game display before window return;
its first run safely declined to arm. On 2026-10-15, a native display read showed
2560x1440@165 during gameplay despite the logged initial 1280x960@165. Upstream
`Win32/DisplayMode.cpp::installHooks` resets the physical display to registry
settings before installing display hooks; the defaults then present at desktop
resolution. The 1280x960 launcher step is therefore an initialization mode, not
proof of a persistent physical game mode. Revision 3b accepts the exact original
native mode or the known initialization mode and skips redundant changes. Its
short real test received an overall pass from the user on 2026-10-15. The log
confirms two minimize/return cycles and clean exit/desktop cleanup. Original and
game-native modes both measured 2560x1440@165, so no physical mode change was
required; the different-mode path remains unvalidated. Automatic pause is not
supported, and this short result on one system is not broad compatibility
validation. See the [focus source and profile notes](../third_party/DDrawCompat-focus/README.md)
for gates, source provenance and test procedure. No experimental DLL or setting
was promoted during those prototype tests. The 0.5.0-alpha candidate now includes
the exact passed DLL as a separately pinned, per-run opt-in. Stock baseline
defaults and the normal working installation remain unchanged. On 2026-10-16 the
fresh extracted package passed the user-confirmed baseline mission/save-load/quit
test and the optional focus-mode regression. Logs confirm two packaged focus
minimize/return cycles, clean exit, exact stock-wrapper/profile restoration and
removal of the pending swap record. Other hardware, different physical modes and
longer focus sessions remain separate requirements.

## Experiment summary

| Experiment | Result |
|---|---|
| Native 1.04 | Black screen, then access violation at `ds9dw.exe+0x8A08` |
| DDrawCompat defaults | Same crash; managed `R5G6B5` creation failed |
| Palettized emulation only | Selected `P8`, but malformed allocation still failed |
| Single-monitor enumeration only | Startup crash remained |
| Virtual 1280×720 | Startup crash remained; mouse issue was not fixed |
| POW2 capability patch | Startup, videos, and menu succeeded |
| Windows 640×480 compatibility flag | Mouse worked; Federation mission 1 completed |
| Temporary 1280×960 desktop | Mouse worked; Federation missions 1–2 completed |
| 1280×960 without D3D9On12 | Gameplay worked; AMD legacy-driver exit faults occurred |
| Add D3D9On12 | Dominion mission 1 ran; Quit detached cleanly with no crash event |

## Scope

The candidate launcher now has driver-tested narrow display fallbacks,
selectable graphics paths, launch diagnostics, an independent guarded recovery
helper, reversible local-profile reset, and a session lock. These changes
do not alter the POW2 texture workaround or establish new gameplay coverage.
Candidate 0.3 explicitly pins DDrawCompat's existing enabled, borderless,
application-aspect defaults. Narrow widescreen fallbacks rely on the reported
1600-pixel input threshold and aspect preservation; they are hypotheses awaiting
gameplay testing, not an additional proven configuration.
See [broader-hardware work](PORTABILITY.md) for evidence and remaining tests.

Multiplayer is not a release gate. GameSpy-era Internet service is outside the
current target, and LAN/direct-IP work is deferred. The current target is a
reversible original-media installation and stable single-player launch on modern
Windows systems.

## Research provenance

- [DDrawCompat](https://github.com/narzoul/DDrawCompat)
- [DDrawCompat configuration reference](https://github.com/narzoul/DDrawCompat/wiki/Configuration)
- [TrekCore Dominion Wars downloads](https://gaming.trekcore.com/dominionwars/downloads.html)
- [DxWnd Dominion Wars investigation](https://sourceforge.net/p/dxwnd/discussion/general/thread/364ab0f613/)

The historical Jiri Dvorak wrapper source was used as research provenance for the
POW2 workaround. Its binary and source are not distributed by this repository.
