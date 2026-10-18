Set-StrictMode -Version 3.0

function Get-DominionWarsFileHash {
    param([Parameter(Mandatory = $true)][string]$Path)
    $stream = [IO.File]::OpenRead($Path)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-', '') }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}

function Initialize-DominionWarsDisplay {
    if ('DominionWarsDisplayMode' -as [type]) { return }
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public static class DominionWarsDisplayMode
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct DEVMODE
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string dmDeviceName;
        public short dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra;
        public int dmFields, dmPositionX, dmPositionY, dmDisplayOrientation, dmDisplayFixedOutput;
        public short dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string dmFormName;
        public short dmLogPixels;
        public int dmBitsPerPel, dmPelsWidth, dmPelsHeight, dmDisplayFlags, dmDisplayFrequency;
        public int dmICMMethod, dmICMIntent, dmMediaType, dmDitherType, dmReserved1, dmReserved2;
        public int dmPanningWidth, dmPanningHeight;
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct DISPLAY_DEVICE
    {
        public int cb;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string DeviceName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)] public string DeviceString;
        public int StateFlags;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)] public string DeviceID;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)] public string DeviceKey;
    }
    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern bool EnumDisplayDevices(string device, uint index, ref DISPLAY_DEVICE display, uint flags);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern bool EnumDisplaySettingsEx(string device, int index, ref DEVMODE mode, uint flags);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern int ChangeDisplaySettingsEx(string device, ref DEVMODE mode, IntPtr hwnd, uint flags, IntPtr param);
    [DllImport("d3d12.dll", ExactSpelling = true)]
    private static extern int D3D12CreateDevice(IntPtr adapter, uint featureLevel, ref Guid iid, out IntPtr device);

    private static DEVMODE EmptyMode()
    {
        var m = new DEVMODE(); m.dmSize = (short)Marshal.SizeOf(typeof(DEVMODE)); return m;
    }
    public static string GetPrimaryDevice()
    {
        for (uint i = 0; ; i++)
        {
            var d = new DISPLAY_DEVICE(); d.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
            if (!EnumDisplayDevices(null, i, ref d, 0)) break;
            if ((d.StateFlags & 5) == 5) return d.DeviceName;
        }
        throw new InvalidOperationException("No active primary display was found. Local console session required.");
    }
    public static DEVMODE GetCurrent(string device)
    {
        var m = EmptyMode();
        if (!EnumDisplaySettingsEx(device, -1, ref m, 0))
            throw new InvalidOperationException("Could not read the primary display mode.");
        return m;
    }
    public static DEVMODE[] GetModes(string device)
    {
        var modes = new List<DEVMODE>();
        for (int i = 0; ; i++)
        {
            var m = EmptyMode();
            if (!EnumDisplaySettingsEx(device, i, ref m, 0)) break;
            modes.Add(m);
        }
        return modes.ToArray();
    }
    public static string GetDeviceIdentity(string device)
    {
        for (uint i = 0; ; i++)
        {
            var d = new DISPLAY_DEVICE(); d.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
            if (!EnumDisplayDevices(null, i, ref d, 0)) break;
            if (d.DeviceName != device) continue;
            var monitor = new DISPLAY_DEVICE(); monitor.cb = Marshal.SizeOf(typeof(DISPLAY_DEVICE));
            EnumDisplayDevices(device, 0, ref monitor, 1);
            return d.DeviceID + "|" + monitor.DeviceID;
        }
        throw new InvalidOperationException("Saved display device is no longer attached.");
    }
    public static int Test(string device, DEVMODE mode)
    {
        return ChangeDisplaySettingsEx(device, ref mode, IntPtr.Zero, 2, IntPtr.Zero); // CDS_TEST
    }
    public static void Apply(string device, DEVMODE mode)
    {
        int result = ChangeDisplaySettingsEx(device, ref mode, IntPtr.Zero, 4, IntPtr.Zero); // CDS_FULLSCREEN, no registry change
        if (result != 0) throw new InvalidOperationException("Display driver rejected the mode (code " + result + ").");
    }
    public static bool CanCreateD3D12Device()
    {
        IntPtr device = IntPtr.Zero;
        try
        {
            var iid = new Guid("189819F1-1DB6-4B57-BE54-1821339B85F7");
            return D3D12CreateDevice(IntPtr.Zero, 0xB000, ref iid, out device) >= 0;
        }
        catch (DllNotFoundException) { return false; }
        catch (EntryPointNotFoundException) { return false; }
        finally { if (device != IntPtr.Zero) Marshal.Release(device); }
    }
}
'@
}

