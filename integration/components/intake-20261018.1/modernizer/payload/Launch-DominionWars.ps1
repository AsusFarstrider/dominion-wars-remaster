[CmdletBinding()]
param(
    [string]$GamePath,
    [ValidateRange(640, 1599)][int]$Width = 1280,
    [ValidateRange(480, 1200)][int]$Height = 960,
    [ValidateRange(0, 1000)][int]$RefreshRate = 0,
    [ValidateSet('Auto', 'Profile', 'Native', 'D3D9On12')][string]$GraphicsBackend = 'Auto',
    [ValidateSet('Baseline', 'Sharper', 'Enhanced3D', 'ResourceSaving')][string]$VisualPreset = 'Baseline',
    [ValidateSet('Baseline', 'Experimental')][string]$FocusMode = 'Baseline',
    [switch]$NoResolutionFallback,
    [switch]$DiagnoseOnly,
    [switch]$NoRecoveryWatchdog,
    [switch]$ConfigureOnly,
    [switch]$ResetCompatibilityProfile,
    [switch]$RestoreCompatibilityProfile
)

$ErrorActionPreference = 'Stop'
$launcherRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exitCode = 1
$mutex = $null
$ownsMutex = $false
$logPath = $null
$recovery = $null
$visualPresetActive = $false
$focusActive = $false
$focusEnvironment = @{}
function Write-LaunchLog([string]$Message) {
    Write-Host $Message
    if ($logPath) {
        try { Add-Content -LiteralPath $logPath -Value ("{0:o} {1}" -f [DateTime]::UtcNow, $Message) -Encoding UTF8 }
        catch { Write-Warning "Could not write launcher log: $($_.Exception.Message)" }
    }
}

