[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$VerifiedGame)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$scratch = Join-Path $root ('.test-lifecycle-' + [guid]::NewGuid().ToString('N'))
$fixture = Join-Path $scratch 'Game with spaces'
New-Item -ItemType Directory -Path $fixture -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $VerifiedGame 'ds9dw.exe') -Destination $fixture
# Structural fixture, not a playable installation; no game data is bundled.
foreach ($name in @('binkw32.dll','rs\Bolts\ds9blt.blt','rs\Bolts\ds9dat.blt',
    'rs\Bolts\ds9media.blt','rs\Missions\shipcapt.mis','rs\Video\IntroDS9.bik')) {
    $path = Join-Path $fixture $name
    New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
    New-Item -ItemType File -Path $path | Out-Null
}
$modernizer = Join-Path $root 'DominionWarsModernizer.ps1'
& $modernizer Install -SourcePath $fixture -Destination $fixture -NoDesktopShortcut
$state = Get-Content -LiteralPath (Join-Path $fixture '.dw-modernizer\state.json') -Raw | ConvertFrom-Json
if ($state.ManagedFiles.Count -ne 21) { throw 'Expected all twenty-one managed files.' }
foreach ($record in $state.ManagedFiles) {
    if ((Get-FileHash -LiteralPath (Join-Path $fixture $record.Name)).Hash -ne $record.InstalledHash) { throw "Install mismatch: $($record.Name)" }
}
# Replace a new module to confirm repair installs its exact packaged bytes.
Copy-Item -LiteralPath (Join-Path $fixture 'Play Dominion Wars.cmd') -Destination (Join-Path $fixture 'DominionWarsLauncher.psm1') -Force
& $modernizer Repair -Destination $fixture
if ((Get-FileHash -LiteralPath (Join-Path $fixture 'DominionWarsLauncher.psm1')).Hash -ne
    (Get-FileHash -LiteralPath (Join-Path $root 'payload\DominionWarsLauncher.psm1')).Hash) { throw 'Module repair failed.' }
Import-Module (Join-Path $root 'payload\DominionWarsFocus.psm1') -Force
$null=Start-DominionWarsFocusSession $fixture
& $modernizer Repair -Destination $fixture -NoDesktopShortcut
if ((Test-Path -LiteralPath (Join-Path $fixture '.dw-launcher\focus-session.json')) -or
    (Get-FileHash -LiteralPath (Join-Path $fixture 'ddraw.dll')).Hash -ne
    (Get-FileHash -LiteralPath (Join-Path $root 'payload\ddraw.dll')).Hash) { throw 'Repair did not recover the interrupted focus session.' }
Write-Host 'PASS: repair recovers interrupted focus session and returns stock wrapper.'
# A tampered dependency must be rejected before an otherwise valid repair writes
# any installed compatibility files. Alter only a disposable package copy.
$brokenPackage = Join-Path $scratch 'Broken package'
New-Item -ItemType Directory -Path $brokenPackage | Out-Null
Copy-Item -LiteralPath $modernizer -Destination $brokenPackage
Copy-Item -LiteralPath (Join-Path $root 'payload') -Destination $brokenPackage -Recurse
$dll = Join-Path $brokenPackage 'payload\ddraw.dll'
Copy-Item -LiteralPath (Join-Path $root 'payload\Play Dominion Wars.cmd') -Destination $dll -Force
$installedProfile = Join-Path $fixture 'DDrawCompat-ds9dw.ini'
$beforeRepairHash = (Get-FileHash -LiteralPath $installedProfile).Hash
$rejected = $false
try { & (Join-Path $brokenPackage 'DominionWarsModernizer.ps1') Repair -Destination $fixture }
catch { $rejected = $_.Exception.Message -match 'unexpected SHA-256' }
if (-not $rejected -or (Get-FileHash -LiteralPath $installedProfile).Hash -ne $beforeRepairHash) {
    throw 'Tampered payload was not rejected safely.'
}
Write-Host 'PASS: tampered DDrawCompat rejected before repair writes.'
Copy-Item -LiteralPath (Join-Path $root 'payload\ddraw.dll') -Destination $dll -Force
Copy-Item -LiteralPath (Join-Path $root 'payload\Play Dominion Wars.cmd') -Destination (Join-Path $brokenPackage 'payload\DDrawCompat-Focus.dll') -Force
$rejected=$false
try { & (Join-Path $brokenPackage 'DominionWarsModernizer.ps1') Repair -Destination $fixture }
catch { $rejected=$_.Exception.Message -match 'unexpected SHA-256' }
if (-not $rejected -or (Get-FileHash -LiteralPath $installedProfile).Hash -ne $beforeRepairHash) { throw 'Tampered focus wrapper was not rejected before repair writes.' }
Write-Host 'PASS: tampered optional focus wrapper rejected before repair writes.'
$beforeLockedProfile = (Get-FileHash -LiteralPath $installedProfile).Hash
$handle = [IO.File]::Open($installedProfile, [IO.FileMode]::Open, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None)
try {
    $rejected = $false
    try { & $modernizer Repair -Destination $fixture } catch { $rejected = $_.Exception.Message -match 'locked or read-only' }
    if (-not $rejected) { throw 'Repair failed to preflight a locked managed file.' }
}
finally { $handle.Dispose() }
if ((Get-FileHash -LiteralPath $installedProfile).Hash -ne $beforeLockedProfile) { throw 'Locked-file preflight changed the profile.' }
Write-Host 'PASS: locked managed file rejected before replacement.'
# Diagnostic output lives outside state and intentionally survives removal.
New-Item -ItemType Directory -Path (Join-Path $fixture '.dw-launcher') -Force | Out-Null
New-Item -ItemType File -Path (Join-Path $fixture '.dw-launcher\launcher.log') | Out-Null
$null=Start-DominionWarsFocusSession $fixture
& $modernizer Uninstall -Destination $fixture
foreach ($record in $state.ManagedFiles) {
    if (Test-Path -LiteralPath (Join-Path $fixture $record.Name)) { throw "Uninstall left unchanged managed file: $($record.Name)" }
}
if (-not (Test-Path -LiteralPath (Join-Path $fixture 'ds9dw.exe')) -or
    -not (Test-Path -LiteralPath (Join-Path $fixture '.dw-launcher\launcher.log'))) { throw 'Game or diagnostics were removed.' }
Write-Host 'PASS: twenty-one-file install, repair, and removal; game and diagnostics preserved.'
# Exercise the documented preservation rule for an explicitly changed backend.
& $modernizer Install -SourcePath $fixture -Destination $fixture -NoDesktopShortcut
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$null = Set-DominionWarsGraphicsBackend -ProfilePath $installedProfile -Backend Native
& $modernizer Uninstall -Destination $fixture
if (-not (Test-Path -LiteralPath $installedProfile) -or
    [IO.File]::ReadAllText($installedProfile) -notmatch 'ForceD3D9On12 = off') { throw 'Changed backend profile was not preserved.' }
# A subsequent installation treats that retained profile as an original and
# restores it on removal, even after changing the installed backend again.
$retainedHash = (Get-FileHash -LiteralPath $installedProfile).Hash
& $modernizer Install -SourcePath $fixture -Destination $fixture -NoDesktopShortcut
& $modernizer Uninstall -Destination $fixture
if ((Get-FileHash -LiteralPath $installedProfile).Hash -ne $retainedHash) { throw 'Original compatibility file backup was not restored.' }
Write-Host "PASS: backend-modified profile preserved; pre-existing profile restored. Fixture: $fixture"