function Get-DominionWarsModeCandidates {
    param($Original, [object[]]$Modes, [int]$Width = 1280, [int]$Height = 960,
        [int]$RefreshRate = 0, [switch]$NoResolutionFallback)
    $sizes = @([pscustomobject]@{ Width = $Width; Height = $Height })
    if (-not $NoResolutionFallback) {
        $sizes += @([pscustomobject]@{ Width = 1024; Height = 768 },
            [pscustomobject]@{ Width = 800; Height = 600 },
            [pscustomobject]@{ Width = 640; Height = 480 })
        # Some laptops/TVs expose no 4:3 desktop modes. A real, narrow widescreen
        # mode is a candidate below the reported input threshold, not a validated
        # gameplay fix. DDrawCompat preserves the game aspect.
        if ($Original.dmPelsWidth -ge 800 -and $Original.dmPelsWidth -lt 1600 -and $Original.dmPelsHeight -ge 600) {
            $sizes += [pscustomobject]@{ Width = $Original.dmPelsWidth; Height = $Original.dmPelsHeight }
        }
        $sizes += @($Modes | Where-Object {
            $_.dmPelsWidth -ge 800 -and $_.dmPelsWidth -lt 1600 -and $_.dmPelsHeight -ge 600
        } | Sort-Object @{ Expression = { $_.dmPelsWidth * $_.dmPelsHeight }; Descending = $true } |
            ForEach-Object { [pscustomobject]@{ Width = $_.dmPelsWidth; Height = $_.dmPelsHeight } })
    }
    $seen = @{}
    $seenModes = @{}
    $preferredHz = if ($RefreshRate -gt 0) { $RefreshRate } else { $Original.dmDisplayFrequency }
    foreach ($size in $sizes) {
        $key = "$($size.Width)x$($size.Height)"
        if ($seen.ContainsKey($key)) { continue }
        $seen[$key] = $true
        # Use real modes with valid refresh rate, color depth and orientation.
        $Modes | Where-Object {
            $_.dmPelsWidth -eq $size.Width -and $_.dmPelsHeight -eq $size.Height -and
            $_.dmBitsPerPel -eq $Original.dmBitsPerPel -and
            $_.dmDisplayOrientation -eq $Original.dmDisplayOrientation
        } | Sort-Object @{ Expression = { [Math]::Abs($_.dmDisplayFrequency - $preferredHz) } } |
            ForEach-Object {
                $modeKey = "$key@$($_.dmDisplayFrequency):$($_.dmBitsPerPel):$($_.dmDisplayOrientation)"
                if (-not $seenModes.ContainsKey($modeKey)) { $seenModes[$modeKey] = $true; $_ }
            }
    }
}

function Get-DominionWarsRecoveryPath {
    $sessionId = [Diagnostics.Process]::GetCurrentProcess().SessionId
    return Join-Path $env:LOCALAPPDATA "Dominion Wars Modernizer\DisplayRecovery\session-$sessionId.json"
}

function Write-DominionWarsRecoveryState {
    param([string]$Path, $State)
    New-Item -ItemType Directory -Path (Split-Path -Parent $Path) -Force | Out-Null
    $State.UpdatedUtc = [DateTime]::UtcNow.ToString('o')
    $temporary = "$Path.$PID.new"
    [IO.File]::WriteAllText($temporary, ($State | ConvertTo-Json -Depth 6), (New-Object Text.UTF8Encoding($false)))
    for ($attempt = 0; $attempt -lt 5; $attempt++) {
        try {
            if ([IO.File]::Exists($Path)) { [IO.File]::Replace($temporary, $Path, [NullString]::Value) }
            else { [IO.File]::Move($temporary, $Path) }
            return
        }
        catch { if ($attempt -eq 4) { throw }; Start-Sleep -Milliseconds 100 }
    }
}

