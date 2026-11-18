[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Run)
$ErrorActionPreference='Stop'
$env:PSModulePath=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\Modules'
$Run=[IO.Path]::GetFullPath($Run)
$inputs=Get-Content -LiteralPath (Join-Path $Run 'inputs.json') -Raw | ConvertFrom-Json
$source=Get-Content -LiteralPath (Join-Path $Run 'source-manifest.json') -Raw | ConvertFrom-Json
if($inputs.opening_flow.schema -ne 'quantum-opening-flow-run-1' -or $source.opening_flow.schema -ne 'quantum-opening-flow-source-1'){throw 'Expected the combined opening candidate'}
foreach($pin in @(@($PSCommandPath,$inputs.opening_flow.launcher_sha256),@((Join-Path $Run 'results.json'),$inputs.opening_flow.preflight_sha256),@((Join-Path $Run 'opening-build-checks.json'),$inputs.opening_flow.build_checks_sha256))){
    if((Get-FileHash -LiteralPath $pin[0] -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pin[1]){throw 'Opening preflight/launcher changed'}
}
if(@(Get-Process -Name ds9dw -ErrorAction SilentlyContinue).Count){throw 'Another game is running; close it before this separate test'}
$prior=@{}
try {
    foreach($name in @('QFR_UI_PROBE','QFR_UI_CANVAS','QFR_UI_MAIN_CANVAS','QFR_UI_TRANSITIONS','QFR_OPENING_FLOW')){
        $prior[$name]=[Environment]::GetEnvironmentVariable($name,'Process')
        [Environment]::SetEnvironmentVariable($name,'1','Process')
    }
    [ordered]@{schema='opening-flow-launch-1';source_manifest_sha256=$inputs.source_manifest_sha256;utc=[DateTime]::UtcNow.ToString('o');process_only_opt_ins=@('UI_PROBE','UI_CANVAS','UI_MAIN_CANVAS','UI_TRANSITIONS','OPENING_FLOW');readback='off';hud='original';durable_checkpoint_ms=500} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Run 'opening-launch.json') -Encoding UTF8
    & (Join-Path $PSScriptRoot '..\native-resolution\Launch-NativeProbe.ps1') -Run $Run
}finally{foreach($name in $prior.Keys){[Environment]::SetEnvironmentVariable($name,$prior[$name],'Process')}}
