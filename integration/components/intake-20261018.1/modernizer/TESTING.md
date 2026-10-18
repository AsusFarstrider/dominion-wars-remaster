# Development validation

## Candidate 0.6.0-alpha installer reliability (2026-10-16)

Installer-only changes leave every game-managed launcher, profile and graphics
binary unchanged. The new `payload/DominionWarsInstall.psm1` is used by the
installer, not copied into the game. It uses .NET background file workers and a
no-console native helper with closed stdin and concurrently drained stdout/stderr.
Measured progress is reported roughly every five seconds; sixty seconds of no
observed byte change produces a warning, not automatic termination or a claim
that the operation is healthy. Persistent run/helper logs live outside staging.

Windows PowerShell 5.1 automated validation includes streaming hash/copy results,
large dual-stream output, argument quoting, console isolation/stdin EOF, quiet
and failed helpers, missing/empty/stale extraction output, and cancellation that
joins the owned child while preserving an unrelated helper. Entry-point tests
cover copied-source install success, error logs, explicit completion markers and
the actual batch entry's rejection of an exit-zero/no-marker result. These
cancellation checks use a cancellation token through the same worker-finally
cleanup path, not a physical keyboard Ctrl+C test.

New suites passed: **22 activity checks, 11 entry-point checks and 11 media
reconstruction checks**. Launcher/parser (44 checks including new-file parsing),
focus transactions (16), recovery/profile (24), visual presets (32), shortcuts
(8), dummy-process orchestration and the existing fixture lifecycle also passed.

The real pinned i6comp helper is tested against private copies of the owner's
three original cabinet files. Resource files in this reconstruction fixture are
placeholders, and the interactive updater is explicitly substituted with a copy
of the owner's verified 1.04 executable. This exercises all five extractions,
retail executable verification, successful staging commit, failed-patch cleanup,
failed-extraction cleanup and corrupt-cabinet rejection. It is not a playable
installation, an official patch UI test or a physical-CD timing test. The legacy
32-bit helper requires execution outside this development sandbox; the sandbox
rejects it with an Illegal System DLL Relocation error before startup.

The existing 21-managed-file fixture install/repair/removal checks also pass.
Mounted-image end-to-end runs, physical Ctrl+C behavior and actual slow/read-failing
media still need manual testing; the 0.6.0-alpha release retains these limitations.

### Physical-CD result and console guard revision 2

The user returned collected installation logs from the Windows 10 / GTX 670
computer. The original 0.6.0-alpha candidate verified all three cabinets, copied
532,770,184 resource bytes, completed all five isolated extractions, ran the
official patch, verified the resulting game, installed compatibility files and
ended with a successful final installation result. This run used physical media
but reads were very fast, likely benefiting from cache; it is not a cold-disc
performance measurement. No gameplay logs were included in this collection.

The user clicked in the window, observed a pause while compatibility setup was
displayed, and Esc released it. Timestamps place a 73.4-second gap between the
shortcut-stage announcement and final success. The compatibility-file stage
itself advanced to the shortcut-stage announcement after about 1.7 seconds.
This strongly supports selection-blocked console output/main-thread progress;
no live console-selection flag or blocked stack was captured.

Installer revision `console-guard-2` attempts to disable QuickEdit only for the
current input console before the first installer message. All other mode bits,
including processed Ctrl+C input, remain intact. The original mode is restored
in finally, unless later console-mode changes are detected; redirected input is
a no-op, and failed mode/restoration calls are recorded. No registry or global
console defaults are changed. Seventeen additional guard policy/backend checks
cover restoration, no-console/already-disabled paths, API failures and preserving
later changes. Simulated policy tests are not a substitute for actual mouse
selection on the target Console Host. The revision's mouse-click behavior was
subsequently tested as recorded below; no second full installation was needed
for that focused check.
`tests/Test-InstallerConsoleInteractive.ps1` exercises the same guard and logs a
twenty-second timer without touching the game, display or registry. Its automated
two-second smoke run used redirected input.

