# Help test Dominion Wars Remaster

Use the named **1.15 Alpha / int-20261030.1** player ZIP and the
[installation guide](INSTALL.md), not GitHub's source download. Keep a working
original and copied saves separate. Install each experiment into a new folder.

Start with a short single-player session:

1. Check opening picture/audio, natural endings and skips, Main buttons and
   **Remastered 1.15**. Distinguish naturally watched videos from skipped ones.
2. Check Preferences and returning to Main.
3. Try loading an existing save; also report fresh briefing/fleet/mission setup
   separately if you test it.
4. Check HUD/radar/selection, movement, zoom/camera, Space and control clicks.
5. Check mission **Esc → Main Menu** and returning to Preferences. These paths
   are repaired in 1.15, unlike 1.10; report any regression precisely.
6. Pause before Alt-Tab and test your screenshot/focus workflow if desired.
7. Report save/write/load and normal quit separately. A loaded existing save does
   not establish a new-save roundtrip.

These are requests for volunteer coverage, not statements that every path has
already passed. Do not resize, switch resolution or hotplug a monitor while the
game runs. Multiplayer is unsupported. Stop after corruption/repeated crashes;
retain logs instead of mixing files or repeatedly modifying the installation.
Don't run game tests from multiple development chats simultaneously.

## What is already known

One Windows 11 / AMD RX 6800 XT / three-monitor computer at 96 DPI has actual
1920×1080 and 2560×1440 component evidence. The current packaged human test was
1440p and passed opening/audio/version, Preferences, an existing-save mission,
HUD/radar/selection/camera, paused Alt-Tab, Esc → Main Menu and normal quit.
Game and launcher exited 0 with managed files verified. The automated UI attempt
was incomplete; this is a **human manual pass**, not an agent gameplay pass.

1080p evidence is inherited from byte-identical engine/provider/launcher/asset
components, not a newly run final 1080p package. Fresh-media extraction, new-save
writing and other focus cases have explicitly inherited coverage. No full campaign,
fresh fleet run, new-save roundtrip or exhaustive screenshot/Win+D matrix is claimed.

## Report success as well as failure

Use [GitHub Issues](https://github.com/AsusFarstrider/dominion-wars-remaster/issues/new/choose).
Include:

- Build/version (1.15 Alpha / int-20261030.1), or commit if known.
- Windows version/build, CPU, GPU and driver version.
- Physical screen resolution, scaling/DPI, monitor layout and primary display.
- Disc edition, exact steps, expected/actual result and what you skipped.
- Screenshots/logs if available; executable hash if useful and easy to obtain.

You don't need programming skills. A successful test on previously untested
hardware is valuable. Review personal paths/device identifiers before sharing logs.
Never upload original game files, private credentials or unreviewed saves.
See [limitations](KNOWN-ISSUES.md); passing admission bounds does not establish
support for 4K, other aspect ratios, high DPI or other GPU vendors.
