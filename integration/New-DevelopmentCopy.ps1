[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Destination,[string]$CurrentRecord)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
if(-not $CurrentRecord){$CurrentRecord=Join-Path $root 'current.json'}
$record=Read-RemasterJson $CurrentRecord
if($record.status -ne 'approved-internal-baseline'){throw 'Adoption requires an approved Current Build, not an experiment'}
$null=Assert-RemasterPackage $record.package_root
Assert-RemasterHash $record.package $record.package_sha256
Assert-RemasterHash (Join-Path $record.package_root 'manifest.json') $record.manifest_sha256
$target=Assert-RemasterPlainPath $Destination
if(Test-Path -LiteralPath $target){throw 'Use a new disposable directory. Existing branch work is never overwritten.'}
& (Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe') -NoProfile -ExecutionPolicy Bypass -File (Join-Path $record.package_root 'Remaster.ps1') -Action Install -SourcePath $record.game_root -Destination $target -NoDesktopShortcut -NonInteractive
if($LASTEXITCODE -ne 0){throw 'Development-copy adoption failed; prior branch work unchanged'}
Write-Host "Created independent copy of $($record.build_id) at $target."
