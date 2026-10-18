[CmdletBinding()]
param(
    [ValidateRange(1,120)][int]$Seconds = 20,
    [string]$LogDirectory = (Join-Path $env:LOCALAPPDATA 'Dominion Wars Modernizer\InstallLogs')
)
Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsInstall.psm1') -Force
$guard = $null
try {
    $guard = Start-DWInstallConsoleGuard
    $null = Start-DWInstallLog -Action ConsoleClickTest -Root $LogDirectory
    Write-DWInstallEvent 'Installer console guard test, revision console-guard-2. No game files, shortcut, display settings or registry values are modified.'
    Write-DWInstallEvent $guard.Status
    Write-DWInstallEvent "For about $Seconds seconds: click and drag inside this window, then Alt+Tab away and back. The timer should keep advancing without Esc."
    Write-DWInstallEvent 'If it pauses, wait about ten seconds, press Esc once, and report that it was required. Do not use Ctrl+C to advance the test or deliberately enter keyboard Mark mode.'
    $watch = [Diagnostics.Stopwatch]::StartNew()
    while ($watch.Elapsed.TotalSeconds -lt $Seconds) {
        Write-DWInstallEvent ('Console click test heartbeat; elapsed {0:N1}/{1}s.' -f $watch.Elapsed.TotalSeconds, $Seconds)
        Start-Sleep -Milliseconds 1000
    }
    Write-DWInstallEvent ('Console click test finished; elapsed {0:N1}s. This timer result alone is not evidence that mouse interaction passed; report what happened.' -f $watch.Elapsed.TotalSeconds)
    Write-DWInstallEvent 'Run Collect test logs.cmd from the USB test kit to bring back this run. Normal console mode is restored after this message.'
}
finally { Stop-DWInstallConsoleGuard -Guard $guard }