try {
    Import-Module (Join-Path $launcherRoot 'DominionWarsLauncher.psm1') -Force
    Import-Module (Join-Path $launcherRoot 'DominionWarsFocus.psm1') -Force
    foreach ($name in (Get-DominionWarsFocusEnvironmentNames)) {
        $focusEnvironment[$name] = [Environment]::GetEnvironmentVariable($name,'Process')
        [Environment]::SetEnvironmentVariable($name,$null,'Process')
    }
    if (-not $GamePath) { $GamePath = Join-Path $launcherRoot 'ds9dw.exe' }
    $game = Get-Item -LiteralPath $GamePath
    if ($game.PSIsContainer) { throw 'GamePath must point to ds9dw.exe, not a directory.' }
    if ($ResetCompatibilityProfile -and $RestoreCompatibilityProfile) { throw 'Choose reset or restore, not both.' }
    if ($FocusMode -eq 'Experimental') {
        if ($VisualPreset -ne 'Baseline' -or $ConfigureOnly) { throw 'Experimental Alt+Tab is a per-run baseline-visual option; do not combine it with visual presets or ConfigureOnly.' }
        if ($Width -ne 1280 -or $Height -ne 960) { throw 'Experimental Alt+Tab currently requires the 1280x960 initialization mode. Use the normal launcher for fallback sizes.' }
        $NoResolutionFallback = $true
    }
    $mutex = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Display')
    try { $ownsMutex = $mutex.WaitOne(0) }
    catch [Threading.AbandonedMutexException] { $ownsMutex = $true }
    if (-not $ownsMutex) { throw 'Another Dominion Wars launcher is already active. Close the game before launching again.' }
    $recoveryGuard = New-Object Threading.Mutex($false, 'Local\DominionWarsModernizer.Recovery')
    $canProceed = $false
    try {
        try { $canProceed = $recoveryGuard.WaitOne(0) } catch [Threading.AbandonedMutexException] { $canProceed = $true }
        if (-not $canProceed) { throw 'A desktop recovery helper is still active. Wait for it to finish before launching again.' }
    }
    finally { if ($canProceed) { $recoveryGuard.ReleaseMutex() }; $recoveryGuard.Dispose() }
    if (-not $DiagnoseOnly -and @(Get-Process -Name ds9dw -ErrorAction SilentlyContinue |
        Where-Object { $_.SessionId -eq [Diagnostics.Process]::GetCurrentProcess().SessionId }).Count -gt 0) {
        throw 'Dominion Wars is already running. Close it before launching again.'
    }
    $logRoot = Join-Path $game.DirectoryName '.dw-launcher'
    New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
    $logPath = Join-Path $logRoot 'launcher.log'
    if ((Test-Path -LiteralPath $logPath) -and (Get-Item -LiteralPath $logPath).Length -gt 1MB) {
        Move-Item -LiteralPath $logPath -Destination "$logPath.previous" -Force
    }
    Write-LaunchLog "Launcher started; backend=$GraphicsBackend; visual preset=$VisualPreset; requested=${Width}x${Height}; diagnostics=$DiagnoseOnly"
    if (-not $DiagnoseOnly) {
        $recoveredFocus = Restore-DominionWarsFocusSession -GameRoot $game.DirectoryName
        if ($recoveredFocus) { Write-LaunchLog 'Recovered the stock wrapper and prior profile from an interrupted experimental Alt+Tab session.' }
        if ($ConfigureOnly -and $VisualPreset -ne 'Baseline') { throw 'Visual presets are per-run options; do not combine them with ConfigureOnly.' }
        $recoveredPreset = Restore-DominionWarsVisualPreset -GameRoot $game.DirectoryName
        if ($recoveredPreset) {
            if ($recoveredPreset.PreservedChanges) {
                Write-LaunchLog "Later profile changes preserved; original copy remains at $($recoveredPreset.BackupPath). Preset values may remain; repair restores the packaged profile."
            }
            else { Write-LaunchLog 'Recovered the original profile from an interrupted visual-preset session.' }
        }
        if ($ResetCompatibilityProfile) { Write-LaunchLog "Local profiles reset; originals backed up to $(Reset-DominionWarsProfile -GameRoot $game.DirectoryName)" }
        if ($RestoreCompatibilityProfile) { Write-LaunchLog "Local profiles restored from $(Restore-DominionWarsProfile -GameRoot $game.DirectoryName)" }
    }
    Initialize-DominionWarsDisplay
    $device = [DominionWarsDisplayMode]::GetPrimaryDevice()
    $original = [DominionWarsDisplayMode]::GetCurrent($device)
    $modes = @([DominionWarsDisplayMode]::GetModes($device))
    $candidates = @(Get-DominionWarsModeCandidates -Original $original -Modes $modes -Width $Width -Height $Height -RefreshRate $RefreshRate -NoResolutionFallback:$NoResolutionFallback)
    $tested = @($candidates | ForEach-Object {
        [pscustomobject]@{ Width = $_.dmPelsWidth; Height = $_.dmPelsHeight;
            RefreshRate = $_.dmDisplayFrequency; DriverTestResult = [DominionWarsDisplayMode]::Test($device, $_); Mode = $_ }
    })
    $chosen = $tested | Where-Object { $_.DriverTestResult -eq 0 } | Select-Object -First 1
    $profilePath = Join-Path $game.DirectoryName 'DDrawCompat-ds9dw.ini'
    $profile = if (Test-Path -LiteralPath $profilePath) { [IO.File]::ReadAllText($profilePath) } else { $null }
    $canD3D12 = [DominionWarsDisplayMode]::CanCreateD3D12Device()
    $selectedBackend = Resolve-DominionWarsGraphicsBackend -Profile $profile -Backend $GraphicsBackend -CanCreateD3D12 $canD3D12
    $inventoryErrors = @()
    $os = $null; $gpus = @(); $screens = @()
    try { $os = Get-CimInstance Win32_OperatingSystem -OperationTimeoutSec 5 | Select-Object Caption, Version, BuildNumber, OSArchitecture }
    catch { $inventoryErrors += "OS inventory: $($_.Exception.Message)" }
    try { $gpus = @(Get-CimInstance Win32_VideoController -OperationTimeoutSec 5 | Select-Object Name, DriverVersion, PNPDeviceID) }
    catch { $inventoryErrors += "GPU inventory: $($_.Exception.Message)" }
    try {
        Add-Type -AssemblyName System.Windows.Forms
        $screens = @([Windows.Forms.Screen]::AllScreens | ForEach-Object {
            [pscustomobject]@{ Device = $_.DeviceName; Primary = $_.Primary; Bounds = $_.Bounds.ToString() }
        })
    }
    catch { $inventoryErrors += "Monitor inventory: $($_.Exception.Message)" }
    $additionalConfigs = @(@(
        (Join-Path $env:PROGRAMDATA 'DDrawCompat\DDrawCompat.ini'),
        (Join-Path $env:LOCALAPPDATA 'DDrawCompat\DDrawCompat.ini'),
        (Join-Path $game.DirectoryName 'DDrawCompat.ini'),
        (Join-Path $game.DirectoryName 'DDrawCompatOverlay-ds9dw.ini')
    ) | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | ForEach-Object {
        $configHash = $null; $configError = $null
        try { $configHash = Get-DominionWarsFileHash -Path $_ }
        catch { $configError = $_.Exception.Message }
        [pscustomobject]@{ Path = $_; Sha256 = $configHash; ReadError = $configError }
    })
    $wrapperPath = Join-Path $game.DirectoryName 'ddraw.dll'
    $wrapperHash = if (Test-Path -LiteralPath $wrapperPath -PathType Leaf) { Get-DominionWarsFileHash -Path $wrapperPath } else { $null }
    $diagnostics = [ordered]@{
        RecordedUtc = [DateTime]::UtcNow.ToString('o'); Windows = $os; GPUs = $gpus; Screens = $screens
        PowerShellVersion = $PSVersionTable.PSVersion.ToString(); Is64BitProcess = [Environment]::Is64BitProcess
        PrimaryDevice = $device; OriginalMode = $original; RequestedMode = "${Width}x${Height}"
        PreferredRefreshRate = $RefreshRate; RecoveryWatchdogEnabled = -not $NoRecoveryWatchdog
        TestedModes = @($tested | Select-Object Width, Height, RefreshRate, DriverTestResult)
        DefaultAdapterD3D12Available = $canD3D12; ExecutableHash = Get-DominionWarsFileHash -Path $game.FullName
        GraphicsBackendRequested = $GraphicsBackend; GraphicsBackendSelection = $selectedBackend
        VisualPresetRequested = $VisualPreset; VisualPresetSettings = Get-DominionWarsVisualSettings $VisualPreset
        PendingVisualPresetRecovery = Test-Path -LiteralPath (Join-Path $logRoot 'visual-preset.json') -PathType Leaf
        FocusModeRequested = $FocusMode
        PendingFocusRecovery = Test-Path -LiteralPath (Join-Path $logRoot 'focus-session.json') -PathType Leaf
        Profile = $profile; InventoryErrors = $inventoryErrors
        WrapperHash = $wrapperHash; AdditionalDDrawCompatConfigs = $additionalConfigs
        LegacyWindowsCompatibilityFlags = @(Get-DominionWarsLegacyFlags -GamePath $game.FullName)
        SelectedMode = if ($chosen) { "$($chosen.Width)x$($chosen.Height)@$($chosen.RefreshRate)" } else { $null }
    }
    $reportPath = Join-Path $logRoot 'diagnostics.json'
    $diagnostics | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $reportPath -Encoding UTF8
    Write-LaunchLog "Primary=$device; original=$($original.dmPelsWidth)x$($original.dmPelsHeight)@$($original.dmDisplayFrequency); default-adapter D3D12=$canD3D12"
    if ($DiagnoseOnly) {
        Write-LaunchLog "Diagnostics saved to $reportPath. No game launched, profile edited, or display mode changed."
        $exitCode = 0
    }
    elseif ($ConfigureOnly) {
        if ($GraphicsBackend -notin @('Auto', 'Profile')) { $null = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend $GraphicsBackend }
        Write-LaunchLog 'Configuration completed. No game launched or display mode changed.'
        $exitCode = 0
    }
    else {
        if (-not $chosen) { throw 'No driver-tested compatible desktop mode is available. See .dw-launcher\diagnostics.json; try GPU scaling or another primary monitor. No display changes were made.' }
        if ($GraphicsBackend -eq 'D3D9On12' -and -not $canD3D12) {
            throw 'The default GPU cannot create a DirectX 12 device. Try Play Dominion Wars - Native Graphics.cmd. Hybrid GPU selection may also need adjustment in Windows Graphics settings.'
        }
        $profile = Set-DominionWarsGraphicsBackend -ProfilePath $profilePath -Backend $selectedBackend
        if ($FocusMode -eq 'Experimental') {
            $snapshot = ConvertTo-DominionWarsFocusSnapshot $original
            $null = Start-DominionWarsFocusSession -GameRoot $game.DirectoryName
            $focusActive = $true
            $env:DW_EXPERIMENTAL_FOCUS_GUARD = '1'
            $env:DW_EXPERIMENTAL_FOCUS_DESKTOP = '1'
            $env:DW_FOCUS_DEVICE = $device
            $env:DW_FOCUS_DEVICE_IDENTITY = [DominionWarsDisplayMode]::GetDeviceIdentity($device)
            $env:DW_FOCUS_ORIGINAL_MODE = $snapshot
            $profile = [IO.File]::ReadAllText($profilePath)
            $diagnostics.WrapperHash = Get-DominionWarsFileHash $wrapperPath
            Write-LaunchLog 'Experimental Alt+Tab enabled for this run. Pause manually; the original wrapper/profile will be restored afterward.'
        }
        if ($VisualPreset -ne 'Baseline') {
            $null = Start-DominionWarsVisualPreset -GameRoot $game.DirectoryName -Preset $VisualPreset
            $visualPresetActive = $true
            $profile = [IO.File]::ReadAllText($profilePath)
            Write-LaunchLog "Experimental $VisualPreset preset applied for this run only. The desktop mode and required compatibility fixes are unchanged."
        }
        if ($GraphicsBackend -eq 'Auto' -and $selectedBackend -eq 'Native') {
            Write-LaunchLog 'Automatic compatibility selection: the default adapter cannot create D3D12. Saved the system-default D3D9 path, preserving the texture fix. Hybrid-GPU selection may need a manual override.'
        }
        if ($GraphicsBackend -eq 'Profile' -and -not $canD3D12 -and $profile -match '(?im)^\s*ForceD3D9On12\s*=\s*on\s*(?:[#;].*)?$') {
            Write-LaunchLog 'Warning: the default GPU cannot create a D3D12 device. If startup fails, try Play Dominion Wars - Native Graphics.cmd; no automatic vendor-based selection was made.'
        }
        $diagnostics.Profile = $profile
        $diagnostics | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $reportPath -Encoding UTF8
        if ($additionalConfigs.Count -gt 0) {
            Write-LaunchLog 'Additional DDrawCompat configuration files detected. These can affect rendering; the overlay file can override this profile. See diagnostics and the final configuration in the DDrawCompat log.'
        }
        if (@($diagnostics.LegacyWindowsCompatibilityFlags | Where-Object { $_.Flags }).Count -gt 0) {
            Write-LaunchLog 'Legacy Windows compatibility flags were detected. They can force display/DPI behavior; inspect diagnostics before changing them. No registry settings were modified.'
        }
        Add-Content -LiteralPath $logPath -Value "Game-specific profile:`r`n$profile" -Encoding UTF8
        Write-LaunchLog "Selected $($chosen.Width)x$($chosen.Height)@$($chosen.RefreshRate); backend selection persists when explicitly changed."
        $script:sessionExitCode = 1
        $script:restoreFailed = $false
        $restoreState = [pscustomobject]@{
            Phase = 'Armed'; Device = $device; DeviceIdentity = [DominionWarsDisplayMode]::GetDeviceIdentity($device)
            OriginalMode = $original; TemporaryMode = $chosen.Mode
        }
        if (-not $NoRecoveryWatchdog) {
            $recovery = Start-DominionWarsRecovery -HelperPath (Join-Path $launcherRoot 'Restore-DominionWarsDesktop.ps1') `
                -Device $device -Original $original -Temporary $chosen.Mode -GameRoot $game.DirectoryName
            Write-LaunchLog "Independent display recovery armed; state=$($recovery.Path)"
        }
        Invoke-DominionWarsSession -Apply {
            if ($recovery) { $recovery.State.Phase = 'Armed'; Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State }
            [DominionWarsDisplayMode]::Apply($device, $chosen.Mode)
        } -Run {
            $process = Start-Process -FilePath $game.FullName -WorkingDirectory $game.DirectoryName -PassThru
            Write-LaunchLog "Game PID=$($process.Id)"
            $null = $process.Handle
            $process.WaitForExit(); $process.Refresh()
            $script:sessionExitCode = $process.ExitCode
            Write-LaunchLog "Game exit code=$script:sessionExitCode"
        } -Restore {
            try {
                $restoreResult = Restore-DominionWarsDesktopMode -State $restoreState
                if ($recovery) { $recovery.State.Phase = 'Complete'; Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State }
                if ($restoreResult -in @('Restore', 'AlreadyRestored')) { Write-LaunchLog "Original desktop mode restored ($restoreResult)." }
                else { Write-LaunchLog "Desktop restoration skipped ($restoreResult); preserving the current monitor or display settings." }
            }
            catch {
                $script:restoreFailed = $true
                Write-LaunchLog "RESTORATION FAILED: $($_.Exception.Message). Restore your resolution in Windows Settings > System > Display."
            }
        }
        $exitCode = if ($script:restoreFailed) { 1 } else { $script:sessionExitCode }
    }
}
catch { Write-LaunchLog "ERROR: $($_.Exception.Message)" }
finally {
    try {
        if ($focusActive) {
            $null = Restore-DominionWarsFocusSession -GameRoot $game.DirectoryName
            Write-LaunchLog 'Stock wrapper and prior profile restored after experimental Alt+Tab.'
        }
        if ($visualPresetActive) {
            $restoredPreset = Restore-DominionWarsVisualPreset -GameRoot $game.DirectoryName
            if ($restoredPreset -and $restoredPreset.PreservedChanges) {
                Write-LaunchLog "Profile changed during play; keeping the later changes. Original profile is at $($restoredPreset.BackupPath)."
            }
            else { Write-LaunchLog 'Original visual settings restored.' }
        }
        if ($recovery -and $recovery.State.Phase -eq 'Prepared') {
            $recovery.State.Phase = 'Complete'; Write-DominionWarsRecoveryState -Path $recovery.Path -State $recovery.State
        }
    }
    catch { $exitCode = 1; Write-LaunchLog "Cleanup failed: $($_.Exception.Message). Original backups remain under .dw-launcher\focus-backups or .dw-launcher\visual-presets; repair or the next launch can recover pending sessions." }
    finally {
        foreach ($name in $focusEnvironment.Keys) { [Environment]::SetEnvironmentVariable($name,$focusEnvironment[$name],'Process') }
        if ($recovery) { $recovery.Process.Dispose() }
        if ($ownsMutex) { $mutex.ReleaseMutex() }
        if ($mutex) { $mutex.Dispose() }
    }
}
exit $exitCode
