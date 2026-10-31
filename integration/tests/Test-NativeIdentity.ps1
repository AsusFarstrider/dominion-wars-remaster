param([string]$TestRoot='E:\Codex\remaster-build-tests')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
$directory=Join-Path $TestRoot ('identity-'+[guid]::NewGuid().ToString('N'))
$null=New-Item -ItemType Directory -Path $directory
$checks=0
function Check([bool]$Value){if(-not $Value){throw 'Native identity check failed'};$script:checks++}
function Refused([scriptblock]$Call){$failed=$false;try{& $Call}catch{$failed=$true};Check $failed}
$identity=[ordered]@{schema=1;product='Dominion Wars Remastered';original_game_version='1.04';remaster_version='1.10';release_stage='alpha';integration_build_id='int-20261027.1';source_revision=('a'*40);flavor='native1440-restored-v7';version_decision_id='unit-fixture-not-approval';in_game_label='Remastered 1.10'}
$path=Join-Path $directory 'version.json'
Write-RemasterJson $path $identity
$manifest=[pscustomobject]@{build_id=$identity.integration_build_id;source_revision=$identity.source_revision;flavor=$identity.flavor;version_identity=[pscustomobject]@{target='version.json';sha256=(Get-RemasterHash $path)}}
$got=Get-RemasterIdentity $directory $manifest
Check ($got.remaster_version -ceq '1.10')
Check ($got.in_game_label -ceq 'Remastered 1.10')
$identity.remaster_version='1.15';$identity.in_game_label='Remastered 1.15'
Write-RemasterJson $path $identity;$manifest.version_identity.sha256=Get-RemasterHash $path
Check ((Get-RemasterIdentity $directory $manifest).in_game_label -ceq 'Remastered 1.15')
$identity.remaster_version='1.10';$identity.in_game_label='Remastered 1.10'
Write-RemasterJson $path $identity;$manifest.version_identity.sha256=Get-RemasterHash $path
Refused {Get-RemasterIdentity $directory ([pscustomobject]@{build_id='legacy'})}
$manifest.version_identity.sha256='0'*64
Refused {Get-RemasterIdentity $directory $manifest}
foreach($change in @(@('remaster_version','1.1'),@('remaster_version',1.1),@('in_game_label','Version 1.04'),@('source_revision','not-a-commit'),@('original_game_version','1.10'),@('release_stage','ready'))) {
    $saved=$identity[$change[0]];$identity[$change[0]]=$change[1]
    Write-RemasterJson $path $identity;$manifest.version_identity.sha256=Get-RemasterHash $path
    Refused {Get-RemasterIdentity $directory $manifest}
    $identity[$change[0]]=$saved
}
Write-RemasterJson $path $identity;$manifest.version_identity.sha256=Get-RemasterHash $path
$manifest.build_id='int-20261027.2'
Refused {Get-RemasterIdentity $directory $manifest}
$render=[pscustomobject]@{rendering=[pscustomobject]@{profile='native-1440-v1';width=2560;height=1440}}
$mode=[pscustomobject]@{dmPelsWidth=2560;dmPelsHeight=1440;dmPositionX=0;dmPositionY=0}
Assert-RemasterOutput $render $mode;$checks++
$mode.dmPelsWidth=1920;$mode.dmPelsHeight=1080
Refused {Assert-RemasterOutput $render $mode}
$mode.dmPelsWidth=2560;$mode.dmPelsHeight=1440;$mode.dmPositionX=-2560
Refused {Assert-RemasterOutput $render $mode}
$render.rendering.profile='unknown'
Refused {Assert-RemasterOutput $render $mode}
$patches=[pscustomobject]@{asset=$null;asset_patches=@([pscustomobject]@{target='rs/Video/Paramount.bik'},[pscustomobject]@{target='rs/Video/GizmoLogo.bik'})}
Check (@(Get-RemasterAssetPatches $patches).Count -eq 2)
$patches.asset_patches[1].target='rs/Video/Paramount.bik'
Refused {Get-RemasterAssetPatches $patches}
Write-Host "PASS: $checks native identity/output/asset checks. Synthetic fixtures only; no game launched. Retained at $directory"
