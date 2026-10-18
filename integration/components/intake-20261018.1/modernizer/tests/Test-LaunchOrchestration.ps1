[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$launcher = Join-Path $root 'payload\Launch-DominionWars.ps1'
$scratch = Join-Path $root ('.test-orchestration-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
$profile = Join-Path $scratch 'DDrawCompat-ds9dw.ini'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $profile
$probe = Join-Path $scratch 'probe.exe'
$compiler = Join-Path $env:SystemRoot 'Microsoft.NET\Framework\v4.0.30319\csc.exe'
& $compiler /nologo /target:winexe "/out:$probe" (Join-Path $PSScriptRoot 'ExitProbe.cs')
if ($LASTEXITCODE -ne 0) { throw 'Could not compile the disposable exit-code probe.' }
# Substitute only display operations; exercise the real launcher and process
# waiting, but never actually change a monitor mode or start Dominion Wars.
Add-Type -TypeDefinition @'
using System;
public static class DominionWarsDisplayMode
{
    public struct DEVMODE {
        public int dmPelsWidth, dmPelsHeight, dmDisplayFrequency, dmBitsPerPel, dmDisplayOrientation, dmPositionX, dmPositionY;
    }
    public static int ApplyCalls;
    public static bool FailNextApply;
    public static bool D3D12Available = true;
    public static bool SimulateUserModeChange;
    public static bool SimulatePrimaryChange;
    public static DEVMODE Current;
    public static string GetPrimaryDevice() { return SimulatePrimaryChange && ApplyCalls > 0 ? "REPLACEMENT-DISPLAY" : "TEST-DISPLAY"; }
    public static string GetDeviceIdentity(string d) { return "TEST-MONITOR"; }
    public static DEVMODE GetCurrent(string d) {
        if (ApplyCalls == 0) Current = new DEVMODE { dmPelsWidth=2560, dmPelsHeight=1440, dmDisplayFrequency=144, dmBitsPerPel=32 };
        if (SimulateUserModeChange && ApplyCalls > 0) Current = new DEVMODE { dmPelsWidth=1920, dmPelsHeight=1080, dmDisplayFrequency=60, dmBitsPerPel=32 };
        return Current;
    }
    public static DEVMODE[] GetModes(string d) {
        return new DEVMODE[] { new DEVMODE { dmPelsWidth=1280, dmPelsHeight=960, dmDisplayFrequency=60, dmBitsPerPel=32 } };
    }
    public static int Test(string d, DEVMODE m) { return 0; }
    public static bool CanCreateD3D12Device() { return D3D12Available; }
    public static void Apply(string d, DEVMODE m) {
        ApplyCalls++;
        if (FailNextApply) { FailNextApply=false; throw new Exception("simulated display rejection"); }
        Current = m;
    }
}
'@
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    Write-Host "PASS: $Message"
}
$hash = (Get-FileHash -LiteralPath $profile).Hash
& $launcher -GamePath $probe -DiagnoseOnly -GraphicsBackend Native -VisualPreset Sharper
Assert ($LASTEXITCODE -eq 0 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0) 'Diagnostics do not change displays'
Assert ((Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Diagnostics do not edit the profile even with a backend argument'
& $launcher -GamePath $probe -FocusMode Experimental -DiagnoseOnly -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 0 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0 -and (Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Experimental diagnostics do not swap files or change the display'
& $launcher -GamePath $probe -FocusMode Experimental -VisualPreset Sharper -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0) 'Focus mode rejects visual preset mixing before display changes'
& $launcher -GamePath $probe -FocusMode Experimental -Width 1024 -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0) 'Focus mode rejects unvalidated initialization sizes before display changes'
$savedGuard=$env:DW_EXPERIMENTAL_FOCUS_GUARD
$env:DW_EXPERIMENTAL_FOCUS_GUARD='1'
$env:DW_TEST_FOCUS_ENVIRONMENT_OUTPUT=Join-Path $scratch 'child-focus-environment.txt'
try {
    & $launcher -GamePath $probe -NoRecoveryWatchdog
    Assert ($LASTEXITCODE -eq 37 -and [IO.File]::ReadAllText($env:DW_TEST_FOCUS_ENVIRONMENT_OUTPUT) -eq '<unset>') 'Baseline child cannot inherit experimental focus opt-in'
    Assert ($env:DW_EXPERIMENTAL_FOCUS_GUARD -eq '1') 'Caller process environment restored after launch'
}
finally { $env:DW_EXPERIMENTAL_FOCUS_GUARD=$savedGuard; $env:DW_TEST_FOCUS_ENVIRONMENT_OUTPUT=$null }
[DominionWarsDisplayMode]::ApplyCalls=0
& $launcher -GamePath $probe -VisualPreset Sharper -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and (Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Per-run sharper preset restores exact original profile after child exit'
$report = Get-Content -LiteralPath (Join-Path $scratch '.dw-launcher\diagnostics.json') -Raw | ConvertFrom-Json
Assert ($report.VisualPresetRequested -eq 'Sharper' -and $report.Profile -match 'DisplayFilter = bicubic\(0\)') 'Diagnostics record the actual experimental preset used during the run'
[DominionWarsDisplayMode]::ApplyCalls = 0
& $launcher -GamePath $probe -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37) 'Real child process exit code propagates'
Assert ([DominionWarsDisplayMode]::ApplyCalls -eq 2) 'Successful process wait restores original mode'
[DominionWarsDisplayMode]::ApplyCalls = 0
[DominionWarsDisplayMode]::FailNextApply = $true
& $launcher -GamePath $probe -VisualPreset Enhanced3D -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 1) 'Rejected display change returns failure without reapplying an already-original desktop'
Assert ((Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Display failure restores the original visual profile'
[DominionWarsDisplayMode]::ApplyCalls = 0
$invalid = Join-Path $scratch 'invalid.exe'
New-Item -ItemType File -Path $invalid | Out-Null
& $launcher -GamePath $invalid -VisualPreset ResourceSaving -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 2) 'Invalid executable launch returns failure and restores desktop'
Assert ((Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Process-start failure restores the original visual profile'
[DominionWarsDisplayMode]::ApplyCalls = 0
[DominionWarsDisplayMode]::SimulateUserModeChange = $true
& $launcher -GamePath $probe -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and [DominionWarsDisplayMode]::ApplyCalls -eq 1 -and
    [DominionWarsDisplayMode]::Current.dmPelsWidth -eq 1920) 'Normal shutdown preserves a display adjustment made after launch'
[DominionWarsDisplayMode]::SimulateUserModeChange = $false
[DominionWarsDisplayMode]::ApplyCalls = 0
[DominionWarsDisplayMode]::SimulatePrimaryChange = $true
& $launcher -GamePath $probe -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and [DominionWarsDisplayMode]::ApplyCalls -eq 1) 'Normal shutdown never applies the saved mode after the primary monitor changes'
[DominionWarsDisplayMode]::SimulatePrimaryChange = $false
[DominionWarsDisplayMode]::ApplyCalls = 0
& $launcher -GamePath $probe -VisualPreset Enhanced3D -ConfigureOnly -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0 -and
    (Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Per-run preset cannot be left applied by ConfigureOnly'
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$null = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Enhanced3D
$pendingHash = (Get-FileHash -LiteralPath $profile).Hash
& $launcher -GamePath $probe -DiagnoseOnly -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 0 -and (Get-FileHash -LiteralPath $profile).Hash -eq $pendingHash) 'Diagnostics do not modify an interrupted preset record or profile'
& $launcher -GamePath $probe -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and (Get-FileHash -LiteralPath $profile).Hash -eq $hash) 'Next ordinary launch recovers an interrupted preset before starting'
[DominionWarsDisplayMode]::ApplyCalls = 0
[DominionWarsDisplayMode]::D3D12Available = $false
& $launcher -GamePath $probe -GraphicsBackend D3D9On12 -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 1 -and [DominionWarsDisplayMode]::ApplyCalls -eq 0) 'Unsupported explicit D3D9On12 fails before changing desktop'
& $launcher -GamePath $probe -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and [IO.File]::ReadAllText($profile) -match 'ForceD3D9On12 = off') 'Normal launcher automatically selects system-default graphics when DX12 is unavailable'
& $launcher -GamePath $probe -GraphicsBackend Native -NoRecoveryWatchdog
Assert ($LASTEXITCODE -eq 37 -and [IO.File]::ReadAllText($profile) -match 'ForceD3D9On12 = off') 'Alternate backend launches and persists selection'
$mutex = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Display')
$null = $mutex.WaitOne(0)
try {
    $output = & (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') -NoProfile -ExecutionPolicy Bypass -File $launcher -GamePath $probe -DiagnoseOnly | Out-String
    Assert ($LASTEXITCODE -eq 1 -and $output -match 'already active') 'Separate competing launcher is rejected'
}
finally { $mutex.ReleaseMutex(); $mutex.Dispose() }
& (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'Test-MinimalModuleEnvironment.ps1') -GamePath $probe
if ($LASTEXITCODE -ne 0) { throw 'Restricted module environment regression failed.' }
Write-Host "PASS: orchestration checks; fake displays only. Disposable output: $scratch"
