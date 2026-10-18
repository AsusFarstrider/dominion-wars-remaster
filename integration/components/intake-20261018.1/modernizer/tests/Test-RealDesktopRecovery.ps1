[CmdletBinding()]
param([switch]$ExerciseRealDisplay, [switch]$Worker, [string]$EvidenceRoot)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
if (-not $ExerciseRealDisplay) { throw 'This opt-in test changes the primary desktop temporarily. Supply -ExerciseRealDisplay only when ready.' }
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
Initialize-DominionWarsDisplay
$session = [Diagnostics.Process]::GetCurrentProcess().SessionId
if (@(Get-Process -Name ds9dw -ErrorAction SilentlyContinue | Where-Object { $_.SessionId -eq $session }).Count) {
    throw 'Close Dominion Wars before running the real-display recovery test.'
}
function SameMode($First, $Second) {
    foreach ($field in @('dmPelsWidth', 'dmPelsHeight', 'dmDisplayFrequency', 'dmBitsPerPel',
        'dmDisplayOrientation', 'dmPositionX', 'dmPositionY')) {
        if ($First.$field -ne $Second.$field) { return $false }
    }
    return $true
}
if ($Worker) {
    if (-not $EvidenceRoot -or -not (Test-Path -LiteralPath $EvidenceRoot -PathType Container)) { throw 'Worker evidence directory is missing.' }
    $lock = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Display')
    $ownsLock = $false; $recovery = $null
    try {
        try { $ownsLock = $lock.WaitOne(0) } catch [Threading.AbandonedMutexException] { $ownsLock = $true }
        if (-not $ownsLock) { throw 'Another launcher owns the display.' }
        $guard = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Recovery')
        $ownsGuard = $false
        try {
            try { $ownsGuard = $guard.WaitOne(0) } catch [Threading.AbandonedMutexException] { $ownsGuard = $true }
            if (-not $ownsGuard) { throw 'Another recovery helper is active.' }
        }
        finally { if ($ownsGuard) { $guard.ReleaseMutex() }; $guard.Dispose() }
        $device = [DominionWarsDisplayMode]::GetPrimaryDevice()
        $original = [DominionWarsDisplayMode]::GetCurrent($device)
        $saved = Get-Content -LiteralPath (Join-Path $EvidenceRoot 'original.json') -Raw | ConvertFrom-Json
        if ($device -ne $saved.Device -or [DominionWarsDisplayMode]::GetDeviceIdentity($device) -ne $saved.DeviceIdentity -or
            -not (SameMode $original $saved.OriginalMode)) { throw 'Display changed while preparing the test; no mode was applied.' }
        $modes = @([DominionWarsDisplayMode]::GetModes($device))
        $temporary = Get-DominionWarsModeCandidates -Original $original -Modes $modes |
            Where-Object { -not (SameMode $_ $original) -and [DominionWarsDisplayMode]::Test($device, $_) -eq 0 } |
            Select-Object -First 1
        if (-not $temporary) { throw 'No distinct driver-tested temporary mode is available.' }
        $recovery = Start-DominionWarsRecovery -HelperPath (Join-Path $root 'payload\Restore-DominionWarsDesktop.ps1') `
            -Device $device -Original $original -Temporary $temporary -GameRoot $EvidenceRoot `
            -StatePath (Join-Path $EvidenceRoot 'recovery.json')
        $recovery.State.Phase = 'Armed'
        Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State
        [DominionWarsDisplayMode]::Apply($device, $temporary)
        [pscustomobject]@{
            WorkerPid = $PID; HelperPid = $recovery.Process.Id
            HelperStartTicks = $recovery.Process.StartTime.ToUniversalTime().Ticks.ToString()
        } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $EvidenceRoot 'ready.json') -Encoding UTF8
        # Controller deliberately kills only this disposable worker. If it does
        # not, a bounded wait and normal cleanup avoid leaving the test armed.
        $timer = [Diagnostics.Stopwatch]::StartNew()
        while ($timer.Elapsed.TotalSeconds -lt 30) { Start-Sleep -Milliseconds 250 }
        throw 'Controller did not terminate the worker in time.'
    }
    finally {
        if ($recovery) {
            $null = Restore-DominionWarsDesktopMode -State $recovery.State
            $recovery.State.Phase = 'Complete'
            Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State
            $recovery.Process.Dispose()
        }
        if ($ownsLock) { $lock.ReleaseMutex() }; $lock.Dispose()
    }
    exit
}
if ($EvidenceRoot) { throw 'EvidenceRoot is reserved for the disposable worker.' }
$scratch = Join-Path $root ('.test-real-recovery-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
$device = [DominionWarsDisplayMode]::GetPrimaryDevice()
$identity = [DominionWarsDisplayMode]::GetDeviceIdentity($device)
$original = [DominionWarsDisplayMode]::GetCurrent($device)
[pscustomobject]@{ Device = $device; DeviceIdentity = $identity; OriginalMode = $original } |
    ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $scratch 'original.json') -Encoding UTF8
