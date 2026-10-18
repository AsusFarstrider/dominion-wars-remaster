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

The first accepted internal baseline is **int-20261018.1**, original-assets flavor,
from source revision `921826392c5b6f5acdc523f4db2429cbad234782`.
The [first experimental download](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/int-20261018.1)
uses this original-assets baseline. GitHub's automatic source ZIP is not a player
installation package. See [release notes](docs/RELEASE-NOTES.md).

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

| Area | Status in the first internal baseline |
|---|---|
| Resolution/interface | Experimental 960×600 logical 16:10 presentation and display-fit rendering; menu, fleet, loading and first-mission UI corrections |
| Startup and input | No forced desktop resize at startup; integrated input/focus handling |
| Gameplay smoke | Owner-observed first-mission/control, focus/capture, separate save/load and clean-exit checks passed on one system |
| Installation | Original-media plus official updater installation and verified-existing-install cloning tested |
| Textures/models/portraits | Original game assets only; private specialist experiments are not included |
| Broader restoration | In development/planned; no finished campaign restoration or universal hardware support |

The resolution branch measured 2304×1440 drawing on a 2560×1440 display. The combined
baseline passed a visual smoke, but its render targets were not independently
remeasured. This is **not full 16:9, arbitrary native UI resolution, or universally
working 1440p**. See [known limitations](docs/KNOWN-ISSUES.md).

## Downloading and testing

Download **dominion-wars-remaster-int-20261018.1.zip** from the
[experimental release assets](https://github.com/AsusFarstrider/dominion-wars-remaster/releases/tag/int-20261018.1),
then extract it fully. Use that player ZIP—not the repository source download.
See [installation](docs/INSTALL.md) and
[testing](docs/TESTING.md). You must supply legitimate North American original
media and the full official 1.04 updater, or a supported verified original/Modernizer
installation. No commercial game files, updater or DRM bypass are supplied.

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
This is not a blanket license for game-derived assets or private artwork. See
[publication audit](docs/PUBLICATION-AUDIT.md) for source/history review.
