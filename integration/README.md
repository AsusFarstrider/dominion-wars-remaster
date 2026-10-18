# Dominion Wars Remaster integration pipeline

Master-owned internal source/build/install tooling. Early experimental work, not
a published remaster or approved Current Build. See the root
MASTER-RELEASE-HANDOFF.md for Master/Release responsibilities.

Source root: this existing shared workspace, with integration code under
integration/. No new Git repository or remote is created. Existing specialist
directories are not staged or overwritten by this tooling.

## Build

Developer requirements: Python3.12 (tested3.12.14), pefile2024.8.26, Git for source identity, owned
verified NA1.04 original data, and the frozen component intake. Windows player
packages need only Windows PowerShell5.1, legitimate media/patch or a supported
existing installation; no developer environment.

```powershell
.\integration\Build-Remaster.ps1 -OriginalGame 'E:\Codex\research\installed-1.04' -Output 'E:\Codex\remaster-builds\new-build' -PythonPath '<Python3.12 executable>'
```

Output must be new. Normal builds require a committed, clean integration source
revision. -Development permits a precisely content-hashed working snapshot for
implementation tests, marks it non-promotable, and is not a release shortcut.
Optional -PrivateAssetDelta supplies the exact private Saber delta identified in
source-lock.json; otherwise no replacement assets are included. Public clearance
is NOT inferred from that flag. No complete game exe/archive enters the ZIP.

Initial source intake was built with tools/capture_inputs.py from identified
branch artifacts. It refuses existing intake, pins532 source/dependency files,
fetches Modernizer from an exact commit, records canonical original input hashes,
and creates resource-aware private art deltas outside source. Normal builds do
NOT recapture mutable branch work. New specialist milestones need a reviewed new
intake/revision, not editing the old snapshot or changing pins to hide a mismatch.

```powershell
python -m unittest discover -s integration/tests -v
powershell.exe -NoProfile -ExecutionPolicy Bypass -File integration/tests/Test-Core.ps1
```

Original game fixture path defaults to the known research original; another
environment sets DW_TEST_ORIGINAL_GAME. Historical branch tests remain captured
as provenance and may need their original fixtures; new integrated checks do not
use mutable branch executables. Rebuild twice and compare package checksums before
promotion. Build record includes composition scope, original/output hashes and ZIP
identity; package manifest identifies runtime/source/dependency versions.
Python and zlib versions are recorded in the manifest. Use those same versions
when reproducing ZIP bytes. Git attributes preserve frozen input bytes rather
than converting line endings during checkout.

## Install, validate and hand off

See docs/INSTALL.md for the actual player prototype, docs/ARCHITECTURE.md for
architecture, docs/INVENTORY.md for branch intake, docs/TESTING.md for regression
and promotion gates, and docs/DISTRIBUTION.md for exclusions/licensing questions.

Current build ID convention is internal int-YYYYMMDD.N, not a public alpha version.
This candidate is int-20261018.1. A successful compile or install does not approve
it as a development baseline. Promote-Current.ps1 requires actual validation plus
human acceptance; it preserves the previous baseline and does not authorize public
release. New-DevelopmentCopy.ps1 only makes NEW copies of an approved Current.

Immutable build handoffs go under handoffs/master/<build-id>/, referencing private
artifacts by absolute path/hash. Release does not independently reassemble branch
work and Master does not publish a competing GitHub repository/release pipeline.
