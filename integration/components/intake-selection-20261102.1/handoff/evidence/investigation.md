# Launch-selected native output prototype

October 30, 2026. Private experimental work; Current remains int-20261029.1.

The prototype selects one physical primary-display size at launch. It keeps the
same engine EXE, provider DLL and profile for each session. It does not change
desktop modes or monitor arrangement, and does not support live resizing or
nonprimary monitor routing in this first pass.

## Source and architecture

Frozen provider: `E:/Codex/research/dominion-rendering/dynamic-output-source-v1`.
Manifest SHA256: `9dd1b04e08ebaffa6fbb2e39bbe22ddd0eed7866612c8a68795c01ef537f7af9`.
DLL SHA256: `7694f070e24b4e1f80b27356a1087c056c04d519a63bab54b12f19e17f72b720`.
Neutral engine SHA256: `e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023`.
Parent is accepted esc-menu-perf-source-v1, manifest e0933cd5…20d4, preserving
the tested ESC → Main Menu and Preferences performance repairs.

Launcher native-display-v1 queries the current primary physical mode, requires
origin0, records its device identity and full mode, and supplies canonical process
environment variables DW_REMASTER_OUTPUT_WIDTH/HEIGHT. It rechecks immediately
before spawning the existing lower launcher; it never writes machine/user env.

The wrapper validates exact managed engine/config/assets. Its checked entry hook
binds all15 reviewed width/height/projection operands before the original EXE CRT
and dimension-consuming constructors run. It modifies process memory only. It
validates the entire instruction recipe and all required page protections before
writes; a subsequent flush/protection/readback failure exits before original CRT.
One immutable session size is published only after success. Native mode validation,
existing runtime menu/HUD geometry and inverse input mapping use that same size.

The existing logical artwork/layout and CPU menu paths are retained. Bink keeps
full2560×1440 source history for the pinned HD movies, including partial updates.
Presentation is identity copying at equal dimensions, otherwise the existing fit
composer into the actual native target, preserving aspect ratio and margins.
Unlock publication and skip guards refer to committed target dimensions together.
This is not an extra game-rendering pipeline or an800×600 HD-video intermediate.

Bounds800..8192 by600..8192 and64Mi pixels are admission safety limits, not a list
of individually patched modes or a claim that every admitted mode is usable.
The full HUD requires the800×600 minimum. Odd widths are not rounded to a mode.

## Offline evidence

Clean Release/x86 build with installed v145 toolset; no warnings/errors. Original
export table preserved. Under adaptive-validation/results.json and hashed logs:

* 245 actual startup transaction/bridge assertions on owned synthetic PE pages,
  mechanically relocated to20000000 because the test host reserves400000.
* 56 managed identity/profile/feature/label assertions, retaining the legacy path.
* 532 actual opening controller/provider assertions, including source-history
  presentation at1024×768,1280×800,1366×768,1920×1080,1920×1200,2560×1440,
  3840×2160 and3440×1440.
* 864 existing combined HUD and480 ESC controller assertions against the new code.
* Launcher source6 Python and43 PowerShell contract checks.

These fixtures do not execute the complete game loader, Bink decoder, GPU or
physical displays. Source review by QUANTUM identified and corrected the initial
fixed-engine gate in OpeningFlow::installSafety before the final build.

## Live candidate and first observed failure

Disposable game: `E:/Codex/remaster-build-tests/adaptive-output-20261030-01`.
Build identity int-20261029.1-adaptive-output-01, experimental, nonpromotable.
Recorder OFF. A separate bounded read-only metadata sampler is available for
window/client/screen/native-image dimensions and state transitions; no game writes
or injected hooks. Its own process requests PMv2 awareness for physical client
measurements; it does not alter game DPI. A native-image descriptor does not by
itself prove all GPU rasterization/presentation stages.

First approved launch recorded physical2560×1440 at0,0 in output-selection.json,
but did NOT start the game. Existing Watch-RemasterFocus.ps1 rejected E81 because
it still required DE3. The helper-error.log captured the exact rejection; no
game PID, sampler, provider startup evidence or gameplay acceptance resulted.
The outer hidden launcher did not leave result.json for this failed preflight.
Launcher has been asked for a scoped new source revision and downstream gate
audit. Preserve this failed run rather than relabeling it a test pass.