function Test-DominionWarsProcessIdentity {
    param([int]$ProcessId, [string]$StartTicks)
    try {
        $process = [Diagnostics.Process]::GetProcessById($ProcessId)
        try { return -not $process.HasExited -and $process.StartTime.ToUniversalTime().Ticks.ToString() -eq $StartTicks }
        finally { $process.Dispose() }
    }
    catch { return $false }
}

function Get-DominionWarsRecoveryDecision {
    param($State, $CurrentMode, [string]$DeviceIdentity, [bool]$ParentAlive, [bool]$GameAlive)
    if ($State.Phase -notin @('Prepared', 'Armed', 'Complete')) { throw 'Unrecognized desktop recovery phase; refusing to change the display.' }
    if ($State.Phase -eq 'Complete') { return 'Complete' }
    if ($ParentAlive -or $GameAlive) { return 'Wait' }
    if ($State.Phase -eq 'Prepared') { return 'NotArmed' }
    if ($DeviceIdentity -ne $State.DeviceIdentity) { return 'DisplayChanged' }
    $original = $State.OriginalMode; $temporary = $State.TemporaryMode
    $fields = @('dmPelsWidth', 'dmPelsHeight', 'dmDisplayFrequency', 'dmBitsPerPel', 'dmDisplayOrientation',
        'dmPositionX', 'dmPositionY')
    $matchesOriginal = $true; $matchesTemporary = $true
    foreach ($field in $fields) {
        if ($CurrentMode.$field -ne $original.$field) { $matchesOriginal = $false }
        if ($CurrentMode.$field -ne $temporary.$field) { $matchesTemporary = $false }
    }
    if ($matchesOriginal) { return 'AlreadyRestored' }
    if ($matchesTemporary) { return 'Restore' }
    return 'UserModeChanged'
}

