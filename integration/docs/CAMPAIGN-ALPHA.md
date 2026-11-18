# Dominion Wars Remastered 1.20 Alpha

This Alpha restores the mission-selection screen, fits Federation Briefing and
Fleet Setup to a wide display, and improves mission transitions and some HUD
labels. The new artwork is **work in progress**. We welcome contributions that
improve it while preserving the original game's style. Please share only art you
made or have permission to share on the project's GitHub issue tracker.

Testing is limited to one Windows 11 / AMD Radeon RX 6800 XT system. Individual
components have different test coverage; do not assume every feature was tested
at both 1920×1080 and 2560×1440. Other GPUs, high DPI, 4K and other monitor layouts
may behave differently. Multiplayer is unsupported.

1. Download the player ZIP, not GitHub's source archive. Extract it fully.
2. Double-click **Install Dominion Wars Remaster.cmd**. Supply legitimate North
   American retail media and the official 1.04 updater, or select a supported
   existing original, Modernizer or remaster installation.
3. Choose a new destination to preserve your working installation and saves.
4. Start the game from the installed shortcut or **Play Dominion Wars Remaster.cmd**.

The package contains replacement artwork and tools, not the original game,
official patch, full modified game executable, or original soundtrack. The
installer reconstructs the game and applicable movie audio from owner files.
Fleet uses the game's own palettes; no palette extraction is needed.

The launcher uses the current primary display at desktop origin (0,0) without
changing its mode. It does not support resizing or monitor changes while running.
Pause before Alt-Tab. Use **Verify Dominion Wars Remaster.cmd** in the installed
folder to check files. Keep the extracted package for Repair and Remove:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action Repair -Destination "C:\Games\Your Remaster"
```

Replace `Repair` with `Remove` to restore a compatible original in that separate
destination. The previous tree and saves are kept in a sibling backup. Use your
actual destination path. Neither action removes the original installation.

Logs are in the installed `.dw-remaster/runs` and `.dw-launcher` folders and under
`%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`. Report the exact failure step,
build/version, edition, Windows version, GPU/driver, resolution and monitor layout.
Review logs for personal information and do not upload original game files.
Unsigned scripts may cause warnings; do not disable Windows security.
