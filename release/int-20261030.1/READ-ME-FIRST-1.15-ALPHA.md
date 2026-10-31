# Read this first — Dominion Wars Remastered 1.15 Alpha

**Early experimental release, not a finished remaster.** Build int-20261030.1 was
approved October 31, 2026 after a current packaged manual test and automated checks.
This external notice supplements the ZIP's historical candidate/pending status;
the tested ZIP and runtime are unchanged.

## What's new and what's tested

The launcher now selects the current primary physical display at desktop origin
(0,0), without changing its mode. Mission **Esc → Main Menu** and Preferences
responsiveness are repaired. Keep the matched engine, wrapper, profile and launcher
together; do not copy them into an older fixed-resolution install.

Actual 1920×1080 and 2560×1440 component trials cover one Windows 11 / AMD RX 6800 XT
computer at 96 DPI. The final packaged manual pass was 1440p; 1080p evidence is
inherited from matching components, not a fresh final-package run. Other resolutions,
aspect ratios, 4K, high DPI and GPU vendors are unverified. One size per launch:
no live resizing, resolution switching or monitor hotplug. Multiplayer is unsupported.

The current human check passed opening/audio/version, Preferences, mission
HUD/radar/selection/camera using an existing save, paused Alt-Tab, Esc → Main Menu,
Preferences return and normal quit. Game and launcher exited 0; files verified.
This is not a full campaign, fresh fleet setup, new-save roundtrip, new disc/updater
run or exhaustive input/display test. The agent UI attempt was incomplete; the
human completed the test. Restored buttons cache about 231 MiB in the 32-bit game.

## Download and install

1. Get **dominion-wars-remastered-1.15-alpha-int-20261030.1.zip** from
   [this release](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.15-alpha).
   Do not use GitHub's automatic source archive as an installer.
2. Keep your working game and back up saves. Supply legitimate North American
   retail media plus the full official 1.04 updater, or a supported verified
   original/Modernizer/remaster installation. Neither game nor updater is supplied.
3. Extract the whole ZIP into an ordinary writable local folder. Avoid cloud or
   junction/reparse paths; allow several GB for game copies, staging and backups.
4. Double-click **Install Dominion Wars Remaster.cmd**. Select a supported existing
   installation or original media/updater when requested. Complete any official
   patch wizard. CD drive letters are not fixed.
5. Choose a **NEW destination**, not your working installation. After verification
   succeeds, use the new desktop shortcut or **Play Dominion Wars Remaster.cmd**.

No programming, Git, Python or encoder is needed by players. Pause before Alt-Tab.
Keep legitimate media available where the original game requires it. Unsigned
scripts/legacy helpers may attract warnings; do not disable Windows security.

## Included visuals and ownership

The embedded Read Me says "No game assets." To clarify: **nine authorized replacement
visual payloads ARE included**, along with restored video deltas. There are no
complete original retail game EXEs/BOLT archives, original soundtrack/decoder,
official updater or disc images. Original audio is reconstructed from your verified
local movies. No no-CD or DRM bypass is provided.

Visual content is unchanged from 1.10 and has the creator's permission for project
distribution. That is not blanket MIT licensing of the art or ownership of original
logos, Star Trek or other third-party rights. MIT covers only newly project-authored
code/docs; DDrawCompat and other components retain their own licenses/notices.

## Verify or undo

Use **Verify Dominion Wars Remaster.cmd** inside the installed game. Keep the
extracted package for its Repair/Remove/Recover operations. Remove rebuilds a
compatible original in the separate destination, retaining the previous remaster
and saves in a sibling backup; it does not delete your original game.
See the [installation guide](https://github.com/AsusFarstrider/dominion-wars-remaster/blob/master/docs/INSTALL.md)
for exact PowerShell commands, recovery and log locations. Do not mix files or run
standalone Modernizer removal against remaster files.

Report successes and failures through
[GitHub Issues](https://github.com/AsusFarstrider/dominion-wars-remaster/issues/new/choose).
Include 1.15 Alpha / int-20261030.1, Windows, GPU/driver, monitor mode/layout/DPI,
disc edition and exact steps. Logs are installed `.dw-remaster/runs`, `.dw-launcher`
and `%LOCALAPPDATA%\Dominion Wars Remastered\Rendering`. Review private paths/device
identifiers before sharing; never upload original game files.

## Exact approved ZIP

182,161,058 bytes; SHA256:

`09b76f38aea263f13c9f596683968972a26f787b70ccc7b1ea9326f568eabd35`

Manifest SHA256:
`9bbb5a7d32e78a66ceb6647bff6157820f2f2357c860727305aaf2591e94e9cf`.

Original runtime source 8a32bcc36117442fffda89cd42580382f3305216; public mapped
source aa8e82ebd511f19aa94e7a3fd7d5ccb48d49aa75. Original/private and sanitized/public
provenance are distinct. Previous 1.10 and int-20261018.1 tags/downloads are unchanged.
