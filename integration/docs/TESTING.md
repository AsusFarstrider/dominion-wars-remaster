# Integrated regression and promotion gates

Build identity is the source revision, manifest hash and package checksum, not a
directory name. Branch test evidence is an input to integration, not a combined
test. Uncommitted development packages cannot be promoted.

## Automated checks on every candidate

1. Python tests cover exact delta round trips, unsupported inputs, malformed copy
   ranges/literals/output hashes and scope of the relocated native mouse hook.
   Check passed native stub body, imports and menu/loading bytes preserved.
2. Windows PowerShell5.1 core tests cover filesystem/hash/delta failure paths.
   All shipped PowerShell files must parse under5.1.
3. Build twice from one frozen revision and identical inputs; compare manifest,
   package bytes, checksums and combined executable. Verify ZIP members and CRC.
4. Install to a NEW directory from verified original/Modernizer data. Verify all
   immutable files and runtime hashes. Source files/saves/shortcuts stay unchanged.
5. Negative tests: unknown executable, changed package, existing/nested target,
   malformed delta and modified immutable base must fail without success markers.
6. Test save preservation, repair, new-destination update clone, removal to a
   compatible original, retained previous directory, and interrupted-commit recovery.
7. Test actual mounted NA disc + official updater through the ROOT remaster entry
   point. Do not count standalone Modernizer tests as this path's pass.

## Short repeatable runtime smoke (human observation required)

Use a separate installed candidate, known monitor arrangement and copied saves.
Deliver instructions BEFORE launch. No four-minute auto-close or observer kills.
Manual screenshots preferred; pausing hides the ship and automatic focus-pause
does not work. Note exact monitor/GPU/driver, Windows build and scale settings.

1. Launch from installed remaster shortcut. Confirm no launcher-applied low-mode
   resize. Menus respond under cursor, no persistent cursor trails or damaged art.
2. New Game -> first Federation fleet setup. Inspect tabs/rows/captain information,
   briefing, objectives, loading background/text/bar. One restart if convenient.
3. Mission: click ships, scene left/right/new margin, minimap, representative HUD
   buttons. Move/rotate/zoom including Space. Check selection/targeting alignment.
4. Alt-Tab away/back, then Win+D and restore through taskbar. Check mouse and Space
   after each. The USER performs Windows-key shortcuts; no global synthetic input.
5. Take screenshots with the user's chosen working tool. Check return/input.
6. Save, load, basic movement/combat and normal quit. Include one movie path and
   another relevant interface screen. Record anything not exercised as pending.
7. Confirm exit code, helper exit, desktop mode/recovery and runtime hashes.

Private Saber-art variant additionally checks natural component damage, near/far
views, repair and available debris, without changing gameplay stats. Do not invent
s-state thresholds or mark every state covered merely because all were installed.

Render-target telemetry is opt-in and bounded. Existing branch measurement proves
that branch's960x600 logical/2304x1440 drawing on the known display, not all combined
paths. If a new combined measurement is needed, make an identified diagnostic run
and retain successful target-bind/draw evidence; screenshot size alone is not it.

## Promotion

Promote-Current.ps1 requires a committed, verified build, the exact installed
manifest, a package-bound validation record, and explicit user acceptance. Every
listed gate must pass or have a documented USER-accepted limitation. It writes an
immutable private promotion record and Current pointer, preserving the previous
record/build. It does not modify source packages or authorize publication.

Until that gate is met, status remains integration-candidate, with no Current Build
pointer fabricated merely to satisfy a deliverable. Specialists continue existing
work; New-DevelopmentCopy.ps1 adopts an approved baseline only into a new directory.

Validation records use schema1, build_id, package_sha256, results keyed by
install_clone, install_media, startup_shutdown, rendering_ui_movies, mouse_keyboard,
focus_taskbar, screenshots, mission and save_load. Each contains status, evidence,
and where relevant reason/user_accepted. user_acceptance.evidence records the
human's exact decision, date and scope. Pending tests are not accepted limitations.
