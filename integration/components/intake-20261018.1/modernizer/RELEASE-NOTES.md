# 0.6.0-alpha prerelease

Installer reliability update; existing game launchers, compatibility profiles and
graphics binaries are unchanged from 0.5.0-alpha.

- Explicit installation stages and approximately five-second activity reports.
  Hash/copy operations report measured bytes and elapsed time. Extraction reports
  observed output bytes; a sixty-second lack of byte changes is a warning, not
  a definitive hang diagnosis or an automatic timeout.
- Cabinet extraction runs without an inherited interactive console, with closed
  stdin and concurrent stdout/stderr capture. File work also runs independently
  of console output; selected console text can still pause visible reporting.
- Installer revision `console-guard-2` attempts to disable console QuickEdit
  during install/repair/removal, restoring the prior mode afterward while
  preserving later changes. Ctrl+C and global console preferences are untouched.
- Persistent install/repair/removal logs under
  `%LOCALAPPDATA%\Dominion Wars Modernizer\InstallLogs`, outside temporary staging.
- An explicit completion marker prevents an exit-zero interruption from being
  displayed as successful installation by `Install Dominion Wars.cmd`.
- Worker cleanup waits for owned I/O/helper termination before staging removal.
  The interactive official updater retains its normal prompts and is not killed
  automatically; close it to cancel that stage.

Forty-four new automated activity, entry-point and media-fixture checks passed,
along with existing launcher/focus/profile/shortcut and fixture lifecycle suites.
The cabinet test uses real private cabinet copies but placeholder resource files
and a substituted patch step; it is not an end-to-end official updater test.
The initial physical-CD candidate completed after Esc released a selection-related
pause near the shortcut stage. The new guard has seventeen additional automated
checks; its target-computer mouse-click/focus test passed, with continuous
heartbeats for 20.2 seconds and exact original-console-mode restoration. Mounted-image and
keyboard Ctrl+C candidate tests remain pending. See [TESTING.md](TESTING.md). No universal compatibility claim or
claim that the original stall's precise cause has been established is made.

The published 0.5.0-alpha package received a brief first-mission pass on Windows
10 / i5-3570K / GTX 670, but its physical-CD installer stalled and needed user
intervention. This is not a clean installer pass or longer-session validation.

---

# 0.5.0-alpha prerelease

At the original 0.5.0-alpha publication, testing covered one Windows 11 / AMD
RX 6800 XT / three-monitor configuration; Intel/NVIDIA testing had not yet been
performed. Later Windows 10 / Intel / NVIDIA results are recorded above and in
[TESTING.md](TESTING.md). This is not a universal Windows compatibility fix.

## Changes

- Separately selected **Play Dominion Wars - Experimental Alt-Tab.cmd** launcher.
  It uses the exact hash-pinned focus wrapper tested on the AMD system. The normal
  desktop shortcut retains stock DDrawCompat and the baseline behavior.
- Per-run DLL/profile swap with verified backups, atomic replacement, normal-exit
  restoration and next-launch recovery after interruption. Later changes/deletions
  are preserved rather than silently overwritten. Repair/removal recognize pending
  swaps, and both bundled DLLs are hash-checked before installation.
- Source patch, upstream license and build provenance for the optional wrapper.
- A new-system testing guide and a manifest-verified release builder. Candidate
  runtime files match the successfully tested package; final changes are documentation.

## Tested and still uncertain

The fresh extracted package's normal launcher passed the user's mission/save-load/
normal-quit check. The optional launcher received an overall pass; logs confirm
two minimize/return cycles, exit code 0, desktop cleanup, exact restoration of the
stock DLL/profile and removal of its recovery record. Automated launcher, focus
transaction, repair/removal, profile/recovery, visual-preset and shortcut checks passed.

Both original and game-native physical modes were 2560x1440@165, so real changes
between different physical resolutions remain untested. The launcher's 1280x960
mode is for initialization; stock DDrawCompat can restore the physical desktop
during startup. Individual screenshot shortcuts were not separately itemized in
the packaged pass. Longer focus-mode sessions, other GPUs and fresh CD installs
on other systems are the next tests. See [TESTING.md](TESTING.md).

## Using the candidate

Extract the complete release ZIP, supply a legitimate North American retail CD
**or a mounted image of your own CD**, plus the official North American 1.04 patch,
then run **Install Dominion
Wars.cmd**. Game assets, the official patch, modified game executables and DRM
bypasses are not included. No Git or development tools are needed.

The image must appear as a Windows CD/DVD drive. ISO or BIN/CUE can be used with
a suitable mounting tool; the modernizer does not mount images or read raw image
files. Any drive letter is accepted. Download the attached
`dominion-wars-modernizer-0.5.0-alpha.zip`, not GitHub's automatically generated
source archives.

Start with the normal desktop shortcut. After it works, try the separately named
experimental launcher in the installed game folder. **Pause manually before
switching away; automatic pause does not work.** The optional mode requires the
verified NA 1.04 executable and a driver-tested 1280x960 initialization mode, and
does not combine with visual presets. The normal baseline still has its known
Alt+Tab restore failure.

For the Intel/NVIDIA machine, follow [new-system testing](docs/NEW-SYSTEM-TESTING.md)
and preserve logs before another run overwrites them. The later Windows 10 /
Intel / NVIDIA installation/startup checks do not establish equivalent optional
focus-mode or extended-play coverage. Close the game before repair/removal;
back up intentional configuration edits before repair resets packaged files.
