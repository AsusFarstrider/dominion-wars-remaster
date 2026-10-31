# Adaptive native output — private Master intake

October30,2026. Human opening-through-mission playtests passed at2560×1440 and
1920×1080 using IDENTICAL engine, wrapper, rendering configuration and Launcher
files. Both processes and managed launchers exited0 and verified installed files.
Each selected physical mode matched the observed engine screen, native QImage and
physical client at0,0. This is a functioning launch-time prototype, not universal
resolution/hardware certification. Current has not changed; no release authorized.

## Apply as a coordinated component

provider-source.patch is against actual approved production-runtime-source-v2
(manifest c992873d…4b84). Its13-file delta includes previously accepted ESC return
and Preferences performance fixes as well as8 modified and3 new adaptive files.
Do not apply the earlier ESC delta twice. Full frozen467-pin source/DLL/tests are
E:/Codex/research/dominion-rendering/dynamic-output-source-v1; manifest9dd1b04e…f7af9,
DLL7694f070…b720. component.json supplies complete hashes. Prior manifest sections
describe historical evidence; adaptive_output is the current source scope.

Apply all3 Launcher files from launcher-source/runtime together. The patch is
against actual int-20261029.1 installed Launcher source. V1 omitted the helper's
DE3-only guard and failed before game spawn; v2 corrects this with explicit
native-display-v1 + exact managed E81 admission, retaining legacy DE3. The original
failed run is preserved, not counted as a gameplay pass. V2 is a separate receipt;
it has not been retroactively inserted into the frozen provider v1 manifest.

Use exact NEUTRAL E81 engine, not DE3 with runtime overrides. Rebuild from legal
exact NA1.04 with the existing pinned repo builder:

    python rendering-research/native-resolution/build_engine.py --original ORIGINAL_EXE --output NEW_DIRECTORY --width 800 --height 600

Those dimensions retain original defaults and produce the same E81 every time;
the launch bridge binds the actual chosen dimensions in process memory. No
per-display EXE generation. The builder and accepted recenter emitter/data are
copied under engine-build-provenance with repository-relative paths. It expects
pefile (existing E:/Codex/tools/pydeps). Preserve the original repository placement
when reproducing; the emitter imports depend on it. No copyrighted EXE/artwork
is copied into this intake. Historical engine.json runtime_validated:false remains
unchanged; later live acceptance is explicitly separate.

Managed manifest: rendering.profile native-display-v1, OMIT rendering.width and
height entirely; engine.output_sha256 E81; wrapper hash as pinned. Runtime flat
Remaster.Rendering.json: matching profile/E81, current version.json SHA, recording
off, unchanged explicit asset permissions. Assets remain the approved baseline.
Regenerate new candidate version identity, runtime inventory, engine recipe,
provenance/package metadata and hashes. Do not promote the private installed
trial or silently reinterpret the released1.10 fixed-resolution package.

## Build and test

Provider MSBuild DDrawCompat.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
/p:PlatformToolset=v145. Installed compiler is VS18 BuildTools/v145, WindowsSDK
10.0.26100.0. BSD0 wrapper license and existing dependencies retained. Clean build
passed with no warnings/errors; exports match parent. Test projects use Win32.
For exact frozen-byte patch reproduction on Windows use
`git -c core.autocrlf=false -c core.whitespace=cr-at-eol apply PATCH`.
The source intentionally retains inherited mixed line endings; automatic CRLF
conversion changes source hashes. Both patches were applied in a disposable
source tree and all13 provider/3Launcher target byte hashes matched.
Pinned build/test commands and logs live under adaptive-validation. Scripts in
adaptive-validation-sources default to original isolated work paths; set equivalent
source/output locations when reproducing in a fresh workspace. Startup fixture
requires owned executable memory and an unrestricted test host; its literals were
mechanically relocated because host400000 was occupied, never forcibly unmapped.

Passed offline: startup245; runtime policy56; opening/HD532; HUD864; ESC480.
Eight simulated video sizes cover4:3,16:10,1366,1080,1440,4K,ultrawide. Launcher
v1 checks6Python/43WinPS; v2 bounded correction5Python/43WinPS/19focus admission.
Simulation is separate from the TWO actual live displays.

Human test request covered natural videos/audio, fitted menus and Preferences,
mission HUD/radar/world selection/camera, paused Alt-Tab and ESC → Main Menu →
Preferences, followed by normal quit. User reported all checks passed at both.
Evidence includes managed result/output selection, flushed selected provider
ledger, external state/window samples and explicit human acceptance. Samples59
and61; no capacity overflow or sampler errors. The initial0×0 client is retained
as startup window metadata rather than hidden. Sampling can miss events within
200ms; the selected128-event ledger is not a comprehensive recorder or GPU capture.

For a new candidate, repeat whole flow on actual1440 primary then1080 primary,
using same binaries. User manually changes and restores primary; no desktop mode
or layout changes by scripts. Verify physical/window/engine dimensions, file
fingerprints, normal exit, UI/input, audio and focus behavior. Further4:3/16:10,
1366,4K/highDPI/ultrawide require actual suitable hardware testing separately.

## Boundaries and removal

One immutable output size per launch, primary-origin0 only. No live resize,
hotplug, arbitrary secondary placement, aspect support guarantee or new UI artwork.
Source-sized Bink history is preserved and fitted only on presentation; source
2560×1440 asset identity checks are intentionally retained. Existing CPU menu/HUD
and inverse input paths are reused. Safety bounds800..8192 /600..8192 /64Mi pixels
are not a shipping support list. Selected GPU surfaces/descriptors do not establish
every rasterization stage; no comprehensive pixel-level claim is made.

The private trial is E:/Codex/remaster-build-tests/adaptive-output-20261030-01.
Its adaptive-parent and adaptive-launcher-v2-parent backups retain exact prior
files/metadata. For rollback, discard this disposable copy or adopt a new approved
Current copy; do not mix fixed/adaptive engine/profile/helper binaries. Original
saves are not rewritten by the component. Master owns supported installation,
uninstall, packaging and version decisions. Review CHECKSUMS.json, component.json,
source patches and live acceptance before creating a new integration candidate.
