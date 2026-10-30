# Dominion Wars Remaster

A faithful fan restoration of *Star Trek: Deep Space Nine – Dominion Wars* (2001),
preserving its original QUANTUM engine, campaigns, gameplay and Star Trek atmosphere
while addressing presentation and technical limitations. This is not a remake.

> **Early Development — Experimental Builds**
>
> The remaster is barely underway. Many planned improvements are not implemented,
> and experimental builds may have visual problems, bugs or incomplete features.
> This is not a finished remaster. You're welcome to follow development, try test
> builds when published, and tell us what works—or doesn't—on your computer.

## Development status

The new experimental player build is **Remastered 1.10 Alpha**, integration
**int-20261029.1**. It targets a **2560×1440 primary display at desktop origin (0,0)
only**, without changing your desktop resolution. Other display modes are refused
before launch. Testing is limited to one Windows 11 / Radeon RX 6800 XT computer
with three monitors; other GPUs, drivers and Windows builds still need testing.

> **Known bug:** In a mission, do not choose **Esc → Main Menu**. It opens an old,
> top-left menu that does not respond to mouse clicks. Close and relaunch the game
> instead. This path is broken in this release; see [known issues](docs/KNOWN-ISSUES.md).

Get the [1.10 Alpha player download](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.10-alpha),
not GitHub's automatic source ZIP. See [release notes](docs/RELEASE-NOTES.md).
The [earlier original-assets experiment](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/int-20261018.1)
remains available separately and unchanged; it does not contain these restorations.

The separate [Dominion Wars Modernizer](https://github.com/AsusFarstrider/dominion-wars-modern)
installs and runs the original game on modern Windows. The remaster builds on that
foundation; its experimental improvements and testing are not Modernizer releases.

## What we're working toward

Preserve the original experience while removing limitations that prevent it from
looking and playing its best. Keep missions, mechanics, balance, recognizable UI
and satisfactory original artwork. Improve resolution/UI behavior, selected art,
ship geometry, effects, compatibility and modest quality-of-life issues where useful.
These are goals, not a list of finished features.

## Current progress

| Area | Status in 1.10 Alpha |
|---|---|
| Resolution/interface | Experimental native 2560×1440 rendering/UI/HUD assembly; original-density HUD artwork retained |
| Opening and Main artwork | Restored opening cards, three opening movies, Main background and six Main button banks included |
| Startup and input | Production launch with recording off, shared Remastered 1.10 label, no forced desktop resolution change |
| Gameplay | Owner approved the new recording-off trial with the Esc → Main Menu bug deferred; not a clean-exit or full-campaign pass |
| Installation | Verified media/official-patch or supported existing-install workflow; installs a separate copy |
| Ship textures/models/portraits | Not restored in this release; specialist experiments are not automatically included |
| Broader restoration | In development/planned; no finished campaign restoration or universal hardware support |

This is a limited native-1440p pilot, **not arbitrary-resolution support** or a
completed first-mission restoration. Restored buttons add about 231 MiB of cached
image data to a 32-bit game. Report memory problems, slowdowns or missing art.
See [known limitations](docs/KNOWN-ISSUES.md).

## Downloading and testing

Download **dominion-wars-remastered-1.10-alpha-int-20261029.1.zip** from the
[1.10 Alpha release assets](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/v1.10-alpha),
then extract it fully. Use that player ZIP—not the repository source download.
See [installation](docs/INSTALL.md) and
[testing](docs/TESTING.md). You must supply legitimate North American original
media and the full official 1.04 updater, or a supported verified original/Modernizer
installation. No complete game executable, original archive, original soundtrack,
official updater or DRM bypass is supplied. Restored movie video is included as
deltas; the original soundtrack is reconstructed from your verified local files.

Run **Install Dominion Wars Remaster.cmd**, choose a new destination, then use the
installed desktop shortcut. No PowerShell knowledge, Git, Python or encoder is
needed to install and play. Read the release's **READ-ME-FIRST-1.10-ALPHA.md**:
it supplements the ZIP's unchanged, creation-time internal-trial wording.

Keep your working original installation. The remaster installs to a separate new
directory; do not overlay a test build onto your only working game.

## Help us test

You don't need programming skills. Playing on another computer and reporting a
success or problem is useful. Include the build ID, Windows version, CPU/GPU,
driver, screen/monitor layout and what happened. Screenshots/logs help when available;
review them for personal information before sharing. Don't upload game files or saves
publicly. [Issues](https://github.com/AsusFarstrider/dominion-wars-remaster/issues/new/choose)
includes simple templates for bugs, graphics/resolution and feedback/successful tests.

## Roadmap and technical information

Our initial major milestone is a **complete, faithfully restored first Federation
mission**, including associated menus, UI, ships, environment and required assets.
There is no promised completion date. See [roadmap](docs/ROADMAP.md),
[integration architecture](integration/docs/ARCHITECTURE.md) and
[build instructions](integration/README.md). Historical technical notes contain
experimental results and are not promises about the public download.

## Credits and legal information

Unofficial fan project; not affiliated with or endorsed by the original game's
developers, publishers or Star Trek rights holders. Rights in the game and Star
Trek remain with their respective owners. See [credits/distribution](docs/CREDITS.md).
Newly project-authored integration/build/install/release code is MIT; existing
third-party licenses remain applicable. See [scope and exceptions](docs/LICENSING.md).
Restored visuals are distributed with the creator's permission for this project,
not under a blanket MIT artwork license. Original game and trademark rights are
not transferred. See
[publication audit](docs/PUBLICATION-AUDIT.md) for source/history review.
