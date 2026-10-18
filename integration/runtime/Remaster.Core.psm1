Set-StrictMode -Version 3.0
$ErrorActionPreference='Stop'

function Get-RemasterHash([string]$Path) {
    $stream=[IO.File]::OpenRead($Path);$algorithm=[Security.Cryptography.SHA256]::Create()
    try {[BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','').ToLowerInvariant()}
    finally {$stream.Dispose();$algorithm.Dispose()}
}
function Assert-RemasterHash([string]$Path,[string]$Expected) {
    if(-not(Test-Path -LiteralPath $Path -PathType Leaf) -or (Get-RemasterHash $Path) -ne $Expected.ToLowerInvariant()) {throw "Missing or unexpected file: $Path"}
}
function Get-RemasterChild([string]$Root,[string]$Relative) {
    if([IO.Path]::IsPathRooted($Relative) -or $Relative -match '(^|[\\/])\.\.([\\/]|$)' -or $Relative.Contains(':')) {throw 'Unsafe relative path'}
    $prefix=[IO.Path]::GetFullPath($Root).TrimEnd('\')+'\'
    $full=[IO.Path]::GetFullPath((Join-Path $Root $Relative))
    if(-not $full.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){throw 'Path escapes its root'}
    return $full
}
function Assert-RemasterPlainPath([string]$Path) {
    $full=[IO.Path]::GetFullPath($Path)
    if($full.TrimEnd('\') -eq [IO.Path]::GetPathRoot($full).TrimEnd('\')) {throw 'A drive root is not an installation directory'}
    $item=$full
    while($item) {
        if(Test-Path -LiteralPath $item) {
            if((Get-Item -LiteralPath $item -Force).Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Reparse paths are not accepted: $item"}
        }
        $parent=Split-Path -Parent $item
        if($parent -eq $item){break};$item=$parent
    }
    return $full.TrimEnd('\')
}
function Assert-RemasterTree([string]$Root) {
    $null=Assert-RemasterPlainPath $Root
    if(@(Get-ChildItem -LiteralPath $Root -Force -Recurse | Where-Object {$_.Attributes -band [IO.FileAttributes]::ReparsePoint}).Count) {throw 'Reparse entry in installation tree'}
}
function Assert-RemasterIdle {
    if(@(Get-Process -Name ds9dw -ErrorAction SilentlyContinue).Count) {throw 'Close all Dominion Wars game sessions first.'}
}
function Read-RemasterJson([string]$Path) {Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json}
function Write-RemasterJson([string]$Path,$Value) {
    $text=$Value | ConvertTo-Json -Depth 20
    [IO.File]::WriteAllText($Path,$text,(New-Object Text.UTF8Encoding($false)))
}
function Assert-RemasterPackage([string]$Root) {
    Assert-RemasterTree $Root
    $manifest=Read-RemasterJson (Join-Path $Root 'manifest.json')
    if($manifest.schema -ne 1 -or $manifest.product -ne 'Dominion Wars Remaster'){throw 'Unsupported package manifest'}
    foreach($record in $manifest.files) {
        $path=Get-RemasterChild $Root $record.path
        Assert-RemasterHash $path $record.sha256
        if((Get-Item -LiteralPath $path).Length -ne $record.size){throw 'Package size mismatch'}
    }
    $allowed=@('manifest.json','CHECKSUMS.json')+@($manifest.files | ForEach-Object {$_.path.Replace('/','\')})
    foreach($file in Get-ChildItem -LiteralPath $Root -Recurse -File -Force) {
        $relative=$file.FullName.Substring($Root.TrimEnd('\').Length+1)
        if($relative -notin $allowed){throw "Unexpected package file: $relative"}
    }
    return $manifest
}
function Invoke-RemasterDelta([string]$InputPath,[string]$DeltaPath,[string]$OutputPath) {
    if(Test-Path -LiteralPath $OutputPath){throw 'Delta output must be new'}
    $delta=Read-RemasterJson $DeltaPath
    if($delta.schema -ne 1 -or $delta.output_size -le 0 -or $delta.output_size -gt 128MB){throw 'Unsupported delta size/schema'}
    Assert-RemasterHash $InputPath $delta.input_sha256
    $source=[IO.File]::ReadAllBytes($InputPath)
    if($source.Length -ne $delta.input_size){throw 'Delta input size mismatch'}
    $output=New-Object IO.MemoryStream
    try {
        foreach($op in $delta.operations) {
            if($op.kind -eq 'copy') {
                $offset=[long]$op.offset;$length=[long]$op.length
                if($offset -lt 0 -or $length -le 0 -or $offset -gt $source.Length -or $length -gt ($source.Length-$offset)){throw 'Invalid delta copy range'}
                $output.Write($source,[int]$offset,[int]$length)
            } elseif($op.kind -eq 'bytes') {
                $bytes=[Convert]::FromBase64String([string]$op.data)
                $output.Write($bytes,0,$bytes.Length)
            } else {throw 'Unknown delta operation'}
            if($output.Length -gt $delta.output_size){throw 'Delta output overrun'}
        }
        if($output.Length -ne $delta.output_size){throw 'Delta output size mismatch'}
        $bytes=$output.ToArray();$algorithm=[Security.Cryptography.SHA256]::Create()
        try {$hash=[BitConverter]::ToString($algorithm.ComputeHash($bytes)).Replace('-','').ToLowerInvariant()}
        finally {$algorithm.Dispose()}
        if($hash -ne $delta.output_sha256){throw 'Delta output hash mismatch'}
        $stream=[IO.File]::Open($OutputPath,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::None)
        try {$stream.Write($bytes,0,$bytes.Length)} finally {$stream.Dispose()}
    } finally {$output.Dispose()}
}
function Test-RemasterInstalled([string]$GameRoot) {
    Assert-RemasterTree $GameRoot
    $manifest=Read-RemasterJson (Join-Path $GameRoot '.dw-remaster\manifest.json')
    $state=Read-RemasterJson (Join-Path $GameRoot '.dw-remaster\installed.json')
    if($manifest.schema -ne 1 -or $state.schema -ne 1 -or $state.build_id -ne $manifest.build_id){throw 'Installed build metadata mismatch'}
    foreach($record in $manifest.original_files) {
        $expected=$record.sha256
        if($record.path -eq 'ds9dw.exe'){$expected=$manifest.engine.output_sha256}
        if($manifest.asset -and $record.path -eq $manifest.asset.target){$expected=$manifest.asset.output_sha256}
        Assert-RemasterHash (Get-RemasterChild $GameRoot $record.path) $expected
    }
    foreach($record in $manifest.runtime_files) {Assert-RemasterHash (Get-RemasterChild $GameRoot $record.target) $record.sha256}
    $profile=@($manifest.profiles | Where-Object {$_.name -eq $state.profile})
    if($profile.Count -ne 1){throw 'Unknown installed backend profile'}
    Assert-RemasterHash (Join-Path $GameRoot 'DDrawCompat-ds9dw.ini') $profile[0].sha256
    Assert-RemasterHash (Join-Path $GameRoot '.dw-remaster\base\ds9dw.exe') $manifest.engine.input_sha256
    if($manifest.asset){Assert-RemasterHash (Get-RemasterChild (Join-Path $GameRoot '.dw-remaster\base\assets') $manifest.asset.target) $manifest.asset.input_sha256}
    return $state
}
function Copy-RemasterOriginal([string]$Source,[string]$Target,$Manifest) {
    Assert-RemasterTree $Source
    if(Test-Path -LiteralPath $Target){throw 'Clean source staging must be new'}
    # Verify every immutable input before copying; do not import old launchers or
    # Modernizer state whose shortcut/backups refer to another installation.
    $installed=Test-Path -LiteralPath (Join-Path $Source '.dw-remaster\installed.json')
    $sourceManifest=$null
    if($installed){$sourceManifest=Read-RemasterJson (Join-Path $Source '.dw-remaster\manifest.json')}
    $inputs=@()
    foreach($record in $Manifest.original_files) {
        $path=Get-RemasterChild $Source $record.path
        if($installed -and $record.path -eq 'ds9dw.exe'){$path=Join-Path $Source '.dw-remaster\base\ds9dw.exe'}
        if($installed -and $sourceManifest.asset -and $record.path -eq $sourceManifest.asset.target){$path=Get-RemasterChild (Join-Path $Source '.dw-remaster\base\assets') $sourceManifest.asset.target}
        Assert-RemasterHash $path $record.sha256
        $inputs+=@{Source=$path;Relative=$record.path}
    }
    $null=New-Item -ItemType Directory -Path $Target
    foreach($input in $inputs) {
        $to=Get-RemasterChild $Target $input.Relative
        $null=New-Item -ItemType Directory -Path (Split-Path -Parent $to) -Force
        Copy-Item -LiteralPath $input.Source -Destination $to
    }
    # Saves are mutable user data, not patched/hashed package assets.
    foreach($save in Get-ChildItem -LiteralPath $Source -Filter '*.sav' -File -Recurse -Force) {
        $relative=$save.FullName.Substring($Source.TrimEnd('\').Length+1)
        if($relative -match '(^|[\\/])\.dw-'){continue}
        $to=Get-RemasterChild $Target $relative
        $null=New-Item -ItemType Directory -Path (Split-Path -Parent $to) -Force
        if(Test-Path -LiteralPath $to){throw 'Save collision with immutable game file'}
        Copy-Item -LiteralPath $save.FullName -Destination $to
    }
}
Export-ModuleMember -Function *-Remaster*
