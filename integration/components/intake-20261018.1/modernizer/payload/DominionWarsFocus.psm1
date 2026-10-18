Set-StrictMode -Version 3.0

$script:BaselineHash = 'F75F0AC48D2782F225C483DC2F1142303A513E8DD8A60793891ADE89F64755EA'
$script:FocusHash = '3AA11B1C81A21C6EF5FB85D2DC31DEEC7EB8217F637094E9ED7B9F6D6BC90490'
$script:GameHash = '03C20A3535D0209111FF9F3B90EBA403E059DF2BE7E784D2746ABF1535C1BE6A'


function Get-FocusFileHash {
    param([string]$Path)
    $stream=[IO.File]::OpenRead($Path)
    $algorithm=[Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','') }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}

function Write-FocusRecord {
    param([string]$Path,$State)
    $State.UpdatedUtc=[DateTime]::UtcNow.ToString('o')
    $temporary=$Path+'.new'
    $existing=Get-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
    if ($existing -and ($existing.Attributes -band [IO.FileAttributes]::ReparsePoint)) { throw 'Redirected focus record temporary file rejected.' }
    [IO.File]::WriteAllText($temporary,($State | ConvertTo-Json -Depth 5),(New-Object Text.UTF8Encoding($false)))
    Move-Item -LiteralPath $temporary -Destination $Path -Force
}

function Get-DominionWarsFocusEnvironmentNames {
    @('DW_EXPERIMENTAL_FOCUS_GUARD','DW_EXPERIMENTAL_FOCUS_DESKTOP','DW_FOCUS_DEVICE','DW_FOCUS_DEVICE_IDENTITY','DW_FOCUS_ORIGINAL_MODE')
}

function ConvertTo-DominionWarsFocusSnapshot {
    param([Parameter(Mandatory=$true)]$Mode)
    $size = [Runtime.InteropServices.Marshal]::SizeOf($Mode)
    if ($size -ne 220 -or $Mode.dmSize -ne 220 -or $Mode.dmDriverExtra -ne 0) {
        throw 'Experimental Alt+Tab requires a complete 220-byte DEVMODEW without private driver data. Use the normal launcher instead.'
    }
    $pointer = [Runtime.InteropServices.Marshal]::AllocHGlobal($size)
    try {
        $bytes = New-Object byte[] $size
        [Runtime.InteropServices.Marshal]::Copy($bytes, 0, $pointer, $size)
        [Runtime.InteropServices.Marshal]::StructureToPtr($Mode, $pointer, $false)
        [Runtime.InteropServices.Marshal]::Copy($pointer, $bytes, 0, $size)
        return [BitConverter]::ToString($bytes).Replace('-', '')
    }
    finally { [Runtime.InteropServices.Marshal]::FreeHGlobal($pointer) }
}

function Assert-FocusPath {
    param([string]$GameRoot, [string]$BackupId)
    if ($BackupId -notmatch '^[a-f0-9]{32}$') { throw 'Invalid experimental Alt+Tab recovery identifier.' }
    $path = $GameRoot
    foreach ($part in @('.dw-launcher','focus-backups',$BackupId)) {
        $path = Join-Path $path $part
        if ((Test-Path -LiteralPath $path) -and ((Get-Item -LiteralPath $path -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
            throw 'Experimental Alt+Tab recovery refuses redirected backup directories.'
        }
    }
    return $path
}

function Copy-FocusFileAtomic {
    param([string]$Source, [string]$Target, [string]$BackupRoot)
    $temporary = Join-Path $BackupRoot ([guid]::NewGuid().ToString('N') + '.swap')
    try {
        Copy-Item -LiteralPath $Source -Destination $temporary
        # All files stay on the game volume. Replace is atomic and refuses a
        # missing destination, preserving deliberate deletions during recovery.
        [IO.File]::Replace($temporary, $Target, [NullString]::Value)
    }
    finally { if (Test-Path -LiteralPath $temporary) { Remove-Item -LiteralPath $temporary } }
}

function Restore-DominionWarsFocusSession {
    param([Parameter(Mandatory=$true)][string]$GameRoot, [switch]$AllowConflicts)
    $recordPath = Join-Path $GameRoot '.dw-launcher\focus-session.json'
    if (-not (Test-Path -LiteralPath $recordPath -PathType Leaf)) { return $null }
    if ((Get-Item -LiteralPath $recordPath -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected focus recovery record rejected.' }
    $record = Get-Content -LiteralPath $recordPath -Raw | ConvertFrom-Json
    if ($record.SchemaVersion -ne 1 -or $record.Phase -notin @('Prepared','Applied')) { throw 'Unrecognized experimental Alt+Tab recovery record.' }
    $backupRoot = Assert-FocusPath -GameRoot $GameRoot -BackupId $record.BackupId
    $specs = @(
        @{ Name='ddraw.dll'; Backup='original-ddraw.dll'; Original=$record.OriginalWrapperHash; Applied=$record.AppliedWrapperHash },
        @{ Name='DDrawCompat-ds9dw.ini'; Backup='original.ini'; Original=$record.OriginalProfileHash; Applied=$record.AppliedProfileHash }
    )
    if ($record.OriginalWrapperHash -ne $script:BaselineHash -or $record.AppliedWrapperHash -ne $script:FocusHash) { throw 'Unexpected focus recovery wrapper identities.' }
    # Validate every backup before restoring either file.
    foreach ($spec in $specs) {
        $backup = Join-Path $backupRoot $spec.Backup
        if ($spec.Original -notmatch '^[A-Fa-f0-9]{64}$' -or $spec.Applied -notmatch '^[A-Fa-f0-9]{64}$' -or
            -not (Test-Path -LiteralPath $backup -PathType Leaf) -or
            ((Get-Item -LiteralPath $backup -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) -or
            (Get-FocusFileHash $backup) -ne $spec.Original) { throw 'Experimental Alt+Tab backup is missing or changed; refusing recovery.' }
    }
    $conflicts = @()
    foreach ($spec in $specs) {
        $target = Join-Path $GameRoot $spec.Name
        $current = if (Test-Path -LiteralPath $target -PathType Leaf) { Get-FocusFileHash $target } else { $null }
        if ($current -eq $spec.Applied -and $current -ne $spec.Original) {
            if ((Get-Item -LiteralPath $target -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected focus target rejected.' }
            Copy-FocusFileAtomic -Source (Join-Path $backupRoot $spec.Backup) -Target $target -BackupRoot $backupRoot
        }
        elseif ($current -ne $spec.Original) { $conflicts += $spec.Name }
    }
    if ($conflicts.Count) {
        Copy-Item -LiteralPath $recordPath -Destination (Join-Path $backupRoot 'conflict.json') -Force
        if (-not $AllowConflicts) { throw "Experimental Alt+Tab recovery preserved changed/deleted files: $($conflicts -join ', '). Run repair before launching; backups are at $backupRoot" }
    }
    Remove-Item -LiteralPath $recordPath
    return [pscustomobject]@{ BackupPath=$backupRoot; PreservedChanges=($conflicts.Count -gt 0); Conflicts=$conflicts }
}

function Start-DominionWarsFocusSession {
    param([Parameter(Mandatory=$true)][string]$GameRoot)
    $recordPath = Join-Path $GameRoot '.dw-launcher\focus-session.json'
    if (Test-Path -LiteralPath $recordPath) { throw 'Recover the unfinished experimental Alt+Tab session first.' }
    $wrapper = Join-Path $GameRoot 'ddraw.dll'
    $focus = Join-Path $GameRoot 'DDrawCompat-Focus.dll'
    $profile = Join-Path $GameRoot 'DDrawCompat-ds9dw.ini'
    if ((Get-FocusFileHash (Join-Path $GameRoot 'ds9dw.exe')) -ne $script:GameHash) { throw 'Experimental Alt+Tab supports only the verified North American 1.04 executable.' }
    if ((Get-FocusFileHash $wrapper) -ne $script:BaselineHash -or (Get-FocusFileHash $focus) -ne $script:FocusHash) { throw 'Unexpected experimental or baseline wrapper. Run repair first.' }
    foreach ($file in @($wrapper,$focus,$profile)) {
        if ((Get-Item -LiteralPath $file -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Experimental Alt+Tab refuses redirected compatibility files.' }
        $handle = [IO.File]::Open($file,[IO.FileMode]::Open,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
        $handle.Dispose()
    }
    $id = [guid]::NewGuid().ToString('N')
    $backupRoot = Assert-FocusPath -GameRoot $GameRoot -BackupId $id
    New-Item -ItemType Directory -Path $backupRoot -Force | Out-Null
    Copy-Item -LiteralPath $wrapper -Destination (Join-Path $backupRoot 'original-ddraw.dll')
    Copy-Item -LiteralPath $profile -Destination (Join-Path $backupRoot 'original.ini')
    $text = [IO.File]::ReadAllText($profile)
    $pattern = '(?im)^[\t ]*AltTabFix[\t ]*=.*$'
    if ([regex]::IsMatch($text,$pattern)) { $text = [regex]::Replace($text,$pattern,'AltTabFix = noactivateapp(0)') }
    else { $text = $text.TrimEnd() + "`r`nAltTabFix = noactivateapp(0)`r`n" }
    $appliedProfile = Join-Path $backupRoot 'applied.ini'
    [IO.File]::WriteAllText($appliedProfile,$text,(New-Object Text.UTF8Encoding($false)))
    $record = [pscustomobject]@{
        SchemaVersion=1; Phase='Prepared'; BackupId=$id; UpdatedUtc=''
        OriginalWrapperHash=Get-FocusFileHash $wrapper; AppliedWrapperHash=$script:FocusHash
        OriginalProfileHash=Get-FocusFileHash $profile; AppliedProfileHash=Get-FocusFileHash $appliedProfile
    }
    Write-FocusRecord -Path $recordPath -State $record
    try {
        Copy-FocusFileAtomic -Source $focus -Target $wrapper -BackupRoot $backupRoot
        Copy-FocusFileAtomic -Source $appliedProfile -Target $profile -BackupRoot $backupRoot
        $record.Phase='Applied'; Write-FocusRecord -Path $recordPath -State $record
    }
    catch { $null=Restore-DominionWarsFocusSession -GameRoot $GameRoot; throw }
    return $record
}

Export-ModuleMember -Function Get-DominionWarsFocusEnvironmentNames, ConvertTo-DominionWarsFocusSnapshot, Start-DominionWarsFocusSession, Restore-DominionWarsFocusSession
