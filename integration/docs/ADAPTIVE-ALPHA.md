# Dominion Wars Remastered — adaptive output Alpha

This early Alpha has very limited testing on one Windows 11 / AMD RX 6800 XT
computer. It is not a universal Windows/GPU compatibility claim or completed
remaster. Actual 1920×1080 and 2560×1440 primary displays were tested at DPI 96.
Other modes, aspect ratios, 4K, high DPI and other GPUs still need testing.

Download the player ZIP from the project's GitHub Release, not its source archive.
Extract it fully into a writable local folder. Double-click **Install Dominion
Wars Remaster.cmd**. Supply legitimate North American original media and the
official 1.04 updater, or select a supported existing original/Modernizer/remaster
installation. Choose a NEW destination to preserve your working game and saves.
The installer verifies original inputs and reconstructs the game and restored
movies locally. No game assets, original soundtrack, updater or complete modified
game executable are supplied. Original audio is reconstructed from owner files.

Launch with the installed shortcut or **Play Dominion Wars Remaster.cmd**. The
launcher selects your current primary physical display at desktop origin (0,0),
without changing its mode. A single engine adapts at startup; do not mix its
EXE, wrapper, profile or launcher with an older fixed-resolution installation.
There is no live resizing or monitor hotplug support. Pause before Alt-Tab.
Multiplayer is unsupported. The mission Esc → Main Menu repair and Preferences
responsiveness repair are included; final combined package validation is recorded
separately rather than inferred from these component results.

Use **Verify Dominion Wars Remaster.cmd** in the installed game to verify files.
Keep this extracted package for Repair/Remove through `Remaster.ps1`; run
`powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Remaster.ps1 -Action
Repair -Destination "C:\Games\Your Remaster"` from the extracted package, using
your actual destination. Replace `Repair` with `Remove` to reconstruct a compatible
original in that separate destination; the previous tree and saves are retained
in a sibling backup. These operations do not delete your original installation.

Logs: installed `.dw-remaster/runs`, `.dw-launcher`, and
`%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`. Review personal information
before sharing. Report the build/version, Windows version, GPU/driver, monitor
layout/mode/DPI, media edition and exact failure step. Never upload original game
files. Unsigned scripts can cause warnings; do not disable Windows security.

Master approves a specific immutable package and separate publication record.
Candidate/held metadata is not, by itself, release approval. Prior releases and
their checksums remain preserved.
