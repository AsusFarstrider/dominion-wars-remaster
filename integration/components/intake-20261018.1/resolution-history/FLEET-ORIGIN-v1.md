# Consolidated fleet UI-origin experiment

October 18, 2026. Based on the visually passed notification-origin candidate.
New independent game-fleet-origin-v1 copy. No launch/no-resize research changes.

## Source finding

Fleet callback 46F030 combines a widget-relative background with direct local
font/image drawing, wrapped text rectangles, and temporary local clip rectangles.
An early local EBP X is overwritten at 46F282, bypassing the previous initial
register correction. Rating-bar images already passed through shifted drawing
calls, but their clip left positions remained unshifted (six local rectangles).
46EDC0 computes ratings; it is not the drawing primitive and is left unchanged.

## Consolidation

137 direct calls within the confirmed callback now use four shared adapters:
hardware image, software image, direct font, and wrapped text. Each reads the
existing signed root UI X at 5F7EE0, rather than hardcoding 80. Font adapters
save the exact origin and font pointer, translate for drawing, then restore
local X while preserving text advancement. Wrapped text uses a translated
rectangle copy, leaving its caller's local rectangle untouched.

The background's two image calls are excluded: they already calculate an
absolute position from widget/parent coordinates. Global rendering APIs are
not hooked. Mission/world drawing is not routed through these adapters.

Seven prior literal corrections are reverted to local positions: initial
upper-left EBP, fleet heading, two large captain-image X arguments, and three
upper-left item-image arguments. Earlier middle and large-ship draw redirects
are superseded by the consolidated adapters. Their old code bytes remain
unreferenced rather than compacting the private PE section.

Ten known local clip-left stores use root-origin adapters: four upper-left item
clips and six rating-bar clips. Their Y, width, and height remain unchanged.
Adapters preserve incoming registers/flags and account for CALL stack changes.
There is no claimed single universal layout source for every game screen.

## Validation and test

Candidate SHA256:
12779AF0B0E41807D58EAFEC6D6724756F32D164268E3475DE79E4EA7D401EBE.
Six structural tests pass; all launch scripts parse.
Known-working 1280x960 startup switch, wide960 display-fit rendering, independent
desktop recovery, focus guard, and Alt-release helper are retained.

User test: fleet setup -> hover ships/captains and other item tabs -> inspect
top-left descriptions/icons/crew, central bar fills/labels/portraits -> mission.
Check that mission objective centering remains intact. Screenshot any remaining
offset or regression. Loading-bar source is separate and unchanged.

Launch through Run-FleetOriginTest-v1.ps1. Research copies only; public installer,
normal game installation, game assets, and original comparison builds untouched.

User visual result: fleet changes looked good; loading bar remains offset as
expected (outside this renderer). User additionally found ghost cursor images
in both side margins of the mission-selection screen (Federation/Dominion,
tutorial/skirmish/custom menu). This screenshot does not show fleet clipping
failure. Margin repaint on that separate screen remains to be mapped. Do not
claim every tab or damage/mission state validated from this single report.