The user then reported "Test went perfectly" on the Windows 10 computer. The
returned console-click log independently records QuickEdit changing from
`0x1F7` to `0x1B7` (only bit `0x40` cleared), twenty approximately one-second
heartbeats, completion after 20.2 seconds and exact original-mode restoration.
Together with the user's report, this is a pass for the requested mouse-click /
focus-switch regression on that console. The timer alone does not identify which
individual mouse/focus actions were performed. It does not validate every
terminal host, keyboard Mark mode, forced process closure or keyboard Ctrl+C
cancellation, nor is it a second full installation of the guard revision.

### Second-system report for published 0.5.0-alpha

The user reported a physical-CD install and approximately one minute in the first
mission on Windows 10 Home build 19045, i5-3570K and GeForce GTX 670 with driver
30.0.14.7514. The install appeared silent after media detection. Read-only SSH
diagnostics found the resource copy complete (approximately 533 MB), while the
first extractor was waiting with no recorded file I/O. Detached usage and cabinet
listing probes returned promptly. The user pressed **Ctrl+C**, not Esc, after
which installation continued and finished. The exact cause remains unconfirmed;
console selection/input/output blockage is a hypothesis, not an established
diagnosis. The user subsequently recalled accidentally clicking inside the
console, making selection-related blockage more plausible; no selection state
was captured before Ctrl+C. The installed executable independently matched the official 1.04 hash.
This is a gameplay-startup smoke pass, not a clean installer or campaign-stability
pass. No remote configuration changes were made during diagnosis.

## Candidate 0.5.0-alpha integration

The exact revision 3b focus DLL is now a separately hash-pinned optional payload,
selected by `Play Dominion Wars - Experimental Alt-Tab.cmd` or
`-FocusMode Experimental`. Normal launches keep stock DDrawCompat and clear inherited
experimental opt-ins from the child environment; the caller's process environment
is restored afterward. No DLL rebuild or game-executable change was made.

Windows PowerShell 5.1 automated checks passed for the 16 focus transaction cases,
normal launcher/orchestration, 24 recovery/profile cases, 32 visual-preset cases,
eight shortcut cases and a 21-file fixture install/repair/removal. New cases cover
interrupted-focus repair/removal, optional DLL tamper rejection, conflict/deletion
preservation, backup/path checks, read-only experimental diagnostics, option mixing
and child-environment isolation. The lifecycle fixture is not playable. These
checks do not establish real-game success of the packaged launcher.

On 2026-10-16 the user confirmed the fresh extracted candidate's normal launcher
reached a mission, saved/loaded and quit normally. The process exited with code 0
and cleanup reported `AlreadyRestored`; its elapsed lifetime was approximately
67 minutes, not proof of continuous active play throughout that time.

The packaged optional focus launcher then received an overall pass for the
requested minimize/return, input/capture and normal-quit sequence. Its log
independently records two minimize/return cycles, approximately 44 seconds of
process lifetime, exit code 0, desktop cleanup and stock-wrapper/profile recovery.
Both restored file hashes exactly match the baseline and `focus-session.json`
was removed. The user did not separately itemize each requested check or screenshot
shortcut. Original and game-native modes both remained 2560x1440@165; actual
different-resolution switching was not exercised.

These tests used a separate playable installation from the extracted package,
with the owner's existing verified 1.04 tree as source; they were not a new
original-CD installation. The regular working installation and desktop shortcut
were unchanged. Tested ZIP SHA-256:
`F7FC2AF91B7CDD51461A6F0C62D94E87EA035867BDA57B9E4EDE4EB3492549C4`.
The publication candidate adds updated documentation/release notes only; its
runtime files must remain byte-identical to that tested archive.

Clean Intel/NVIDIA installer retesting, actual different-resolution transitions and
longer focus-mode sessions remain pending; see [new-system testing](docs/NEW-SYSTEM-TESTING.md).

