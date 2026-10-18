# Guarded Windows-key recovery

October 18, 2026. Same input-window-v2 executable, graphics and no-switch driver.
Extends v1 minimization/Alt-release helper; no native DLL or game change.

Static handler evidence: callback 43B570 dispatches key down/up through tables
43B6C8 and 43B660. Shift=0x10, Ctrl=0x20, Alt=0x40, both Windows keys=0x80,
Apps=0x100 in DWORD 719618. The Win key-up branch 43B5D3 clears bit 0x80.
Shortcut matcher 43BE30 compares stored binding modifier flags (mask 0x1F0)
against this game modifier DWORD. A stale Win bit would reject plain Space;
Alt release alone cannot clear that bit. Runtime staleness not yet established.

Watch-NoResizeFocus-v2 reads that DWORD with read-only process rights on focus
return, after the existing physical-Alt-up and exact game HWND/hash checks.
It logs the mask. Only when bit 0x80 is set while both physical Windows keys are
up does it post local WM_KEYUP for LWin and RWin. Rechecks foreground and physical
Win state immediately before posting. No global input or game memory writes.
Uses keyup lParams C15B0001/C15C0001 (extended scan codes, release bits).
Still bounded to 32 focus-return recovery actions and stops with the game.

Observer additionally records modifier mask and physical Windows-key state;
user test must establish whether this fixes Space after Win+D/taskbar restore.
Screenshot hosts retain the v1 minimization exception. Three source/structure
tests pass and all new PowerShell scripts parse; runtime validation pending.

Active run: runs/no-resize-focus-v2-20261018T195419712Z, PID 12620.
Controller session 11522. Initial reader/helper error logs empty.
Four-minute safety exit remains active. No normal/public launcher changed.

User result: "It worked that time." Two Explorer/desktop focus cycles recorded.
Both returns logged modifier mask 0x80 while physical Windows keys were up,
then successfully queued the guarded game-only Windows-key releases. Observer
also recorded mask 0 with physical keys up. This supplies runtime evidence for
the stale Windows-key diagnosis, not just a coincidental positive user report.
Game exited normally with exit 0; launcher reports desktop AlreadyRestored.
Controller exit 0. No universal keyboard/focus validation implied.
