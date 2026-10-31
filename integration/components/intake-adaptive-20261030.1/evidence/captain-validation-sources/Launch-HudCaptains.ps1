[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Run)
$ErrorActionPreference='Stop'
$inputs=Get-Content -LiteralPath (Join-Path $Run 'inputs.json') -Raw | ConvertFrom-Json
if($inputs.captain_trial.schema -ne 'quantum-captain-trial-run-1'){throw 'Expected the separate captain trial'}
$stream=[IO.File]::OpenRead($PSCommandPath);$digest=[Security.Cryptography.SHA256]::Create()
try{$actual=([BitConverter]::ToString($digest.ComputeHash($stream))).Replace('-','').ToLowerInvariant()}
finally{$digest.Dispose();$stream.Dispose()}
if($actual -ne $inputs.captain_trial.launcher_sha256){throw 'HUD trial launcher changed'}
$prior=[Environment]::GetEnvironmentVariable('QFR_HUD_CAPTAINS','Process')
try {
    $env:QFR_HUD_CAPTAINS='1'
    & (Join-Path $Run 'Launch-Submenus.ps1') -Run $Run
}finally{[Environment]::SetEnvironmentVariable('QFR_HUD_CAPTAINS',$prior,'Process')}
