param([Parameter(Mandatory=$true)][string]$ReceiptSHA)
$ErrorActionPreference='Stop'
$game='E:\Codex\remaster-build-tests\adaptive-output-20261030-01'
$source=Join-Path (Split-Path -Parent $PSScriptRoot) '..\startup-research\adaptive-display-v2-source'
$source=[IO.Path]::GetFullPath($source)
Import-Module (Join-Path $game 'Remaster.Core.psm1') -Force
Assert-RemasterIdle
$receiptPath=Join-Path $source 'source-receipt.json'
Assert-RemasterHash $receiptPath $ReceiptSHA
$receipt=Read-RemasterJson $receiptPath
foreach($r in $receipt.runtime.PSObject.Properties){Assert-RemasterHash (Get-RemasterChild $source ('runtime/'+$r.Name)) $r.Value}
foreach($r in $receipt.upstream.PSObject.Properties){Assert-RemasterHash (Get-RemasterChild $game $r.Name) $r.Value}
Assert-RemasterHash (Join-Path $source 'changes.patch') $receipt.patch_sha256
$null=Test-RemasterInstalled $game
$meta=Join-Path $game '.dw-remaster'
$manifest=Read-RemasterJson (Join-Path $meta 'manifest.json')
if($manifest.build_id -ne 'int-20261029.1-adaptive-output-01'){throw 'Unexpected trial identity'}
$backup=Join-Path $meta 'adaptive-launcher-v2-parent'
if(Test-Path -LiteralPath $backup){throw 'Fix already applied; preserve it'}
$null=New-Item -ItemType Directory -Path $backup
Copy-Item -LiteralPath (Join-Path $meta 'manifest.json') -Destination (Join-Path $backup 'manifest.json')
$names=@('Launch-Remaster.ps1','Remaster.Core.psm1','Watch-RemasterFocus.ps1')
foreach($name in $names){
 Copy-Item -LiteralPath (Join-Path $game $name) -Destination (Join-Path $backup $name)
 Copy-Item -LiteralPath (Join-Path $source "runtime\$name") -Destination (Join-Path $game $name)
}
foreach($r in $manifest.runtime_files){if($r.target -in $names){
 $p=Get-RemasterChild $game $r.target;$r.sha256=Get-RemasterHash $p;$r.size=(Get-Item -LiteralPath $p).Length
}}
Write-RemasterJson (Join-Path $meta 'manifest.json') $manifest
$provenance=Read-RemasterJson (Join-Path $meta 'adaptive-provenance.json')
$provenance | Add-Member -Force NoteProperty launcher_v2_receipt_sha256 $ReceiptSHA
$provenance | Add-Member -Force NoteProperty launcher_v2_procedure_sha256 (Get-RemasterHash $PSCommandPath)
Write-RemasterJson (Join-Path $meta 'adaptive-provenance.json') $provenance
Remove-Module Remaster.Core
Import-Module (Join-Path $game 'Remaster.Core.psm1') -Force
$null=Test-RemasterInstalled $game
Write-Output 'Applied scoped Launcher correction to private adaptive trial only; provider/engine/assets unchanged.'