Failed run: `.dw-remaster/runs/20261031T030504835Z-6d1b54a9e44c4b9dac691df94aa22294`.
The UTC date rolls over; the client local date is October30.

Launcher supplied a separately preserved v2 correction. Receipt SHA256
`274025aa3ca8d7e619f3904c292b8d54eaf3c66e438a2d0965102beb4423e1ac`;
Core `0db55a56d8014b2b5e7f8c1b4c1f7558bb1d8323503446232a40e6750186f12a`;
helper `78778adfe1740624fe52afccb075d670f4581c71b93027ae1c8b7477b5f2d812`.
Only explicit native-display-v1 plus exact managed E81 selects the new focus pin;
legacy remains DE3. All helper rechecks retain the selected exact identity. The
lower no-switch launcher/module had no further fixed-engine gates. Launcher
reports5 scoped Python,43 display and19 focus identity checks passed. Rendering
reviewed the focused patch, verified the source pins and applied it to the private
copy with updated runtime inventory, preserving the v1 parent and failed run.

The corrected launch DID start actual game PID27748. Its flushed provider ledger
records configuration validation, entry-hook installation, binding2560×1440 across
15 sites before EXE CRT, and functional_ready. The external sampler successfully
observed selected mode2560×1440, engine screen2560×1440, native QImage2560×1440,
and physical client2560×1440 at0,0 (sampler PMv2; game window DPI96). This proves
the real loader/startup binding and those observed dimensions on this run; it is
not yet human gameplay acceptance, proof of every GPU stage, or a1080 hardware pass.
Evidence: `.dw-remaster/output-evidence-20261031T031007167Z/events.ndjson`.
The human opening-through-mission check subsequently passed, with process and
managed launcher exit0 and installed-file verification. The sampler retained59
snapshots with no capacity overflow/errors. Selected ledger contained no refusal
or failure. See dynamic-output/live-2560x1440-01/acceptance.json.

The SAME8-file fingerprint (EXE, DLL, rendering config, version identity,3Launcher
files and DDraw configuration) then passed on a real1920×1080 monitor manually
made primary at0,0. Game PID12444 bound1920×1080 before CRT; selected mode, engine
screen, native image and physical client all matched. User reported all requested
checks passed and normal quit; process/launcher exit0 and managed verification
confirmed. Sampler retained61 snapshots, no capacity overflow/errors, and no
selected-ledger failure. See dynamic-output/live-1920x1080-01/acceptance.json.
Both game windows reported DPI96. The0×0 startup client observations are preserved.
The first two live modes are accepted; other physical modes/aspects/DPI and GPU
pixel-level behavior remain unverified. No full recorder was enabled.

## Real validation still required

First: real1440 primary cold launch; natural video/audio/skip, menus/modals,
mission HUD/radar/world picking/camera, paused Alt-Tab, ESC → Main Menu,
Preferences responsiveness, normal quit and managed-file verification.
Second: user manually makes an actual1080 monitor primary at0,0 and repeats with
the SAME EXE/DLL/profile, then restores their preferred primary. No automation of
desktop arrangement is included. Record physical, client and engine dimensions
separately and retain selected-mode/window evidence and actual human results.
Other aspect ratios, DPI arrangements, hardware and live changes remain unproven.

## Reproduction and ownership

Preparation, validation, source-freezing and private-overlay scripts are under
rendering-research/dynamic-output. The validator defaults to the isolated work
tree and calls MSBuild with PlatformToolset=v145; full checks need an unrestricted
test host because sandbox address reservations interfere with the startup fixture.
Freeze preserves parent source pins, test sources/logs, Launcher receipt and
QUANTUM/Resolution contract snapshots. Additional launch fixes must be separately
pinned; never rewrite the frozen v1 receipt to pretend they were included.

Rendering owns the early binding/provider/video fit; Launcher owns physical-mode
selection/focus guard changes; Resolution retains original layout/projection/input
contracts. Master owns integration/version/package decisions after live acceptance.
Do not publish or promote this experimental installation directly.
