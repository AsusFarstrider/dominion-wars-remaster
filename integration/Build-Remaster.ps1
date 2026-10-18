[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$OriginalGame,[Parameter(Mandatory=$true)][string]$Output,
      [string]$PythonPath='python',[string]$PrivateAssetDelta,[switch]$Development)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$buildArgs=@((Join-Path $root 'tools\build_remaster.py'),'--original-game',$OriginalGame,'--output',$Output)
if($PrivateAssetDelta){$buildArgs+=@('--private-asset-delta',$PrivateAssetDelta)}
if($Development){$buildArgs+='--development'}
& $PythonPath @buildArgs
if($LASTEXITCODE -ne 0){throw 'Remaster build failed'}
