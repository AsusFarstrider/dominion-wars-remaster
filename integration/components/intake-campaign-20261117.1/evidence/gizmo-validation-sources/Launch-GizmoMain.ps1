[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Run)
$ErrorActionPreference='Stop';$Run=[IO.Path]::GetFullPath($Run)
$inputs=Get-Content -LiteralPath (Join-Path $Run 'inputs.json') -Raw | ConvertFrom-Json
if($inputs.gizmo_main.schema -ne 'quantum-gizmo-main-run-1'){throw 'Expected isolated Gizmo/Main candidate'}
function Digest([string]$Path){$stream=[IO.File]::OpenRead($Path);$hash=[Security.Cryptography.SHA256]::Create();try{return ([BitConverter]::ToString($hash.ComputeHash($stream))).Replace('-','').ToLowerInvariant()}finally{$stream.Dispose();$hash.Dispose()}}
foreach($pin in @(@($PSCommandPath,$inputs.gizmo_main.launcher_sha256),
    @((Join-Path $inputs.game $inputs.gizmo_main.gizmo.path),$inputs.gizmo_main.gizmo.sha256),
    @((Join-Path $inputs.game $inputs.gizmo_main.main_background.target),$inputs.gizmo_main.main_background.payload_sha256))){
    if((Digest $pin[0]) -ne $pin[1]){throw "Gizmo/Main candidate changed: $($pin[0])"}}
$prior=[Environment]::GetEnvironmentVariable('QFR_MAIN_BACKGROUND','Process')
try{$env:QFR_MAIN_BACKGROUND='1'; & (Join-Path $Run 'Launch-OpeningCards.ps1') -Run $Run}
finally{[Environment]::SetEnvironmentVariable('QFR_MAIN_BACKGROUND',$prior,'Process')}
