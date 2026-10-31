# Adaptive focus identity fix v2 — source only

Fixes the demonstrated pre-game `Unexpected game image` failure. No Current,
game installation, active test, desktop, package, version or launch changes.
Previous adaptive-display-v1-source and receipt remain unchanged.

Use runtime/Remaster.Core.psm1 and runtime/Watch-RemasterFocus.ps1 as a pair in
Rendering's NEW managed disposable candidate. runtime/Launch-Remaster.ps1 is
included for completeness and is byte-identical to v1; do not add a second helper.
Register both changed managed-file hashes before ordinary verification/launch.

The focus helper keeps its DE3 default. Only explicit rendering.profile
native-display-v1 causes it to import the candidate's managed Core and resolve
the expected hash via Get-RemasterFocusExpectedHash. That pure selector requires
the exact canonical E81 engine.output_sha256; arbitrary hash or adaptive+DE3
declarations fail. Wrong/missing capability cannot admit an actual E81 binary.
Initial disk hash check and ALL ongoing window/EXE/repair/minimization checks use
the selected expected hash; no broad unconditional two-hash allowlist. No change
after the existing initial Get-FileHash gate in the helper.

Parent Test-RemasterInstalled remains responsible for actual full managed EXE,
Core/helper/DLL/profile/manifest verification BEFORE helper spawn. No manifest
trust or package-verification bypass was added. Old native-1440-v1 display logic,
800x600..8192x8192/64Mi-pixel adaptive bounds, process output pair, prelaunch mode
recheck, no-switch behavior, focus recovery and finally cleanup remain unchanged.

Downstream audit of the demonstrated adaptive-output-20261030-01 installation:
Launch-NoDesktopSwitch.ps1 has no fixed DE3/E81 EXE allowlist; its Width/Height
parameters are legacy unused request defaults because selection uses current
physical mode. The outer launcher does not pass adaptive dimensions into those
restricted legacy parameters; the process pair flows to the game instead.
DominionWarsLauncher.psm1 contains generic file/profile/recovery hash functions,
not a DE3-only engine gate. Rendering's native-display capability/stable E81
provider identity remains necessary; no runtime provider changes here.

Actual lower launcher SHA256:
6721CAEB4A403F9973036C143E71E8771F4D283E95738FDAA27872B5730476D3.
Unchanged module:
2AA093DBA8143932BCAE26E28004F5EDA72E1AA31DACA1130BDC098D424CB70F.

Checks performed without game/helper process launch or input/display actions:
43 inherited display checks;19 focus identity checks including execution of only
the actual helper's pre-Add-Type initialization under mocked file/module/hash
access;5 Python provenance/scope checks. Negative cases include E81 under1440,
unknown/no capability, adaptive+DE3, wrong disk hash and absent engine identity.
PowerShell parse checks and clean patch application are checked separately.
These do not constitute successful cold startup; Rendering controls retry.

Run from repository root:

    python -B -m unittest discover -s startup-research/adaptive-display-v2-source/tests -v
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File startup-research/adaptive-display-v2-source/tests/Test-AdaptiveDisplay.ps1
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File startup-research/adaptive-display-v2-source/tests/Test-FocusIdentity.ps1

upstream/ pins the v1 launcher/Core plus actual Current DE3 helper. changes.patch
is relative to that explicit source set, not an instruction to patch Current.
No source self-hash or managed hash is silently relaxed. Master owns integration.
