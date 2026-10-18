[CmdletBinding()]
param([string]$OutputPath)
$ErrorActionPreference='Stop'
Set-StrictMode -Version 3.0
$root=$PSScriptRoot
$manifestPath=Join-Path $root 'MANIFEST.json'
$manifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
if ($manifest.version -notmatch '^\d+\.\d+\.\d+-alpha$') { throw 'Unrecognized candidate version.' }
if (-not $OutputPath) { $OutputPath=Join-Path $root ('releases\dominion-wars-modernizer-'+$manifest.version+'-candidate-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmssZ')+'.zip') }
$OutputPath=[IO.Path]::GetFullPath($OutputPath)
if (Test-Path -LiteralPath $OutputPath) { throw 'Refusing to overwrite an existing release archive.' }
foreach ($entry in $manifest.files) {
    if ([IO.Path]::IsPathRooted($entry.path) -or $entry.path -match '(^|[/\\])\.\.([/\\]|$)|:') { throw 'Unsafe manifest path.' }
    $file=Get-Item -LiteralPath (Join-Path $root $entry.path)
    if ($file.Length -ne $entry.size -or (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash -ne $entry.sha256) { throw "Manifest mismatch: $($entry.path)" }
}
$stage=Join-Path $root ('.test-package-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage | Out-Null
foreach ($entry in $manifest.files) {
    $target=Join-Path $stage $entry.path
    New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $root $entry.path) -Destination $target
}
Copy-Item -LiteralPath $manifestPath -Destination $stage
New-Item -ItemType Directory -Path (Split-Path -Parent $OutputPath) -Force | Out-Null
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archivePath=$OutputPath+'.partial-'+[guid]::NewGuid().ToString('N')
$created=[IO.Compression.ZipFile]::Open($archivePath,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($entry in $manifest.files) {
        $null=[IO.Compression.ZipFileExtensions]::CreateEntryFromFile($created,(Join-Path $stage $entry.path),($entry.path -replace '\\','/'),[IO.Compression.CompressionLevel]::Optimal)
    }
    $null=[IO.Compression.ZipFileExtensions]::CreateEntryFromFile($created,(Join-Path $stage 'MANIFEST.json'),'MANIFEST.json',[IO.Compression.CompressionLevel]::Optimal)
}
finally { $created.Dispose() }
$archive=[IO.Compression.ZipFile]::OpenRead($archivePath)
try {
    if ($archive.Entries.Count -ne $manifest.files.Count+1) { throw 'Release archive entry count mismatch.' }
    foreach ($entry in $manifest.files) {
        $zipped=$archive.GetEntry(($entry.path -replace '\\','/'))
        if (-not $zipped -or $zipped.Length -ne $entry.size) { throw "Missing/incorrect archive entry: $($entry.path)" }
        $stream=$zipped.Open(); $algorithm=[Security.Cryptography.SHA256]::Create()
        try { $hash=[BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','') }
        finally { $stream.Dispose(); $algorithm.Dispose() }
        if ($hash -ne $entry.sha256) { throw "Archive hash mismatch: $($entry.path)" }
    }
}
finally { $archive.Dispose() }
Move-Item -LiteralPath $archivePath -Destination $OutputPath
Get-FileHash -LiteralPath $OutputPath -Algorithm SHA256 | Select-Object Path,Hash
Write-Host "Candidate verified: $($manifest.files.Count) manifest files plus MANIFEST.json. Staging retained at $stage"
