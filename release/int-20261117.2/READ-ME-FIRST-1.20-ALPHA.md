# Dominion Wars Remastered 1.20 Alpha

Experimental prerelease int-20261117.2, approved for publication by the project
owner after Master's technical sign-off. This notice supersedes creation-time
candidate wording inside the unchanged ZIP.

This update revisits the campaign interface: faction selection and cursor art,
widescreen Federation briefing and Fleet Setup, a larger Captain roster, and
improved presentation for results, objectives and hover labels. Dominion and
4:3 briefing retain their earlier layout. The new artwork is work in progress.

## Install and play

1. Download the named player ZIP, not GitHub's source archive.
2. Keep your original installation and back up saves. You need legitimate North
   American retail media and the full official 1.04 updater, or a supported
   verified original, Modernizer or remaster install.
3. Extract the ZIP to a writable local folder and double-click **Install Dominion
   Wars Remaster.cmd**. Choose a new destination. Launch from the new shortcut or
   **Play Dominion Wars Remaster.cmd** after verification succeeds.

Use the primary physical display at desktop origin (0,0). One size is picked at
startup. No live resizing or monitor hotplug is supported. Multiplayer remains
unsupported. One live test crashed while switching away with Alt-Tab during
active gameplay. Pause before switching away as a precaution, not a verified
fix. Existing component tests are not proof that every resolution,
campaign or setup works. Use **Verify Dominion Wars Remaster.cmd** to check the
installed files. Keep the extracted package for its repair and removal commands.
Removal restores a compatible original in the separate install and retains the
previous files and saves in a sibling backup.

The download includes replacement visuals, not the complete original game or
official updater. Original movie audio and Fleet palette data come from the
player's verified game files; no separate extraction step is needed.

## Test coverage and known limitation

On the exact candidate at 1440p, the player reached Results from a late autosave
and reported that Exit worked. Earlier in the smoke test, the candidate reached
gameplay after faction selection, Briefing and Fleet Setup, including the eighth
Captain entry. These are bounded checks, not a clean full mission from start to
finish. A separate fresh 1080p run of the exact package exercised the updated
map, all four faction label states, Briefing and confirmation, and exited
normally with managed files verified. The 1440p screenshot run also exited
normally. These checks do not validate every interaction or a full campaign.

The earlier active-gameplay Alt-Tab crash logged an access violation
(`0xC0000005`) in AMD's `amdxc32.dll` on the test system. That identifies where
the crash surfaced, not its root cause. Managed files verified afterward.
A separate attempt to change display modes while the game was running hung
and required force-closing. Change resolution only after quitting.

The exact ZIP was independently reproduced, and all 34 packaged lifecycle
checks passed. No universal Windows, GPU or display compatibility is claimed.

## Exact package

`dominion-wars-remastered-1.20-alpha-int-20261117.2.zip`

SHA-256: `7cfeaeae6cfc6110e686c6c555b6f80162795f37d18a00394ff1daabdc931b07`.
