# 960x600 logical-width probe

Exploratory native 16:10 mode, not final 16:9 or a finished widescreen patch.
Independent game-wide960-v1 copy; no normal installation, working 4:3 preset,
texture branch or public installer changes. Original assets and tested recovery-
read fix. Exact-hash guarded modified exe and focus harness, no blanket hash bypass.

From exact original NA 1.04 exe, seven checked instruction operands change:
initial layout width; mode width global; window default width; two window/reset
caller widths; initial SetRect right edge; projection m11. Width 800 becomes 960,
height stays 600, scene-height deduction stays 154, and m11 is 2*800/960 (5/3).
The change aims to retain object proportions at wider viewport width while
expanding horizontal FOV. No global constant replacement or untraced 0.75 patch.
See wide960-patch-v1.json and build_wide960_probe.py for exact sites/bytes.

Existing layout 0x401860 computes width, scene width, half-width, scene height
and half-height; the known picking routine consumes those globals. Mode/layout
setup 0x429030 derives main viewport width/height from them. Other HUD/mouse/menu
code still contains fixed values; correct layout/input is NOT assumed.

Private profile keeps borderless, app aspect, display(-1) scaling and bilinear
readback, explicitly sets DisplayResolution=desktop and adds 960x600 to
SupportedResolutions alongside defaults. Local wrapper source requires an
advertised emulated mode and maps it through the native desktop override. This
does not create a physical GPU custom mode. Hypothesis on known 2560x1440 desktop:
logical target 960x600, main scene 960x446, scaled target 2304x1440. Must measure.

DLL is the previously tested bounded render-target observer
0E4790D24A030529DA62E3B8A98C5283E0953BBB926BDC7D9C8AA66C316CB566.
Only DW_RESOLUTION_TELEMETRY is enabled. No scratch passes, projection mutation
at runtime or original-buffer readback. At most the original 160 extra target/draw
records. Profile restored with exact guards; independent display watchdog remains.

Test gates:
1. Menus: pointer lines up, clicks work, no trails. Stop/quit if unusable.
2. Mission: HUD visible; note gaps, stretching, clipping or displaced controls.
3. Move/select in left, center and newly available right scene region. Test ship
   selection and ordinary HUD controls. Do not infer picking from cursor motion.
4. Zoom/rotate; screenshot if convenient. Alt-Tab out/back, check layout and input.
5. Quit normally. Inspect exit, desktop restoration, profile hashes and actual
   bind/draw dimensions before any next change.

Full-frame mission screenshot is particularly useful. Fixed-width UI may remain
left anchored or leave an unused right band; report it rather than treating that
alone as a crash. Severe menu/input trouble is a reason to stop the probe.
No success is claimed before user results and target evidence.

## Runtime result

Run 20261017T214501572Z: user reports wider mission, correctly working clicks,
menus/HUD shifted left. Screenshot saved. Bounded telemetry verifies logical
960x600 with successful indexed draws to 2304x1440, including 16 main-scene
viewport records. Normal exit and exact profile/desktop restoration confirmed.
This is a partial pass: wider scene works on this short test; UI placement is
unfinished. Alt-Tab in this variant was not explicitly reported. Next trace UI
canvas centering and hitboxes; do not move world picking along with the UI.