## Installed stability safeguards and 1024×768 check (2026-10-13)

This project is now being tested for compatibility and stability only; visual
and asset-remaster work is outside the current test scope. Existing experimental
presets remain in the source, but were not enabled for this run.

The tested launcher/restoration safeguards were deployed through Repair to the
working installation, with all eighteen managed components verified against the
source payload. The North American 1.04 executable, release DDrawCompat 0.7.1 DLL,
and exact baseline profile hashes remained unchanged. The installed shortcut was
not altered; prior managed files and installation state were copied to local
evidence before repair. No updated release was published.

A run explicitly requested 1024×768 with resolution fallback disabled and
`GraphicsBackend = Profile`, retaining `ForceD3D9On12 = on`. The launcher selected
1024×768 at 165 Hz from an original 2560×1440 at 165 Hz. The user reported that a
mission launched and all seemed good. This is a brief functional check, not
long-session, save/load, or mission-transition validation, and it does not test
automatic failover when 1280×960 is missing on another display.

The launcher returned exit code 0 and reported `AlreadyRestored`. The harness
confirmed original width, height, refresh, color depth, orientation and position,
and an unchanged baseline profile. Evidence stays under the working game's
`.dw-launcher/testing/stability-fallback-20261013-0209014b56f546e1b3625536cd4bfe64`.

The same baseline/backend then passed a brief explicitly selected 800×600 at
165 Hz run. The user again reported successful mission launch and normal
operation. Exit code was 0, desktop restoration was recorded as `AlreadyRestored`,
all seven saved desktop fields matched afterward, and the baseline profile was
unchanged. Evidence is in the `800x600` subdirectory of the same local folder.
This adds a smaller-mode functional result, not long-session validation or proof
that automatic failover works on a monitor lacking the preferred modes.

## System-default D3D9 check: mission worked, process faulted (2026-10-13)

The alternate `Native` backend was tested at 1280×960 at 165 Hz, changing only
`ForceD3D9On12` to `off` from the baseline rendering recipe. This selects the
system-default D3D9 implementation, not a universally forced native driver.
Windows 11 Home build 26300 and RX 6800 XT driver 32.0.21045.5002 were recorded.

The user reported successful mission launch and normal-looking operation, and a
rendered ship/mission HUD was observed. However, the process ended with
`-1073741819` (`0xC0000005`). Windows Application Error event 1000 identifies
`AMDXN32.DLL` version 32.0.21045.5002 at offset `0x00BECC8E`, matching game PID
8180 (`0x1FF4`). The wrapper log confirms `ForceD3D9On12 = off` and has no
successful detach record. A short mission appearing to work is therefore not a
stability pass. This is consistent with the earlier AMD native-path fault, not
evidence that another GPU vendor's default D3D9 implementation must fail.

Desktop cleanup completed as `AlreadyRestored`; all seven captured desktop
fields matched afterward. The test harness restored the exact known-good profile
from its hash-verified backup after the game ended. Evidence stays in the
`native1280` subdirectory of the local testing folder above. D3D9On12 remains the
default on the known AMD configuration; no automatic crash retry, vendor heuristic
or game-executable modification was added.

A subsequent longer baseline run was launched with `Auto`, the restored profile,
1280×960 and no optional visual preset. The actual main menu and game PID 9756
were verified. That run failed during save loading as recorded below;
stdout/stderr are preserved in the `extended1280` evidence subdirectory.

## Fresh AutoSave reload failed on the baseline (2026-10-13)

The user saved to AutoSave during the baseline run and then tried to load it;
the game crashed. It was a newly written save, not just an old-edition or old-
backend save being carried into this test. The process returned `-1073741819`
(`0xC0000005`). Application Error event 1000 identifies `ds9dw.exe+0x1DCE8`,
matching PID 9756. This is distinct from the alternate backend's AMD DLL fault.

