param([Parameter(Mandatory=$true)][string]$SourceManifestSHA,[Parameter(Mandatory=$true)][string]$WrapperSHA)
$ErrorActionPreference='Stop'
$game='E:\Codex\remaster-build-tests\esc-main-menu-reentry-20261029-02'
$source='E:\Codex\research\dominion-rendering\esc-main-menu-source-v1'
Import-Module (Join-Path $game 'Remaster.Core.psm1') -Force
Assert-RemasterIdle
Assert-RemasterHash (Join-Path $source 'qfr-source.json') $SourceManifestSHA
Assert-RemasterHash (Join-Path $source 'Release\ddraw.dll') $WrapperSHA
$null=Test-RemasterInstalled $game
$meta=Join-Path $game '.dw-remaster'
$manifest=Read-RemasterJson (Join-Path $meta 'manifest.json')
if($manifest.build_id -ne 'int-20261029.1'){throw 'Expected a NEW unchanged accepted-baseline copy'}
$backup=Join-Path $meta 'esc-reentry-parent'
if(Test-Path -LiteralPath $backup){throw 'Trial overlay already exists; preserve it'}
$null=New-Item -ItemType Directory -Path $backup
foreach($name in @('ddraw.dll','version.json','Remaster.Rendering.json')){Copy-Item -LiteralPath (Join-Path $game $name) -Destination (Join-Path $backup $name)}
foreach($name in @('manifest.json','installed.json')){Copy-Item -LiteralPath (Join-Path $meta $name) -Destination (Join-Path $backup $name)}
$state=Read-RemasterJson (Join-Path $meta 'installed.json')
$identity=Read-RemasterJson (Join-Path $game 'version.json')
$config=Read-RemasterJson (Join-Path $game 'Remaster.Rendering.json')
$identity.integration_build_id=$manifest.build_id=$state.build_id='int-20261029.1-esc-reentry-01'
$identity.flavor=$manifest.flavor='native1440-esc-reentry-01'
$identity.release_stage=$manifest.release_stage='experimental'
$identity.version_decision_id=$manifest.version_decision_id='rendering-esc-reentry-20261029-01'
Write-RemasterJson (Join-Path $game 'version.json') $identity
$manifest.version_identity.sha256=Get-RemasterHash (Join-Path $game 'version.json')
$config.version_sha256=$manifest.version_identity.sha256
if($config.recording -ne 'off'){throw 'Recorder must remain OFF'}
Write-RemasterJson (Join-Path $game 'Remaster.Rendering.json') $config
Copy-Item -LiteralPath (Join-Path $source 'Release\ddraw.dll') -Destination (Join-Path $game 'ddraw.dll')
$manifest.wrapper.sha256=$WrapperSHA
$manifest.rendering.source_manifest_sha256=$SourceManifestSHA
foreach($r in $manifest.runtime_files){
 if($r.target -in @('ddraw.dll','version.json','Remaster.Rendering.json')){
  $p=Get-RemasterChild $game $r.target;$r.sha256=Get-RemasterHash $p;$r.size=(Get-Item -LiteralPath $p).Length
 }
}
$manifest.status='private-esc-reentry-trial-not-a-release-package'
$manifest.uncommitted_source=$true
$manifest.approvals.internal_baseline='experimental child; never promote directly'
$manifest.approvals.public_release='not authorized'
$state.status='private-experimental'
Write-RemasterJson (Join-Path $meta 'manifest.json') $manifest
Write-RemasterJson (Join-Path $meta 'installed.json') $state
$null=Test-RemasterInstalled $game
Write-RemasterJson (Join-Path $meta 'esc-reentry-provenance.json') ([ordered]@{
 status='private-uncommitted-nonpromotable-experiment';parent_runtime_source=$manifest.source_revision;
 parent_build='int-20261029.1';parent_manifest_sha256=(Get-RemasterHash (Join-Path $backup 'manifest.json'));
 source_manifest_sha256=$SourceManifestSHA;wrapper_sha256=$WrapperSHA;procedure_sha256=(Get-RemasterHash $PSCommandPath);
 changes='One CanvasRuntime ownership policy correction; new experimental metadata/config binding. Engine/assets/profile/launcher unchanged; recording OFF.';
 game_launched=$false;current_modified=$false
})
Write-Output "Prepared corrected experimental trial: $game"
