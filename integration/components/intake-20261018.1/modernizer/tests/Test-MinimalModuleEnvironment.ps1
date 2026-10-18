# Run in a separate Windows PowerShell process. This mimics a startup environment
# where module-discovered Get-FileHash is unavailable; display tests are read-only.
[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$GamePath)
$ErrorActionPreference = 'Stop'
$env:PSModulePath = ''
# Some hosts preload Utility despite the empty search path. A failing stub makes
# accidental reliance on its hash command detectable on those hosts as well.
function global:Get-FileHash { throw 'Get-FileHash is unavailable in this regression fixture.' }
function global:Get-CimInstance { throw 'CIM inventory is unavailable in this regression fixture.' }
$root = Split-Path -Parent $PSScriptRoot
& (Join-Path $root 'payload\Launch-DominionWars.ps1') -GamePath $GamePath -DiagnoseOnly
if ($LASTEXITCODE -ne 0) { throw 'Diagnostics failed without module-discovered hash command.' }
Write-Host 'PASS: read-only launcher diagnostics without Get-FileHash or CIM modules.'