The preserved minidump locates the fault at `0x0041DCE8`, instruction
`test byte ptr [esi+4], 0x80`, with ESI `0x36E27338` and an invalid read at
`0x36E2733C`. That proves an invalid-pointer read in the game, not the reason
the pointer became invalid. Save corruption, state teardown/reload handling and
wrapper interactions have not yet been separated.

The wrapper recorded successful detach despite the fatal process exit, and the
desktop was already restored. The exact baseline profile remained unchanged.
The dump, AutoSave.sav (61,754 bytes), the other existing save and analysis output
were preserved locally under `extended1280/save-crash`. These files contain game
data and must not be included in the repository or public issue attachments.
Save/load stability is a failed requirement; no longer-session pass is claimed.

A fresh baseline process was launched to load the unchanged AutoSave from the
main menu without first starting a mission or writing another save. The user
reported a clean load; PID 3164 later returned exit code 0 with desktop cleanup
recorded as `AlreadyRestored`. The save still matches the preserved failing-run
copy, SHA-256 `2D9802F43AE5DFAE04795BEA895E6DD6052552FB7ED168229A98E4AF7948AEAD`.
This argues against a consistently unreadable save file, but does not establish
why the earlier same-session save/reload sequence failed. It is not a fix.

An unchanged baseline process (PID 2976) was then launched to load the same save
from the main menu and reload it from inside the mission without an intervening
save. This separates repeated mission-state loading from writing the save file;
the user reported that both loads worked. PID 2976 then exited with code 0 and
desktop cleanup recorded `AlreadyRestored`. The AutoSave hash and baseline
profile were unchanged. Evidence is under `save-crash/repeat-load-1`. Repeated
loading alone did not reproduce the original fault in this run.

The next baseline process (PID 12864) was launched to load AutoSave, create a new
named slot and immediately reload that new slot, without overwriting AutoSave.
Existing saves were copied to `save-crash/new-slot-1/before-saves` before launch.
This investigates the write/reload step; it is not yet evidence of an overwrite-
specific bug. The user reported that it all worked. The new `DWTEST01.sav` was
81,905 bytes, AutoSave remained unchanged, and PID 12864 exited with code 0 and
desktop cleanup recorded `AlreadyRestored`. Saves and final logs are preserved
under `save-crash/new-slot-1`.

The original-style new-mission / overwrite-AutoSave / immediate-reload sequence
was then requested on another unchanged baseline run. The user reported no crash;
PID 23920 exited with code 0 and desktop cleanup recorded `AlreadyRestored`.
AutoSave changed to 56,328 bytes, SHA-256
`0663ABA3BB0D6D7B85717F13749136CDE1E0D9E3A475BE7E47C6C3713FCE9B68`, confirming
an overwrite occurred. Before/after saves and final logs are preserved under
`save-crash/autosave-overwrite-1`. The exact baseline profile hash remained
unchanged throughout. The original crash remains unresolved and was not
reproduced by these controls; no fix or deterministic overwrite bug is claimed.

Further read-only inspection of the original dump identifies window-procedure
arguments with message `0x0202` (`WM_LBUTTONUP`), wParam 0 and lParam `0x01E00280`
(client coordinates 640,480). The stack includes game return addresses
`0x00433991` and `0x0041DF9E`. Disassembly shows an indirect UI-handler call and
a recursive object traversal with rectangle/flag tests and callbacks before the
fault. This makes UI callback lifetime/reentrancy a research lead; it does not
prove a use-after-free, a double-click problem, or the function that invalidated
the pointer. No input/focus workaround or binary change has been applied.

## Stability and real desktop recovery checks (2026-10-10)

Ordinary launcher cleanup now shares the independent helper's guarded restoration
operation. Dummy-process orchestration checks confirm later display adjustments
and primary-monitor changes are preserved, normal exit restores the temporary
mode, and rejected switches do not reapply an already-original desktop. An
unknown recovery phase is rejected rather than treated as permission to restore.

