[CmdletBinding()]
param([string]$StatePath, [switch]$Manual)
$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'DominionWarsLauncher.psm1') -Force
if (-not $StatePath) { $StatePath = Get-DominionWarsRecoveryPath }
if (-not (Test-Path -LiteralPath $StatePath -PathType Leaf)) { Write-Host 'No saved Dominion Wars display state was found.'; exit 0 }
$guard = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Recovery')
$ownsGuard = $false; $displayLock = $null; $ownsDisplay = $false
try {
    try { $ownsGuard = $guard.WaitOne(0) } catch [Threading.AbandonedMutexException] { $ownsGuard = $true }
    if (-not $ownsGuard) { throw 'A display recovery helper is already running.' }
    $state = Get-Content -LiteralPath $StatePath -Raw | ConvertFrom-Json
    if ($state.SchemaVersion -ne 1 -or $state.SessionId -ne [Diagnostics.Process]::GetCurrentProcess().SessionId) {
        throw 'Saved state does not belong to this Windows session.'
    }
    $state.Ready = $true
    Write-DominionWarsRecoveryState -Path $StatePath -State $state
    while ($true) {
        $state = Get-Content -LiteralPath $StatePath -Raw | ConvertFrom-Json
        if ($state.Phase -eq 'Complete') { break }
        $parentAlive = Test-DominionWarsProcessIdentity -ProcessId $state.ParentPid -StartTicks $state.ParentStartTicks
        $gameAlive = @(Get-Process -Name ds9dw -ErrorAction SilentlyContinue | Where-Object { $_.SessionId -eq $state.SessionId }).Count -gt 0
        if ($parentAlive -or $gameAlive) {
            if ($Manual) { throw 'Close Dominion Wars and its launcher before requesting manual recovery.' }
            Start-Sleep -Milliseconds 500; continue
        }
        if ($state.Phase -eq 'Prepared') { $decision = 'NotArmed'; break }
        $displayLock = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Display')
        try { $ownsDisplay = $displayLock.WaitOne(0) } catch [Threading.AbandonedMutexException] { $ownsDisplay = $true }
        if (-not $ownsDisplay) { throw 'Another launcher owns the display; recovery was not applied.' }
        $decision = Restore-DominionWarsDesktopMode -State $state
        break
    }
    if ($state.Phase -ne 'Complete') {
        $state.Phase = 'Complete'
        $state | Add-Member -NotePropertyName RecoveryResult -NotePropertyValue $decision -Force
        Write-DominionWarsRecoveryState -Path $StatePath -State $state
        Write-Host "Display recovery result: $decision"
    }
}
catch {
    $message = "{0:o} Recovery error: {1}" -f [DateTime]::UtcNow, $_.Exception.Message
    Add-Content -LiteralPath "$StatePath.log" -Value $message -Encoding UTF8
    Write-Host $message
    exit 1
}
finally {
    if ($ownsDisplay) { $displayLock.ReleaseMutex() }; if ($displayLock) { $displayLock.Dispose() }
    if ($ownsGuard) { $guard.ReleaseMutex() }; $guard.Dispose()
}
