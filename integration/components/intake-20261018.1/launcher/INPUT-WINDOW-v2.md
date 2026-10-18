# Window-derived mouse recentering

October 18, 2026. Private NA1.04 candidate, based on the prior exact no-switch
control; no public launcher or installer changed. Separate workspace game copies.

V1 queried 713DE8 as an HWND. Static follow-up proved this was a color-depth
value copied from display-object+0x10, not a window handle. GetClientRect failed
and the guarded patch retained the previous dimensions. User reported unchanged
behavior. Its bounded observer closed the test automatically.

V2 uses A387A4, display-object+0x34: CreateWindowExA's return is stored there by
4A65D9, and existing GetClientRect calls use that same field. Hook at 49E5EE
preserves flags and all registers, queries GetClientRect into a local RECT,
checks positive signed-WORD-sized dimensions, updates only AB9EE4/AB9EE6, restores
state, replays the displaced cache loads, and resumes the original recentering.
No global GetSystemMetrics spoof or fixed 1280x960 replacement. API failure
retains the existing dimensions. No desktop mode is applied by private launcher.

V2 executable SHA256:
FB972D8FBABD6DCC929E78F74A8C3C158EA2F8A8A66DBC040291E643A9E91CD7.
Six v2 structural tests pass; unittest also discovers the five imported v1 tests
(eleven executions, not eleven distinct v2 cases). PowerShell parse checks pass.

Runtime trace:
runs/input-window-v2-20261018T193351010Z/observation/input-samples.json.
Cached dimensions now 960x600, recenter (480,300), logical cursor changed from
(205,537) to (451,518) in observed samples. This is positive evidence the patch
executes and the logical cursor moves. User reports: "Seems to be working well.
No issues noted." This is a positive interactive smoke pass; exact mission,
Alt-Tab, screenshot and normal-exit coverage was not specified. Do not call
the overall startup problem universally fixed yet. At acknowledgment, the
launcher log still showed the game active, so final cleanup remains pending.

Safety observer closes only the exact PID/path/start-time instance at 150
seconds. Driver retains recovery, profile restoration, Alt-release helper, and
the same graphics profile/executable changes unrelated to this patch.
Active controller session at launch: 1431. Check cleanup after exit.

Limitation: original SetCursorPos uses screen coordinates while this patch
supplies client dimensions. Known input-window origin is (0,0). Handling
nonzero-origin/moved windows and focus changes remains unvalidated. UI offsets
and other resolution-branch work are deliberately unchanged.