The opt-in real-display test passed on the same Windows 11 build 26300 / RX 6800
XT configuration described below. It confirmed a real 1280×960 at 165 Hz switch,
then deliberately killed only a disposable worker. The independent helper
completed with `AlreadyRestored`; the desktop had returned before its restoration
decision. A separate synthetic orphan retained the dead worker's parent identity
and used the controller to establish the temporary mode. The shipped helper then
completed with `Restore`, returning to 2560×1440 at 165 Hz. Width, height, refresh,
color depth, orientation, position and primary monitor identity were checked.

Two earlier harness attempts rejected an otherwise-complete `AlreadyRestored`
record: the first required `Restore`, and the second did not retain a process
handle before observing the non-child helper's exit. Those are harness failures,
not helper passes or gameplay failures. The successful run retains the process
handle and distinguishes natural parent termination from synthetic orphan recovery.

Evidence remains local in `.test-real-recovery-5581c552472c4de0ad3bc26caa4b7b60`.
No game executable, saves, installed shortcut, or installed compatibility profile
was changed. This validates controlled launcher/desktop recovery on one system,
not recovery from every gameplay/driver crash, helper death, reboot, or power loss.

Post-change Windows PowerShell 5.1 regressions passed: 36 launcher/parser checks,
full dummy-process orchestration, 24 recovery/profile checks, 32 visual-preset
checks, eight shortcut checks and eighteen-file fixture installation/repair/
removal. The recovery/profile suite needed normal child-process file access
outside the sandbox; its sandbox-denied attempt is not counted as a pass.
At the time of these checks, the source changes were not deployed or published.
They were installed locally on 2026-10-13 as recorded above; the existing 0.4
candidate ZIP still predates this hardening.

## Candidate 0.4 real baseline test (2026-10-10)

The user authorized treating the working installation and saves as disposable.
The hash-verified 0.4 candidate was installed in place; the desktop shortcut was
left unchanged. Windows 11 Home build 26300, RX 6800 XT driver 32.0.21045.5002,
three displays, and a Virtual Desktop Monitor adapter were present. The launcher
selected temporary 1280×960 at 165 Hz from an original 2560×1440 at 165 Hz.

The game ran for approximately 4 minutes 52 seconds. The user reported everything
seemed to work except Alt+Tab: restoring the game produced
`Error with Screen RestoreSurface() on primary: DDERR_NOEXCLUSIVEMODE`.
The actual fleet-setup screen and rendered ship were observed. This report does
not independently confirm each requested save/load or gameplay exercise.

The game returned exit code 0 and DDrawCompat detached successfully; the launcher
reported restoring the original desktop mode. These cleanup results do **not**
mean the Alt+Tab test passed. Info-level graphics logging did not capture the
failing API call sequence. The error text is user-reported.

After preserving final logs and the exact baseline profile, the disposable
installation alone was changed to test `AltTabFix = noactivateapp(1)`. This
setting is supported by the bundled 0.7.1 source and suppresses DirectDraw's
activation handling while still delivering focus notifications to the game.
The subsequent run with `noactivateapp(1)` did not repeat the original error,
but the user reported a transparent/non-redrawing game window and mouse confinement
in an upper-left region on return. It failed the Alt+Tab test. Launcher exit code
was 1; the original desktop mode was restored. The mechanism of termination is
unknown, so exit code 1 is not treated as evidence of a Windows exception.

The next isolated run used `noactivateapp(0)`, suppressing the game's focus
notifications too. The user reported the same transparent/non-redrawing window
and confined-mouse symptom. This run lasted approximately 44 seconds, exited
with code 1, and the launcher reported desktop restoration. Neither notification-
suppression variant is a working Alt+Tab fix.

The `keepvidmem(1)` retry also failed: restoring the game produced the original
`DDERR_NOEXCLUSIVEMODE` error. The actual mission was visibly rendering before
the test. The retry lasted approximately 29 seconds, exited with code 0, detached
successfully and restored the desktop. None of these three settings is an
accepted Alt+Tab fix. The original release DLL and graphics backend were retained.