$workerProcess = $null; $helperProcess = $null
try {
    $arguments = '-NoProfile -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -ExerciseRealDisplay -Worker -EvidenceRoot "' + $scratch + '"'
    $workerProcess = Start-Process -FilePath (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') `
        -ArgumentList $arguments -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput (Join-Path $scratch 'worker-output.log') -RedirectStandardError (Join-Path $scratch 'worker-error.log')
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $readyPath = Join-Path $scratch 'ready.json'
    while (-not (Test-Path -LiteralPath $readyPath)) {
        if ($workerProcess.HasExited) { throw "Worker exited before readiness; inspect $scratch." }
        if ($timer.Elapsed.TotalSeconds -ge 15) { throw 'Timed out waiting for the disposable worker.' }
        Start-Sleep -Milliseconds 100
    }
    $ready = Get-Content -LiteralPath $readyPath -Raw | ConvertFrom-Json
    $state = Get-Content -LiteralPath (Join-Path $scratch 'recovery.json') -Raw | ConvertFrom-Json
    if ($ready.WorkerPid -ne $workerProcess.Id -or $state.ParentPid -ne $workerProcess.Id -or $state.Phase -ne 'Armed' -or
        -not (SameMode ([DominionWarsDisplayMode]::GetCurrent($device)) $state.TemporaryMode)) { throw 'Worker did not arm the expected real temporary mode.' }
    if (-not (Test-DominionWarsProcessIdentity -ProcessId $ready.HelperPid -StartTicks $ready.HelperStartTicks)) { throw 'Recovery helper is not running.' }
    $helperProcess = [Diagnostics.Process]::GetProcessById($ready.HelperPid)
    $null = $helperProcess.Handle
    Write-Host "Temporary $($state.TemporaryMode.dmPelsWidth)x$($state.TemporaryMode.dmPelsHeight) mode confirmed; terminating disposable worker PID=$($workerProcess.Id)."
    $workerProcess.Kill()
    if (-not $workerProcess.WaitForExit(5000)) { throw 'Worker did not terminate.' }
    if (-not $helperProcess.WaitForExit(15000)) { throw 'Independent helper did not finish recovery.' }
    $helperProcess.Refresh()
    $state = Get-Content -LiteralPath (Join-Path $scratch 'recovery.json') -Raw | ConvertFrom-Json
    if ($helperProcess.ExitCode -ne 0 -or $state.Phase -ne 'Complete' -or $state.RecoveryResult -notin @('Restore', 'AlreadyRestored')) {
        throw "Independent helper did not record successful restoration (exit=$($helperProcess.ExitCode), phase=$($state.Phase), result=$($state.RecoveryResult))."
    }
    $terminationResult = $state.RecoveryResult
    if ($terminationResult -eq 'AlreadyRestored') {
        # Some systems have already reset the temporary mode when its owner
        # terminates. Exercise the helper's actual Apply path separately: keep
        # the same dead parent identity, but let this live controller establish
        # the orphaned mode. This is a synthetic orphan, not a game crash.
        if (-not (SameMode ([DominionWarsDisplayMode]::GetCurrent($device)) $original)) { throw 'AlreadyRestored result disagrees with the real desktop.' }
        $helperProcess.Dispose(); $helperProcess = $null
        $state.Phase = 'Armed'; $state.Ready = $false
        Write-DominionWarsRecoveryState -Path (Join-Path $scratch 'recovery.json') -State $state
        [DominionWarsDisplayMode]::Apply($device, (ConvertFrom-DominionWarsModeSnapshot $state.TemporaryMode))
        $helperArguments = '-NoProfile -ExecutionPolicy Bypass -File "' + (Join-Path $root 'payload\Restore-DominionWarsDesktop.ps1') + '" -StatePath "' + (Join-Path $scratch 'recovery.json') + '"'
        $helperProcess = Start-Process -FilePath (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') `
            -ArgumentList $helperArguments -WindowStyle Hidden -PassThru
        if (-not $helperProcess.WaitForExit(15000)) { throw 'Helper did not finish the synthetic orphan recovery.' }
        $helperProcess.Refresh()
        $state = Get-Content -LiteralPath (Join-Path $scratch 'recovery.json') -Raw | ConvertFrom-Json
        if ($helperProcess.ExitCode -ne 0 -or $state.Phase -ne 'Complete' -or $state.RecoveryResult -ne 'Restore') { throw 'Helper did not restore the synthetic orphaned mode.' }
    }
    $after = [DominionWarsDisplayMode]::GetCurrent($device)
    if ([DominionWarsDisplayMode]::GetPrimaryDevice() -ne $device -or
        [DominionWarsDisplayMode]::GetDeviceIdentity($device) -ne $identity -or -not (SameMode $after $original)) { throw 'Original desktop mode was not fully restored.' }
    [pscustomobject]@{ Passed = $true; RecordedUtc = [DateTime]::UtcNow.ToString('o'); ParentTerminationResult = $terminationResult; Result = $state.RecoveryResult; RestoredMode = $after } |
        ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $scratch 'result.json') -Encoding UTF8
    Write-Host "PASS: independent helper restored the real desktop after parent termination. Evidence: $scratch"
}
finally {
    if ($workerProcess -and -not $workerProcess.HasExited) { $workerProcess.Kill(); $null = $workerProcess.WaitForExit(5000) }
    if ($helperProcess -and -not $helperProcess.HasExited) { $null = $helperProcess.WaitForExit(5000) }
    # Emergency cleanup cannot turn a failed watchdog test into a pass. Only
    # touch the same monitor still displaying this fixture's temporary mode.
    $statePath = Join-Path $scratch 'recovery.json'
    if (Test-Path -LiteralPath $statePath) {
        $state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json
        if ([DominionWarsDisplayMode]::GetPrimaryDevice() -eq $device -and [DominionWarsDisplayMode]::GetDeviceIdentity($device) -eq $identity -and
            (SameMode ([DominionWarsDisplayMode]::GetCurrent($device)) $state.TemporaryMode)) {
            [DominionWarsDisplayMode]::Apply($device, $original)
            Write-Warning 'Controller performed emergency display restoration; this is not a watchdog pass.'
        }
    }
    if ($helperProcess) { $helperProcess.Dispose() }; if ($workerProcess) { $workerProcess.Dispose() }
}