function Restore-DominionWarsDesktopMode {
    param([Parameter(Mandatory = $true)]$State)
    Initialize-DominionWarsDisplay
    # Normal cleanup and orphan recovery must make the same conservative choice.
    # Never apply an old mode to a replacement monitor or overwrite a display
    # adjustment made since launch, even when the original launcher is alive.
    if ([DominionWarsDisplayMode]::GetPrimaryDevice() -ne $State.Device) { return 'DisplayChanged' }
    $identity = [DominionWarsDisplayMode]::GetDeviceIdentity($State.Device)
    $current = [DominionWarsDisplayMode]::GetCurrent($State.Device)
    $decision = Get-DominionWarsRecoveryDecision -State $State -CurrentMode $current `
        -DeviceIdentity $identity -ParentAlive $false -GameAlive $false
    if ($decision -eq 'Restore') {
        [DominionWarsDisplayMode]::Apply($State.Device, (ConvertFrom-DominionWarsModeSnapshot $State.OriginalMode))
    }
    return $decision
}

function ConvertFrom-DominionWarsModeSnapshot {
    param($Snapshot)
    Initialize-DominionWarsDisplay
    $mode = New-Object 'DominionWarsDisplayMode+DEVMODE'
    foreach ($field in ([type]'DominionWarsDisplayMode+DEVMODE').GetFields()) {
        $name = $field.Name
        if ($Snapshot.PSObject.Properties.Name -contains $name) { $mode.$name = $Snapshot.$name }
    }
    return $mode
}

function Start-DominionWarsRecovery {
    param([string]$HelperPath, [string]$Device, $Original, $Temporary, [string]$GameRoot, [string]$StatePath)
    if (-not (Test-Path -LiteralPath $HelperPath -PathType Leaf)) { throw 'Display recovery helper is missing. Run repair before launching.' }
    $path = if ($StatePath) { $StatePath } else { Get-DominionWarsRecoveryPath }
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $old = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
        if ($old.Phase -notin @('Complete', 'Prepared')) {
            $current = [DominionWarsDisplayMode]::GetCurrent($old.Device)
            $identity = [DominionWarsDisplayMode]::GetDeviceIdentity($old.Device)
            $decision = Get-DominionWarsRecoveryDecision -State $old -CurrentMode $current -DeviceIdentity $identity -ParentAlive $false -GameAlive $false
            if ($decision -ne 'AlreadyRestored') {
                throw 'An unfinished desktop recovery record exists. Run Recover Dominion Wars Desktop.cmd before launching again.'
            }
        }
    }
    $parent = [Diagnostics.Process]::GetCurrentProcess()
    $state = [pscustomobject]@{
        SchemaVersion = 1; Phase = 'Prepared'; Ready = $false; UpdatedUtc = ''
        ParentPid = $PID; ParentStartTicks = $parent.StartTime.ToUniversalTime().Ticks.ToString()
        SessionId = $parent.SessionId; Device = $Device; DeviceIdentity = [DominionWarsDisplayMode]::GetDeviceIdentity($Device)
        OriginalMode = $Original; TemporaryMode = $Temporary; GameRoot = $GameRoot
    }
    Write-DominionWarsRecoveryState -Path $path -State $state
    $arguments = '-NoLogo -NoProfile -ExecutionPolicy Bypass -File "' + $HelperPath + '" -StatePath "' + $path + '"'
    $process = Start-Process -FilePath (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') `
        -ArgumentList $arguments -WindowStyle Hidden -PassThru
    $timer = [Diagnostics.Stopwatch]::StartNew()
    while ($timer.Elapsed.TotalSeconds -lt 8) {
        if ($process.HasExited) { throw 'Display recovery helper exited before becoming ready. No display change was made.' }
        $saved = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
        if ($saved.Ready) { $state.Ready = $true; return [pscustomobject]@{ Path = $path; State = $state; Process = $process } }
        Start-Sleep -Milliseconds 100
    }
    $state.Phase = 'Complete'; Write-DominionWarsRecoveryState -Path $path -State $state
    throw 'Display recovery helper did not become ready in time. No display change was made.'
}

