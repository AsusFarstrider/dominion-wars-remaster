[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$scratch = Join-Path $root ('.test-launcher-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
$script:checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    $script:checks++; Write-Host "PASS: $Message"
}
function Mode([int]$W, [int]$H, [int]$Hz = 60, [int]$Depth = 32, [int]$Orientation = 0) {
    [pscustomobject]@{ dmPelsWidth = $W; dmPelsHeight = $H; dmDisplayFrequency = $Hz;
        dmBitsPerPel = $Depth; dmDisplayOrientation = $Orientation }
}
$original = Mode 2560 1440 144
$modes = @((Mode 800 600), (Mode 1280 960 60), (Mode 1280 960 120),
    (Mode 1280 960 144 16), (Mode 1024 768), (Mode 640 480), (Mode 1280 960 144 32 1))
$ordered = @(Get-DominionWarsModeCandidates -Original $original -Modes $modes)
Assert ($ordered.Count -eq 5) 'Reject wrong color depth and orientation'
Assert ($ordered[0].dmPelsWidth -eq 1280 -and $ordered[0].dmDisplayFrequency -eq 120) 'Prefer requested size with nearest supported refresh rate'
Assert ($ordered[2].dmPelsWidth -eq 1024 -and $ordered[3].dmPelsWidth -eq 800) 'Fall back in descending 4:3 size order'
$fallback = @(Get-DominionWarsModeCandidates -Original $original -Modes @((Mode 800 600), (Mode 1024 768)))
Assert ($fallback[0].dmPelsWidth -eq 1024) 'Missing 1280x960 does not prevent smaller supported modes'
Assert (@(Get-DominionWarsModeCandidates -Original $original -Modes @() ).Count -eq 0) 'Empty mode list is handled'
Assert (@(Get-DominionWarsModeCandidates -Original $original -Modes $modes -NoResolutionFallback).Count -eq 2) 'Strict requested-mode selection disables fallbacks'
$custom = @(Get-DominionWarsModeCandidates -Original $original -Modes $modes -Width 1024 -Height 768)
Assert (@($custom | Where-Object { $_.dmPelsWidth -eq 1024 }).Count -eq 1) 'Duplicate fallback sizes are eliminated'
$wide = @(Get-DominionWarsModeCandidates -Original (Mode 1366 768) -Modes @((Mode 1366 768), (Mode 1920 1080)))
Assert ($wide.Count -eq 1 -and $wide[0].dmPelsWidth -eq 1366) 'Narrow native widescreen mode can be used when no 4:3 modes exist'
Assert (@(Get-DominionWarsModeCandidates -Original $original -Modes @((Mode 1600 900), (Mode 1920 1080))).Count -eq 0) 'Expanded fallback never crosses the known mouse-failure width'
$hz = @(Get-DominionWarsModeCandidates -Original $original -Modes $modes -RefreshRate 60)
Assert ($hz[0].dmDisplayFrequency -eq 60) 'Explicit 60 Hz preference ranks supported refresh rates correctly'

$profilePath = Join-Path $scratch 'profile.ini'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $profilePath
$before = (Get-FileHash -LiteralPath $profilePath).Hash
Assert ((Get-DominionWarsFileHash -Path $profilePath) -eq $before) '.NET SHA-256 matches the standard command'
$null = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend Profile
Assert ((Get-FileHash -LiteralPath $profilePath).Hash -eq $before) 'Normal launch leaves profile bytes untouched'
$native = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend Native
Assert ($native -match 'ForceD3D9On12 = off') 'Alternate path disables forced D3D9On12'
Assert ($native -match 'CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps\|=0x2' -and $native -match 'PalettizedTextures = on' -and $native -match 'CompatFixes = singlemonitor') 'Alternate path preserves game-specific fixes'
$translated = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend D3D9On12
Assert ($translated -match 'ForceD3D9On12 = on') 'Known-good backend can be restored'
Assert ((Resolve-DominionWarsGraphicsBackend -Profile $translated -Backend Auto -CanCreateD3D12 $true) -eq 'Profile') 'Automatic backend retains the known recipe on DX12-capable hardware'
Assert ((Resolve-DominionWarsGraphicsBackend -Profile $translated -Backend Auto -CanCreateD3D12 $false) -eq 'Native') 'Automatic backend avoids forcing D3D9On12 when DX12 creation is unavailable'
Assert ((Resolve-DominionWarsGraphicsBackend -Profile $native -Backend Auto -CanCreateD3D12 $true) -eq 'Profile') 'Automatic backend honors a saved system-default selection'
Assert ((Resolve-DominionWarsGraphicsBackend -Profile $translated -Backend Profile -CanCreateD3D12 $false) -eq 'Profile') 'Explicit profile selection opts out of automatic adjustment'
Assert ((Resolve-DominionWarsGraphicsBackend -Profile $translated -Backend D3D9On12 -CanCreateD3D12 $false) -eq 'D3D9On12') 'Explicit backend remains explicit for prelaunch capability rejection'
$missingRejected = $false
try { $null = Set-DominionWarsGraphicsBackend -ProfilePath (Join-Path $scratch 'missing.ini') -Backend Native }
catch { $missingRejected = $true }
Assert $missingRejected 'Missing profile gives a repair error'

$events = New-Object 'Collections.Generic.List[string]'
Invoke-DominionWarsSession -Apply { $events.Add('apply') } -Run { $events.Add('run') } -Restore { $events.Add('restore') }
Assert (($events -join ',') -eq 'apply,run,restore') 'Normal session restores after process completion'
$events.Clear(); $failed = $false
try {
    Invoke-DominionWarsSession -Apply { $events.Add('apply') } -Run { throw 'simulated startup failure' } -Restore { $events.Add('restore') }
} catch { $failed = $true }
Assert ($failed -and ($events -join ',') -eq 'apply,restore') 'Process-start failure still restores the desktop'
$events.Clear(); $failed = $false
try {
    Invoke-DominionWarsSession -Apply { $events.Add('apply'); throw 'simulated display failure' } -Run { $events.Add('run') } -Restore { $events.Add('restore') }
} catch { $failed = $true }
Assert ($failed -and ($events -join ',') -eq 'apply,restore') 'Display failure prevents process launch and attempts restoration'

# Parser checks cover all shipped PowerShell files on Windows PowerShell 5.1.
$scripts = @(Get-Item -LiteralPath (Join-Path $root 'DominionWarsModernizer.ps1')) +
    @(Get-ChildItem -LiteralPath (Join-Path $root 'payload'), (Join-Path $root 'tests') -Recurse -File)
$scripts = @($scripts | Where-Object { $_.Extension -in '.ps1', '.psm1' })
foreach ($file in $scripts) {
    $tokens = $null; $errors = $null
    $null = [Management.Automation.Language.Parser]::ParseFile($file.FullName, [ref]$tokens, [ref]$errors)
    Assert ($errors.Count -eq 0) "PowerShell syntax: $($file.Name)"
}
Write-Host "$script:checks checks passed. Disposable output: $scratch"
