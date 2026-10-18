# Loading-origin validation on the passed menu repaint

Candidate EA1EA1B8E580265D304C3905199951BE2288E9FC87BB9DAE503D23191C29139B.
Independent game-selection-loading-v3; launch Run-SelectionLoadingTest-v3.ps1.

The corrected menu repaint passed visually in v2 and is byte-identical here.
Three additional anchors in direct progress updater 434560 add existing signed
root UI X: background parent origin at 434569, text origin at 4345A3, and bar
origin at 434625. The latter also supplies its clipping rectangle's X.
Normal parent-relative loading callback 434700 and stored widget X unchanged.

Four additional structural checks pass, including exact equality of the passed
menu stub and unchanged normal loading callback. PowerShell launch scripts
parse; exact executable, profile, and focus-tool hashes pinned as usual.
Known working startup resize, Alt-release workaround, and recovery retained.
No cross-thread launch changes or asset changes incorporated.

User test: mission start and restart, inspect loading background/text/bar
centering. User reported everything looked good. Earlier failed v1 combined candidate is NOT
used; v3 is built from fleet-origin-v1 with corrected repaint explicitly enabled.

Run selection-loading-v3-20261018T200532198Z-scale2-bilinear-readback-displayfit-16x10
completed with launcher exit 0, profile restored, assets/wrapper unchanged,
and desktop AlreadyRestored. This is a short visual pass on the known test
configuration, not exhaustive screen or hardware validation.
