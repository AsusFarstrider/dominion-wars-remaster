[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$BuildDirectory,
      [Parameter(Mandatory=$true)][string]$InstalledGame,
      [Parameter(Mandatory=$true)][string]$ValidationRecord,
      [Parameter(Mandatory=$true)][switch]$UserAccepted,
      [string]$BaselineStore='E:\Codex\remaster-builds\baselines')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
if(-not $UserAccepted){throw 'Explicit user acceptance is required; no automatic promotion'}
Assert-RemasterIdle
$build=Read-RemasterJson (Join-Path $BuildDirectory 'build-record.json')
$packageRoot=Join-Path $BuildDirectory 'package'
$manifest=Assert-RemasterPackage $packageRoot
Assert-RemasterHash (Join-Path $packageRoot 'manifest.json') $build.manifest_sha256
Assert-RemasterHash $build.package $build.package_sha256
if($manifest.uncommitted_source -or $manifest.source_revision -notmatch '^[a-f0-9]{40}$'){throw 'An uncommitted/development build cannot become Current Build'}
$installed=Test-RemasterInstalled $InstalledGame
Assert-RemasterHash (Join-Path $InstalledGame '.dw-remaster\manifest.json') $build.manifest_sha256
if($installed.build_id -ne $manifest.build_id){throw 'Installed validation target differs from build'}
$validation=Read-RemasterJson $ValidationRecord
if($validation.build_id -ne $manifest.build_id -or $validation.package_sha256 -ne $build.package_sha256){throw 'Validation belongs to another package'}
foreach($category in @('install_clone','install_media','startup_shutdown','rendering_ui_movies','mouse_keyboard','focus_taskbar','screenshots','mission','save_load')) {
    $result=$validation.results.$category
    if($result.status -eq 'pass') {if(-not $result.evidence){throw "Missing test evidence: $category"}}
    elseif($result.status -eq 'accepted-limitation') {if(-not $result.reason -or -not $result.user_accepted){throw "Unapproved limitation: $category"}}
    else {throw "Promotion gate not passed: $category"}
}
if(-not $validation.user_acceptance -or -not $validation.user_acceptance.evidence){throw 'Record the human acceptance and its scope'}
$null=New-Item -ItemType Directory -Path $BaselineStore -Force
$id=$manifest.build_id+'-'+$build.package_sha256.Substring(0,12)
$recordPath=Join-Path $BaselineStore ($id+'.json')
if(Test-Path -LiteralPath $recordPath){throw 'Baseline promotion record is immutable and already exists'}
$pointerPath=Join-Path $root 'current.json'
$previous=if(Test-Path -LiteralPath $pointerPath){Read-RemasterJson $pointerPath}else{$null}
$record=[ordered]@{schema=1;status='approved-internal-baseline';build_id=$manifest.build_id;source_revision=$manifest.source_revision;
    package_root=$packageRoot;package=$build.package;package_sha256=$build.package_sha256;manifest_sha256=$build.manifest_sha256;
    game_root=[IO.Path]::GetFullPath($InstalledGame);validation_path=[IO.Path]::GetFullPath($ValidationRecord);validation_sha256=(Get-RemasterHash $ValidationRecord);
    previous=$previous;accepted_utc=[DateTime]::UtcNow.ToString('o');user_acceptance=$validation.user_acceptance;public_release='not authorized'}
Write-RemasterJson $recordPath $record
$temporary=Join-Path $root ('current.'+[guid]::NewGuid().ToString('N')+'.json')
Write-RemasterJson $temporary $record
Move-Item -LiteralPath $temporary -Destination $pointerPath -Force
Write-Host "Current internal baseline: $id. Previous baseline record preserved. No public release authorized."
