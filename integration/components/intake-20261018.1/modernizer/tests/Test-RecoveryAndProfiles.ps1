[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$scratch = Join-Path $root ('.test-recovery-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }; Write-Host "PASS: $Message"
}
function Mode([int]$W, [int]$H, [int]$Hz = 60) {
    [pscustomobject]@{ dmPelsWidth = $W; dmPelsHeight = $H; dmDisplayFrequency = $Hz; dmBitsPerPel = 32; dmDisplayOrientation = 0; dmPositionX = 0; dmPositionY = 0 }
}
$original = Mode 2560 1440 144; $temporary = Mode 1280 960
$state = [pscustomobject]@{ Phase = 'Armed'; DeviceIdentity = 'test monitor'; OriginalMode = $original; TemporaryMode = $temporary }
$args = @{ State = $state; CurrentMode = $temporary; DeviceIdentity = 'test monitor'; ParentAlive = $false; GameAlive = $false }
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'Restore') 'Orphaned temporary mode is eligible for recovery'
$args.ParentAlive = $true
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'Wait') 'Recovery waits for the launcher'
$args.ParentAlive = $false; $args.GameAlive = $true
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'Wait') 'Recovery waits for a still-running game'
$args.GameAlive = $false; $args.DeviceIdentity = 'replacement monitor'
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'DisplayChanged') 'Hot-plugged/replaced monitor is not changed'
$args.DeviceIdentity = 'test monitor'; $args.CurrentMode = $original
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'AlreadyRestored') 'Already-restored mode is left alone'
$args.CurrentMode = Mode 1920 1080
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'UserModeChanged') 'Later user resolution is not overwritten'
$args.CurrentMode = Mode 1280 960 75
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'UserModeChanged') 'Later user refresh rate is not overwritten'
$args.CurrentMode = Mode 1280 960
$args.CurrentMode.dmPositionX = 1280
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'UserModeChanged') 'Later monitor positioning is not overwritten'
$state.Phase = 'Prepared'
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'NotArmed') 'Prepared but unused recovery state never switches the display'
$state.Phase = 'Complete'
Assert ((Get-DominionWarsRecoveryDecision @args) -eq 'Complete') 'Completed recovery does nothing'
$state.Phase = 'Unrecognized'
$rejected = $false
try { $null = Get-DominionWarsRecoveryDecision @args } catch { $rejected = $true }
Assert $rejected 'An unrecognized recovery phase cannot authorize display changes'

Initialize-DominionWarsDisplay
$device = [DominionWarsDisplayMode]::GetPrimaryDevice()
$current = [DominionWarsDisplayMode]::GetCurrent($device)
$snapshot = $current | ConvertTo-Json | ConvertFrom-Json
$roundtrip = ConvertFrom-DominionWarsModeSnapshot $snapshot
Assert ($roundtrip.dmSize -eq $current.dmSize -and $roundtrip.dmDisplayFrequency -eq $current.dmDisplayFrequency -and
    $roundtrip.dmPelsWidth -eq $current.dmPelsWidth -and $roundtrip.dmFields -eq $current.dmFields) 'Complete native display snapshot survives JSON round trip'
$recovery = Start-DominionWarsRecovery -HelperPath (Join-Path $root 'payload\Restore-DominionWarsDesktop.ps1') -Device $device `
    -Original $current -Temporary $current -GameRoot $scratch -StatePath (Join-Path $scratch 'recovery.json')
try {
    Assert $recovery.State.Ready 'Independent helper signals readiness before any mode change'
    $recovery.State.Phase = 'Complete'
    Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State
    Assert ($recovery.Process.WaitForExit(5000)) 'Helper exits when the parent marks recovery complete'
    $recovery.Process.Refresh()
    Assert ($recovery.Process.ExitCode -eq 0) 'Prepared helper completes without applying a display mode'
}
finally {
    $recovery.State.Phase = 'Complete'
    Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State
    $recovery.Process.Dispose()
}

$profileNames = @('DDrawCompat-ds9dw.ini', 'DDrawCompat.ini', 'DDrawCompatOverlay-ds9dw.ini')
foreach ($name in $profileNames) {
    Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination (Join-Path $scratch $name)
}
$originalHashes = @{}
foreach ($name in $profileNames) { $originalHashes[$name] = Get-DominionWarsFileHash (Join-Path $scratch $name) }
$backup = Reset-DominionWarsProfile $scratch
$clean = [IO.File]::ReadAllText((Join-Path $scratch 'DDrawCompat-ds9dw.ini')).Replace("`r`n", "`n")
$packaged = [IO.File]::ReadAllText((Join-Path $root 'payload\DDrawCompat-ds9dw.ini')).Replace("`r`n", "`n")
Assert ($clean -eq $packaged) 'Reset recipe matches the packaged profile without drift'
Assert (-not (Test-Path -LiteralPath (Join-Path $scratch 'DDrawCompatOverlay-ds9dw.ini')) -and
    -not (Test-Path -LiteralPath (Join-Path $scratch 'DDrawCompat.ini'))) 'Reset removes conflicting game-local overlays only after backing them up'
Assert (Test-Path -LiteralPath (Join-Path $backup 'DDrawCompatOverlay-ds9dw.ini')) 'Overlay original remains recoverable'
$rejected = $false
try { $null = Reset-DominionWarsProfile $scratch } catch { $rejected = $true }
Assert $rejected 'Repeated reset cannot silently replace the pending undo record'
$profilePath = Join-Path $scratch 'DDrawCompat-ds9dw.ini'
$cleanCopy = Join-Path $scratch 'clean-profile.ini'
Copy-Item -LiteralPath $profilePath -Destination $cleanCopy
$null = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend Native
$rejected = $false
try { $null = Restore-DominionWarsProfile $scratch } catch { $rejected = $true }
Assert $rejected 'Undo refuses to overwrite changes made after reset'
Copy-Item -LiteralPath $cleanCopy -Destination $profilePath -Force
$null = Restore-DominionWarsProfile $scratch
foreach ($name in $profileNames) { Assert ((Get-DominionWarsFileHash (Join-Path $scratch $name)) -eq $originalHashes[$name]) "Reset undo restores original bytes: $name" }
$after = [DominionWarsDisplayMode]::GetCurrent($device)
Assert ($after.dmPelsWidth -eq $current.dmPelsWidth -and $after.dmPelsHeight -eq $current.dmPelsHeight -and
    $after.dmDisplayFrequency -eq $current.dmDisplayFrequency) 'No real display mode was changed during these checks'
Write-Host "Recovery/profile checks passed. Disposable output: $scratch"
