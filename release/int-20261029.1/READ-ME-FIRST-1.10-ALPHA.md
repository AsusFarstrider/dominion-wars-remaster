# Read this first — Dominion Wars Remastered 1.10 Alpha

This is a very early experimental remaster, not a finished game. The owner approved
the exact int-20261029.1 package on October 29, 2026 with the bug below deferred.
**This notice supersedes only the ZIP's historical "internal trial / unapproved /
held" status wording.** Its actual runtime, checksums and limitations are unchanged.

## Important bug and display requirement

- **Do not select Esc → Main Menu during a mission.** It opens an old menu in the
  top-left that ignores mouse clicks. Close and relaunch instead. Save first where
  possible. If already stuck, try Alt+F4 or end only ds9dw.exe in Task Manager;
  unsaved progress may be lost. This release does not fix that route.
- Your primary display must be physical **2560×1440**, at desktop origin **(0,0)**.
  Other primary modes are refused; the launcher does not resize your desktop.
  This is not a 1080p/4K or arbitrary-resolution release.
- Validated only through limited testing on one Windows 11 / AMD RX 6800 XT
  computer with three monitors. Multiplayer is unsupported; avoid its menu button.
  No full-campaign, all-GPU or clean-shutdown pass is claimed.

## Download and install

1. Download **dominion-wars-remastered-1.10-alpha-int-20261029.1.zip** from
   [this release](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.10-alpha).
   Do not use GitHub's automatic source ZIP as an installer.
2. Keep your working original game and back up saves. You need legitimate North
   American original media plus the full official 1.04 updater, or a supported
   verified original/Modernizer installation. Neither the game nor updater is supplied.
3. Extract the ZIP fully to an ordinary writable local folder. Avoid cloud/reparse
   directories. Allow several GB for game copies, staging and retained backups.
4. Double-click **Install Dominion Wars Remaster.cmd**. Select a supported existing
   installation or original media and official patch when requested. Complete any
   official patch wizard prompts. No fixed CD drive letter is required.
5. Choose a **new destination**, not your only working game. After verification
   succeeds, launch with the new desktop shortcut or **Play Dominion Wars Remaster.cmd**.

No Git, Python, compiler or movie encoder is needed by players. Restored visuals
are included; original audio is reconstructed from your verified files. Original
game/Star Trek/logo rights are not transferred. Newly authored project code has
scoped MIT licensing; DDrawCompat and other components retain their own licenses.
Unsigned scripts or legacy helpers can trigger security warnings; do not disable
Windows security to install this alpha.

## Verify or undo

Use **Verify Dominion Wars Remaster.cmd** in the installed game. Keep the extracted
package for its PowerShell Repair/Remove operations. Remove rebuilds a compatible
original in the separate installation and retains the previous remaster/saves in
a sibling backup; it does not delete your original game. The
[installation guide](https://github.com/AsusFarstrider/dominion-wars-remaster/blob/master/docs/INSTALL.md)
provides exact commands and recovery/log locations. Do not mix EXEs/wrappers or run
the standalone Modernizer's removal against these remaster files.

Report successes as well as bugs through
[GitHub Issues](https://github.com/AsusFarstrider/dominion-wars-remaster/issues/new/choose).
Include build int-20261029.1 / 1.10 Alpha, Windows, GPU/driver, monitor layout/mode,
disc edition, what you did and where it failed. Logs are in the installed
`.dw-remaster/runs`, `.dw-launcher` and the renderer's
`%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`. Review personal information
before sharing; never upload original game files.

## Exact immutable player ZIP

182,134,278 bytes; SHA256:

`fdd3f3ee6bc619bcc841324ec7d3841fc6188379904301590293714de4a007ee`

Original source f025d0cfaacaa1c28964792424ee7b77f37ace97; manifest SHA256
b0319fd0a248d8862e2641ddc8eb03218632fbfd4d63a2384ff1203827bed43d.
This ZIP is not the failed int-20261027.1 trial. The older int-20261018.1 public
experiment stays separate and unchanged.