function Reset-DominionWarsProfile {
    param([string]$GameRoot)
    $recordPath = Join-Path $GameRoot '.dw-launcher\profile-reset.json'
    if (Test-Path -LiteralPath $recordPath -PathType Leaf) { throw 'A previous reset is still available to undo. Undo it before resetting again; originals remain in .dw-launcher\profile-backups.' }
    $backupRoot = Join-Path $GameRoot ('.dw-launcher\profile-backups\' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    $names = @('DDrawCompat-ds9dw.ini', 'DDrawCompat.ini', 'DDrawCompatOverlay-ds9dw.ini')
    $records = @()
    # Copy all originals successfully before changing any local configuration.
    foreach ($name in $names) {
        $path = Join-Path $GameRoot $name
        $exists = Test-Path -LiteralPath $path -PathType Leaf
        if ($exists) { Copy-Item -LiteralPath $path -Destination (Join-Path $backupRoot $name) }
        $records += [pscustomobject]@{ Name = $name; Existed = $exists; AppliedHash = $null }
    }
    $recipe = @(
        '# Dominion Wars modern Windows compatibility profile',
        'EnableDDrawCompat = on', 'FullscreenMode = borderless', 'DisplayAspectRatio = app',
        'PalettizedTextures = on', 'CompatFixes = singlemonitor',
        'CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps|=0x2', 'ForceD3D9On12 = on'
    ) -join "`r`n"
    $profilePath = Join-Path $GameRoot $names[0]
    try {
        [IO.File]::WriteAllText($profilePath, ($recipe + "`r`n"), (New-Object Text.UTF8Encoding($false)))
        $records[0].AppliedHash = Get-DominionWarsFileHash $profilePath
        foreach ($name in $names[1..2]) {
            $path = Join-Path $GameRoot $name
            if (Test-Path -LiteralPath $path -PathType Leaf) { Remove-Item -LiteralPath $path }
        }
        [pscustomobject]@{ SchemaVersion = 1; BackupRoot = $backupRoot; Files = $records } |
            ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $recordPath -Encoding UTF8
    }
    catch {
        $failure = $_.Exception.Message
        foreach ($file in $records) {
            $path = Join-Path $GameRoot $file.Name
            if ($file.Existed) { Copy-Item -LiteralPath (Join-Path $backupRoot $file.Name) -Destination $path -Force -ErrorAction Continue }
            elseif (Test-Path -LiteralPath $path -PathType Leaf) { Remove-Item -LiteralPath $path -ErrorAction Continue }
        }
        throw "Profile reset failed: $failure. Original copies are preserved at '$backupRoot'."
    }
    return $backupRoot
}

function Restore-DominionWarsProfile {
    param([string]$GameRoot)
    $recordPath = Join-Path $GameRoot '.dw-launcher\profile-reset.json'
    if (-not (Test-Path -LiteralPath $recordPath -PathType Leaf)) { throw 'No previous profile reset is available to undo.' }
    $record = Get-Content -LiteralPath $recordPath -Raw | ConvertFrom-Json
    $expectedRoot = [IO.Path]::GetFullPath((Join-Path $GameRoot '.dw-launcher\profile-backups')) + '\'
    if (-not ([IO.Path]::GetFullPath($record.BackupRoot) + '\').StartsWith($expectedRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Profile backup path is outside this installation.'
    }
    foreach ($file in $record.Files) {
        if ($file.Name -notin @('DDrawCompat-ds9dw.ini', 'DDrawCompat.ini', 'DDrawCompatOverlay-ds9dw.ini')) { throw 'Unrecognized profile backup entry.' }
        $path = Join-Path $GameRoot $file.Name
        if ((Test-Path -LiteralPath $path -PathType Leaf) -and
            (-not $file.AppliedHash -or (Get-DominionWarsFileHash $path) -ne $file.AppliedHash)) {
            throw "'$($file.Name)' changed after the reset. Original backups are kept at '$($record.BackupRoot)'; refusing to overwrite newer changes."
        }
        if ($file.Existed -and -not (Test-Path -LiteralPath (Join-Path $record.BackupRoot $file.Name) -PathType Leaf)) { throw "Original profile backup is missing: $($file.Name)" }
    }
    foreach ($file in $record.Files) {
        $path = Join-Path $GameRoot $file.Name
        if ($file.Existed) { Copy-Item -LiteralPath (Join-Path $record.BackupRoot $file.Name) -Destination $path -Force }
        elseif (Test-Path -LiteralPath $path -PathType Leaf) { Remove-Item -LiteralPath $path }
    }
    Remove-Item -LiteralPath $recordPath
    return $record.BackupRoot
}

function Get-DominionWarsLegacyFlags {
    param([string]$GamePath)
    $records = @()
    foreach ($hive in @([Microsoft.Win32.RegistryHive]::CurrentUser, [Microsoft.Win32.RegistryHive]::LocalMachine)) {
        foreach ($view in @([Microsoft.Win32.RegistryView]::Registry32, [Microsoft.Win32.RegistryView]::Registry64)) {
            $base = $null; $key = $null
            try {
                $base = [Microsoft.Win32.RegistryKey]::OpenBaseKey($hive, $view)
                $key = $base.OpenSubKey('Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers')
                if ($key) {
                    $flags = $key.GetValue($GamePath)
                    if ($flags) { $records += [pscustomobject]@{ Source = "$hive/$view"; Flags = [string]$flags; Error = $null } }
                }
            }
            catch { $records += [pscustomobject]@{ Source = "$hive/$view"; Flags = $null; Error = $_.Exception.Message } }
            finally { if ($key) { $key.Dispose() }; if ($base) { $base.Dispose() } }
        }
    }
    $inherited = [Environment]::GetEnvironmentVariable('__COMPAT_LAYER', 'Process')
    if ($inherited) { $records += [pscustomobject]@{ Source = 'Inherited __COMPAT_LAYER'; Flags = $inherited; Error = $null } }
    return $records
}

function Get-DominionWarsVisualSettings {
    param([ValidateSet('Baseline', 'Sharper', 'Enhanced3D', 'ResourceSaving')][string]$Preset)
    # Each preset isolates one change; none touches the required texture fixes,
    # graphics backend, CPU affinity, mouse settings, or desktop resolution.
    $settings = [ordered]@{
        DisplayFilter = 'bilinear(0)'; ResolutionScale = 'app(1)'
        Antialiasing = 'off'; FpsLimiter = 'off'
    }
    switch ($Preset) {
        'Sharper' { $settings.DisplayFilter = 'bicubic(0)' }
        'Enhanced3D' { $settings.ResolutionScale = 'app(2)' }
        'ResourceSaving' { $settings.FpsLimiter = 'flipstart(60)' }
    }
    return $settings
}

function Start-DominionWarsVisualPreset {
    param([string]$GameRoot,
        [ValidateSet('Baseline', 'Sharper', 'Enhanced3D', 'ResourceSaving')][string]$Preset)
    if ($Preset -eq 'Baseline') { return $null }
    $profilePath = Join-Path $GameRoot 'DDrawCompat-ds9dw.ini'
    $recordPath = Join-Path $GameRoot '.dw-launcher\visual-preset.json'
    if (Test-Path -LiteralPath $recordPath) { throw 'An unfinished visual preset must be recovered before starting another.' }
    if (-not (Test-Path -LiteralPath $profilePath -PathType Leaf)) { throw 'Compatibility profile is missing. Run repair first.' }
    $backupRoot = Join-Path $GameRoot ('.dw-launcher\visual-presets\' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    $backupPath = Join-Path $backupRoot 'original.ini'
    Copy-Item -LiteralPath $profilePath -Destination $backupPath
    $originalHash = Get-DominionWarsFileHash $backupPath
    $text = [IO.File]::ReadAllText($profilePath)
    foreach ($entry in (Get-DominionWarsVisualSettings $Preset).GetEnumerator()) {
        $pattern = '(?im)^[\t ]*' + [regex]::Escape($entry.Key) + '[\t ]*=.*$'
        $line = $entry.Key + ' = ' + $entry.Value
        if ([regex]::IsMatch($text, $pattern)) { $text = [regex]::Replace($text, $pattern, $line) }
        else { $text = $text.TrimEnd() + "`r`n$line`r`n" }
    }
    $bytes = (New-Object Text.UTF8Encoding($false)).GetBytes($text)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try { $appliedHash = [BitConverter]::ToString($algorithm.ComputeHash($bytes)).Replace('-', '') }
    finally { $algorithm.Dispose() }
    $record = [pscustomobject]@{
        SchemaVersion = 1; Preset = $Preset; BackupPath = $backupPath
        OriginalHash = $originalHash; AppliedHash = $appliedHash; UpdatedUtc = ''
    }
    # Record the backup and intended result before touching the profile so a
    # killed launcher can recover on the next ordinary launch.
    Write-DominionWarsRecoveryState -Path $recordPath -State $record
    try { [IO.File]::WriteAllBytes($profilePath, $bytes) }
    catch {
        Copy-Item -LiteralPath $backupPath -Destination $profilePath -Force
        Remove-Item -LiteralPath $recordPath
        throw
    }
    return $record
}

function Restore-DominionWarsVisualPreset {
    param([string]$GameRoot)
    $recordPath = Join-Path $GameRoot '.dw-launcher\visual-preset.json'
    if (-not (Test-Path -LiteralPath $recordPath -PathType Leaf)) { return $null }
    $record = Get-Content -LiteralPath $recordPath -Raw | ConvertFrom-Json
    $expectedRoot = [IO.Path]::GetFullPath((Join-Path $GameRoot '.dw-launcher\visual-presets')) + '\'
    if ($record.SchemaVersion -ne 1 -or
        -not [IO.Path]::GetFullPath($record.BackupPath).StartsWith($expectedRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Visual preset recovery record is invalid or outside this installation.'
    }
    if ((Get-DominionWarsFileHash $record.BackupPath) -ne $record.OriginalHash) {
        throw 'Visual preset backup has changed; refusing to restore it.'
    }
    $profilePath = Join-Path $GameRoot 'DDrawCompat-ds9dw.ini'
    $currentHash = if (Test-Path -LiteralPath $profilePath -PathType Leaf) { Get-DominionWarsFileHash $profilePath } else { $null }
    $preservedChanges = $false
    if ($currentHash -eq $record.AppliedHash) {
        Copy-Item -LiteralPath $record.BackupPath -Destination $profilePath -Force
    }
    elseif ($currentHash -ne $record.OriginalHash) {
        # Keep later edits or an intentional deletion, rather than overwriting
        # the user. Archive the record and retain the original backup.
        $preservedChanges = $true
        Copy-Item -LiteralPath $recordPath -Destination (Join-Path (Split-Path -Parent $record.BackupPath) 'conflict.json')
    }
    Remove-Item -LiteralPath $recordPath
    return [pscustomobject]@{ PreservedChanges = $preservedChanges; BackupPath = $record.BackupPath; Preset = $record.Preset }
}

function Resolve-DominionWarsGraphicsBackend {
    param([string]$Profile, [ValidateSet('Auto', 'Profile', 'Native', 'D3D9On12')][string]$Backend,
        [bool]$CanCreateD3D12)
    if ($Backend -ne 'Auto') { return $Backend }
    # Keep the user's selection unless it forces a path the default adapter
    # cannot create. No vendor guesses or crash-triggered relaunches.
    if (-not $CanCreateD3D12 -and $Profile -match '(?im)^\s*ForceD3D9On12\s*=\s*on\s*(?:[#;].*)?$') {
        return 'Native'
    }
    return 'Profile'
}

function Set-DominionWarsGraphicsBackend {
    param([string]$ProfilePath, [ValidateSet('Profile', 'Native', 'D3D9On12')][string]$Backend)
    if (-not (Test-Path -LiteralPath $ProfilePath -PathType Leaf)) {
        throw 'Compatibility profile is missing. Run Repair Dominion Wars.cmd first.'
    }
    $text = [IO.File]::ReadAllText($ProfilePath)
    if ($Backend -ne 'Profile') {
        $value = if ($Backend -eq 'Native') { 'off' } else { 'on' }
        $pattern = '(?im)^\s*ForceD3D9On12\s*=.*$'
        if ([regex]::IsMatch($text, $pattern)) {
            $text = [regex]::Replace($text, $pattern, "ForceD3D9On12 = $value")
        }
        else { $text = $text.TrimEnd() + "`r`nForceD3D9On12 = $value`r`n" }
        [IO.File]::WriteAllText($ProfilePath, $text, (New-Object Text.UTF8Encoding($false)))
    }
    return $text
}

function Invoke-DominionWarsSession {
    # Injected operations let tests cover failures without touching the desktop.
    param([Parameter(Mandatory = $true)][scriptblock]$Apply,
        [Parameter(Mandatory = $true)][scriptblock]$Run,
        [Parameter(Mandatory = $true)][scriptblock]$Restore)
    try { & $Apply; & $Run }
    finally { & $Restore }
}

Export-ModuleMember -Function Initialize-DominionWarsDisplay, Get-DominionWarsModeCandidates,
    Set-DominionWarsGraphicsBackend, Invoke-DominionWarsSession, Get-DominionWarsFileHash,
    Get-DominionWarsRecoveryPath, Write-DominionWarsRecoveryState, Test-DominionWarsProcessIdentity,
    Get-DominionWarsRecoveryDecision, ConvertFrom-DominionWarsModeSnapshot, Start-DominionWarsRecovery
Export-ModuleMember -Function Restore-DominionWarsDesktopMode
Export-ModuleMember -Function Reset-DominionWarsProfile, Restore-DominionWarsProfile
Export-ModuleMember -Function Get-DominionWarsLegacyFlags
Export-ModuleMember -Function Resolve-DominionWarsGraphicsBackend
Export-ModuleMember -Function Get-DominionWarsVisualSettings, Start-DominionWarsVisualPreset, Restore-DominionWarsVisualPreset
