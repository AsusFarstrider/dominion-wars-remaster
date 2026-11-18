param([Parameter(Mandatory=$true)][string]$SourceManifestSHA,[Parameter(Mandatory=$true)][string]$WrapperSHA)
$ErrorActionPreference='Stop'
$game='E:\Codex\remaster-build-tests\adaptive-output-20261030-01'
$source='E:\Codex\research\dominion-rendering\dynamic-output-source-v1'
$engine='E:\Codex\research\dominion-rendering\native-engine-800x600-01\ds9dw.exe'
Import-Module (Join-Path $game 'Remaster.Core.psm1') -Force
Assert-RemasterIdle
Assert-RemasterHash (Join-Path $source 'qfr-source.json') $SourceManifestSHA
Assert-RemasterHash (Join-Path $source 'Release\ddraw.dll') $WrapperSHA
Assert-RemasterHash $engine 'e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023'
$null=Test-RemasterInstalled $game
$meta=Join-Path $game '.dw-remaster'
$manifest=Read-RemasterJson (Join-Path $meta 'manifest.json')
if($manifest.build_id -ne 'int-20261029.1'){throw 'Requires NEW unchanged Current copy'}
$backup=Join-Path $meta 'adaptive-parent'
if(Test-Path -LiteralPath $backup){throw 'Trial already exists; preserve it'}
$null=New-Item -ItemType Directory -Path $backup
foreach($name in @('ds9dw.exe','ddraw.dll','version.json','Remaster.Rendering.json','Launch-Remaster.ps1','Remaster.Core.psm1')){Copy-Item -LiteralPath (Join-Path $game $name) -Destination (Join-Path $backup $name)}
foreach($name in @('manifest.json','installed.json')){Copy-Item -LiteralPath (Join-Path $meta $name) -Destination (Join-Path $backup $name)}
$state=Read-RemasterJson (Join-Path $meta 'installed.json')
$identity=Read-RemasterJson (Join-Path $game 'version.json')
$config=Read-RemasterJson (Join-Path $game 'Remaster.Rendering.json')
$identity.integration_build_id=$manifest.build_id=$state.build_id='int-20261029.1-adaptive-output-01'
$identity.flavor=$manifest.flavor='native-display-prototype-01'
$identity.release_stage=$manifest.release_stage='experimental'
$identity.version_decision_id=$manifest.version_decision_id='rendering-adaptive-output-20261030-01'
Write-RemasterJson (Join-Path $game 'version.json') $identity
$manifest.version_identity.sha256=Get-RemasterHash (Join-Path $game 'version.json')
$config.version_sha256=$manifest.version_identity.sha256
$config.profile=$manifest.rendering.profile='native-display-v1'
$config.engine_sha256=$manifest.engine.output_sha256='e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023'
if($config.recording -ne 'off'){throw 'Recorder must remain OFF'}
Write-RemasterJson (Join-Path $game 'Remaster.Rendering.json') $config
$manifest.rendering.PSObject.Properties.Remove('width')
$manifest.rendering.PSObject.Properties.Remove('height')
$manifest.rendering.mode_policy='launch-selected physical primary at0,0; no desktop change; no live resize'
$manifest.rendering.source_manifest_sha256=$SourceManifestSHA
Copy-Item -LiteralPath $engine -Destination (Join-Path $game 'ds9dw.exe')
Copy-Item -LiteralPath (Join-Path $source 'Release\ddraw.dll') -Destination (Join-Path $game 'ddraw.dll')
foreach($name in @('Launch-Remaster.ps1','Remaster.Core.psm1')){Copy-Item -LiteralPath (Join-Path $source "launcher-source\runtime\$name") -Destination (Join-Path $game $name)}
$manifest.wrapper.sha256=$WrapperSHA
foreach($r in $manifest.runtime_files){
 if($r.target -in @('ddraw.dll','version.json','Remaster.Rendering.json','Launch-Remaster.ps1','Remaster.Core.psm1')){
  $p=Get-RemasterChild $game $r.target;$r.sha256=Get-RemasterHash $p;$r.size=(Get-Item -LiteralPath $p).Length
 }
}
$manifest.status='private-adaptive-output-trial-not-a-release-package'
$manifest.uncommitted_source=$true
$manifest.approvals.internal_baseline='experimental child; never promote directly'
$manifest.approvals.public_release='not authorized'
$state.status='private-experimental'
Write-RemasterJson (Join-Path $meta 'manifest.json') $manifest
Write-RemasterJson (Join-Path $meta 'installed.json') $state
Remove-Module Remaster.Core
Import-Module (Join-Path $game 'Remaster.Core.psm1') -Force
$null=Test-RemasterInstalled $game
Write-RemasterJson (Join-Path $meta 'adaptive-provenance.json') ([ordered]@{
 status='private-uncommitted-nonpromotable-experiment';parent_build='int-20261029.1';
 parent_manifest_sha256=(Get-RemasterHash (Join-Path $backup 'manifest.json'));
 source_manifest_sha256=$SourceManifestSHA;wrapper_sha256=$WrapperSHA;engine_sha256=$config.engine_sha256;
 launcher_receipt_sha256=(Get-RemasterHash (Join-Path $source 'launcher-source\source-receipt.json'));
 procedure_sha256=(Get-RemasterHash $PSCommandPath);recording='off';game_launched=$false;current_modified=$false;
 changes='Accepted ESC/performance fixes; launch-selected native engine operands; source-history HD fitting; coordinated Launcher two-file capability. Existing artwork retained. Same EXE/DLL/profile for every session.'
})
Write-Output "Prepared adaptive trial: $game"