A subsequent baseline run changed only `LogLevel = debug`, using the same release
DLL. It captured the reported error in the primary surface's Restore call while
the game window was still minimized, inside `WM_ACTIVATE` handling. The trace
and read-only executable inspection narrow the next investigation to restoration/
activation ordering; see [focus-loss findings](docs/COMPATIBILITY.md#alt-tab-failure-investigation).
The diagnostic harness restored the exact packaged baseline profile on exit;
debug logging is disabled. This approximately 58-second diagnostic run also exited
with code 0 and restored the desktop, despite the visible error.

Packaged defaults remain unchanged. There is no validated packaged message-order
correction. Visual presets and forced-interruption recovery remain untested in
actual gameplay.

## Private focus experiment (2026-10-14)

A separate game copy and opt-in custom DDrawCompat 0.7.1 build suppressed the
verified NA 1.04 game's `WM_ACTIVATE` handling, used `noactivateapp(0)`, and kept
the borderless image visible instead of automatically minimizing. On the same
Windows 11 / RX 6800 XT / three-monitor system, the user reported:

- Alt+Tab released focus, permitting interaction with other monitors.
- Returning restored normal rendering and mouse input; flickering stopped.
- A keyboard screenshot captured gameplay while the game was focused. The exact
  shortcut was not independently established; unfocused capture was not tested.
- The fullscreen image flickered while unfocused and did not minimize.

The archived launcher recorded exit code 0, unchanged private profile and original
desktop restoration (`AlreadyRestored`). The flicker means this is only partial
success. Automatic pause, taskbar/minimized return, repeated transitions and
other systems remain unvalidated. Pause manually before switching away.

Revision 2 held the last presented frame while unfocused. The user confirmed
that flickering stopped and separately tested that automatic pause does not work;
manual pause is required. Its run exited with code 0 and the launcher reported
original desktop restoration. This does not validate every return or capture path.

Revision 3 adds opt-in minimization and saved-desktop switching, including
reactivation and taskbar restore handling. Native mode snapshot and comparison
checks initially passed (19 checks, no display changes). Its first run safely
declined to arm: stock DDrawCompat had reset the physical display to its original
2560x1440@165 during startup. Exit and desktop cleanup completed normally.
Revision 3b accepts the exact original mode as well as the launcher initialization
mode, records the actual game-native target and skips redundant display changes.
Its expanded checks passed (23 checks, no display changes).

On 2026-10-15, the user reported the requested revision 3b test worked. The wrapper
log confirms two minimize/return cycles, reactivation after minimization and
presentation resuming. The approximately 38-second process exited with code 0,
the private profile was unchanged, and cleanup reported `AlreadyRestored`.
Original and game-native modes both measured 2560x1440@165, so these successful
cycles required no actual display-driver mode change. Different-mode switching
is not validated by this result. Individual taskbar, input and screenshot checks
were not separately itemized in the user's overall pass report. Automatic pause
remains unsupported. Longer play, repeated transitions and other hardware still
need testing. Exact passed DLL SHA-256:
`3AA11B1C81A21C6EF5FB85D2DC31DEEC7EB8217F637094E9ED7B9F6D6BC90490`.
Source and harness
are documented in [the focus experiment](experiments/focus-guard/README.md), not
included in the release manifest. The normal installation and packaged DLL/profile
remain unchanged.

## Candidate 0.4 preset checks (2026-10-09)

The optional visual/resource-saving presets have not been gameplay-tested.
All 32 visual-preset checks passed in Windows PowerShell 5.1: each preset isolates
one enhancement, resets other preset controls, preserves core fixes and backend,
restores exact original bytes, keeps later edits and original backups, rejects
replacement of a pending record, and rejects a tampered backup.

Full launcher orchestration with fake display operations and a real dummy child
passed per-run Sharper cleanup, diagnostic capture of applied settings, Enhanced
3D cleanup on display rejection, Resource-saving cleanup on process-start failure,
configure-only rejection, read-only diagnostics with an interrupted preset, and
next-launch recovery before an ordinary baseline run.

A non-playable fixture passed installation, repair and removal of all eighteen
managed files, including the four preset command wrappers. The user's working
installation/shortcut/saves were not changed; no real game or display switch was
used for these checks. See [graphics experiments](docs/GRAPHICS.md) for the later
comparison plan. Automatic success does not establish a visual/performance win.

## Candidate 0.3 safety and portability checks (2026-10-09)

Gameplay testing is deliberately deferred. No real game launch or physical
display-mode change was used for these new checks, and the user's working game,
saves, and real desktop shortcut have not been upgraded.

Windows PowerShell 5.1 automatic checks cover:

- requested/fallback display ordering, narrow widescreen/current-mode candidates,
  missing modes, refresh preferences, deduplication, and strict no-fallback behavior;
- launcher orchestration, exit propagation, restoration after simulated failures,
  competing-launcher rejection, and startup without module-discovered hash/CIM commands;
- the normal launcher's automatic system-default D3D9 selection when DX12 is
  unavailable, preservation of saved alternatives, and explicit-backend behavior;
- helper readiness and completion using a disposable, never-armed recovery record;
  pure recovery-policy tests for live processes, replaced monitors, later mode/
  refresh/position changes, and already-restored or unused records;
- native display snapshot JSON round-trip and confirmation that the current
  real mode remains unchanged;
- local-profile reset matching the packaged recipe, backups of conflicting
  profiles, byte-exact undo, repeated-reset rejection, and refusal to overwrite
  edits made after reset;
- installation/repair/removal of all fourteen managed files in a non-playable
  fixture, tampered-DLL rejection, and locked-file rejection before replacement;
- shortcut ownership and byte-exact restoration on a fake desktop, preservation
  of edited shortcuts, and targeted running-game mutation guards.

At this stage on 2026-10-09, actual display restoration after parent death had
not yet been validated; the later 2026-10-10 checks above add that evidence.
Its policy and process handshake were tested separately;
the helper test requires normal local process/file access outside the Codex
execution sandbox. A sandbox file-replacement denial was not treated as a
successful helper run.

See [portability work](docs/PORTABILITY.md) for implemented wins, intentionally
deferred guesses, reproducible developer checks, and the later gameplay matrix.

## Candidate portability changes (2026-10-09)

The 0.2.0-alpha launcher candidate has passed automatic mode-selection, profile
preservation/backend selection, and restoration-order checks on Windows
PowerShell 5.1. Full launcher orchestration tests use fake display operations and
a small test process: diagnostics leave the profile/display unchanged; child exit
code 37 propagates; rejected mode and executable-start failures attempt restore;
unsupported explicit D3D9On12 is rejected before a switch; and a competing
launcher in a separate process is blocked.

A disposable fixture passed installation of all nine managed files, repair of a
corrupted launcher module, rejection of a tampered DDrawCompat DLL before repair
writes, and removal while preserving the executable and logs. A backend-modified
profile was retained, and an original profile backup was restored on removal.
The fixture contains structural placeholders, not a playable reconstructed game.

Read-only real hardware diagnostics passed outside the execution sandbox:
Windows 11 Home build 26300, RX 6800 XT driver 32.0.21045.5002, a Virtual Desktop
Monitor adapter also present, three screens, primary 2560×1440 at 165 Hz. All 60
enumerated candidate records passed the driver's mode test. No mode was changed
and no game was launched. The sandbox denied CIM inventory and mode tests, which
was separately verified to be an execution restriction rather than hardware failure.

This newer OS/driver combination and the updated launcher have **not** received
the extended gameplay validation described below. See [portability work](docs/PORTABILITY.md)
for the test matrix and reproducible test commands.

### Real candidate startup and exit

A separate playable candidate was reconstructed from the clean 1.04 baseline,
with a copy of the user's existing save. The original working installation,
desktop shortcut, and saves were not altered.

The first attempt failed before switching the display because `Get-FileHash`
was unavailable in the actual startup environment. Launcher and installer hash
calculation now use .NET SHA-256 rather than relying on discovery of that command.
A separate-process regression with an empty `PSModulePath` and failing legacy
hash/CIM command stubs covers this failure even when a host preloads modules.

After repairing the candidate, the real game ran for about 52 seconds using the
packaged profile and 1280×960 at 165 Hz. The log showed `P8`, `R5G6B5`, `D16`, and
vertex-buffer resources; the game exited with code 0, DDrawCompat detached
successfully, and the launcher reported restoration of the saved desktop mode.
The user said it seemed to work correctly. No mission completion, save/load, or
extended gameplay validation is inferred from this short test.

## Original gameplay baseline

Test date: 2026-07-19

Environment: Windows 11 build 10.0.26200, AMD Radeon RX 6800 XT, three displays,
original North American retail media mounted as a CD-ROM drive.

## Updater lifecycle

A disposable verified 1.04 fixture passed:

1. in-place installation;
2. backup of a pre-existing compatibility configuration;
3. deliberate corruption of the installed configuration;
4. repair back to the packaged SHA-256;
5. modernizer removal;
6. restoration of the pre-existing file;
7. removal of files that did not exist before installation;
8. preservation of the game executable and data.

## Original-media reconstruction

The development updater reconstructed a new game tree directly from mounted
original media using the hash-verified bundled i6comp v0.20 extractor. It launched
the user-supplied official 1.04 patch, waited for its normal UI to complete, and
verified the patched executable as:

`03C20A3535D0209111FF9F3B90EBA403E059DF2BE7E784D2746ABF1535C1BE6A`

Every game file in the result was then compared by relative path, size, and
SHA-256 with the independently preserved clean 1.04 baseline:

- expected files: 31
- reconstructed game files: 31
- differences: 0
- result: PASS

Repair and modernizer removal also passed against this full reconstruction, and
the underlying game tree remained installed.

## Packaged launcher

The first packaged-launch test exposed that `$PSScriptRoot` was unavailable while
PowerShell evaluated a parameter default. Path resolution was moved into the
launcher body and the installed test copy was repaired. The repeated test then:

- resolved `ds9dw.exe` relative to the installed launcher;
- started the game through the packaged DDrawCompat profile;
- created the expected `P8`, `R5G6B5`, and `D16` resources;
- returned exit code 0; and
- ended with `DDrawCompat detached successfully`.

The user confirmed that the packaged build reached the main menu and exited via
Quit. Windows recorded no `ds9dw.exe` application fault, and the primary desktop
was restored to its original 2560x1440 mode.

The command wrappers preserve the underlying PowerShell exit code so a displayed
error cannot be mistaken for a successful automated run.

## Longer gameplay session

A later continuous session ran for approximately 56 minutes. The user completed
the first three missions in sequence and played the fourth until defeat. The
session produced no new Windows application-error event and ended with
`DDrawCompat detached successfully` at 22:55:04. Rendering resources remained on
the expected `P8`, `R5G6B5`, `D16`, and vertex-buffer paths in the captured log.

## Path independence

With no media path supplied, mounted-media discovery selected a valid Dominion
Wars CD-ROM source. An explicit path can override automatic selection. A text
scan found no development workspace path embedded in the package. The default
destination is resolved at runtime from the current user's `LOCALAPPDATA` folder.

## Bundled extractor validation

The bundled i6comp v0.20 executable is checked before use against SHA-256
`1181AE0DADA52EA5A5990D11F966E5986BF0A01614D2D5E5AD63B7C79E29FA29`.
The package includes the source-header authorization and credits for fOSSiL and
Morlac. A modified or substituted extractor is rejected before execution.
