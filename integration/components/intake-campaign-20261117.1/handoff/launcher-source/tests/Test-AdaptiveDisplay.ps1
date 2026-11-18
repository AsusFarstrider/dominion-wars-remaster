$ErrorActionPreference='Stop'
$root=Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
$script:passed=0
function Check([bool]$Condition,[string]$Name){if(-not $Condition){throw "FAILED: $Name"};$script:passed++}
function Refuses([scriptblock]$Action,[string]$Name){$caught=$false;try{& $Action}catch{$caught=$true};Check $caught $Name}
function Mode([int]$Width=2560,[int]$Height=1440){
 return [pscustomobject]@{dmPelsWidth=$Width;dmPelsHeight=$Height;dmPositionX=0;dmPositionY=0;dmDisplayFrequency=165;dmDisplayOrientation=0;dmBitsPerPel=32}
}
function Manifest {
 return [pscustomobject]@{rendering=[pscustomobject]@{profile='native-display-v1';recording='off'};engine=[pscustomobject]@{output_sha256='e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023'}}
}
$m=Manifest;$mode=Mode
$selection=Get-RemasterOutputSelection $m $mode '\\.\DISPLAY1' 'test-monitor-identity'
Check ($selection.width -eq 2560 -and $selection.height -eq 1440) 'physical primary dimensions'
Check ($selection.profile -ceq 'native-display-v1' -and -not $selection.live_resize) 'explicit stable session contract'
Check ($selection.source -ceq 'primary-current-physical-mode') 'physical source labeled'
$null=Assert-RemasterOutput $m $mode
Check $true 'adaptive capability admitted'
foreach($dimensions in @(@(1920,1080),@(800,600),@(1366,768),@(1365,767),@(8192,8192))){
 $s=Get-RemasterOutputSelection $m (Mode $dimensions[0] $dimensions[1]) 'display' 'identity'
 Check ($s.width -eq $dimensions[0] -and $s.height -eq $dimensions[1]) ('bounded selection '+$dimensions[0])
}
foreach($pair in @(@(799,600),@(800,599),@(640,480),@(8193,1080),@(1920,8193),@(0,0),@(-1,1080))){
 Refuses {Get-RemasterOutputSelection $m (Mode $pair[0] $pair[1]) 'display' 'identity'} 'reject dimensions outside bounds'
}
$bad=Mode;$bad.dmPelsWidth='1920'
Refuses {Get-RemasterOutputSelection $m $bad 'display' 'identity'} 'reject string dimensions rather than coercion'
$bad=Mode;$bad.dmPelsWidth=1920.5
Refuses {Get-RemasterOutputSelection $m $bad 'display' 'identity'} 'reject fractional dimensions'
foreach($field in @('dmPositionX','dmPositionY')){
 $bad=Mode;$bad.$field=-1920
 Refuses {Get-RemasterOutputSelection $m $bad 'display' 'identity'} 'reject nonzero physical origin'
}
$badManifest=Manifest;$badManifest.engine.output_sha256='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6'
Refuses {Get-RemasterOutputSelection $badManifest $mode 'display' 'identity'} 'fixed1440 EXE cannot claim adaptive capability'
$badManifest=Manifest;$badManifest.rendering | Add-Member width 2560
Refuses {Get-RemasterOutputSelection $badManifest $mode 'display' 'identity'} 'reject fixed dimensions on adaptive declaration'
$badManifest=Manifest;$badManifest.rendering.profile='unknown'
Refuses {Assert-RemasterOutput $badManifest $mode} 'unknown capability rejected'
$legacy=[pscustomobject]@{rendering=[pscustomobject]@{profile='native-1440-v1';width=2560;height=1440}}
$null=Assert-RemasterOutput $legacy $mode
Check $true 'legacy1440 valid unchanged'
Refuses {Assert-RemasterOutput $legacy (Mode 1920 1080)} 'legacy1440 still refuses1080'
$noCapability=[pscustomobject]@{build_id='int-20261018.1'}
$null=Assert-RemasterOutput $noCapability (Mode 800 600)
Check $true 'older missing rendering capability remains legacy'
$null=Assert-RemasterOutputUnchanged $selection $mode '\\.\DISPLAY1' 'test-monitor-identity'
Check $true 'unchanged physical snapshot admitted'
foreach($field in @('dmPelsWidth','dmPelsHeight','dmPositionX','dmPositionY','dmDisplayFrequency','dmDisplayOrientation','dmBitsPerPel')){
 $changed=Mode;$changed.$field++
 Refuses {Assert-RemasterOutputUnchanged $selection $changed '\\.\DISPLAY1' 'test-monitor-identity'} ('mode recheck '+$field)
}
Refuses {Assert-RemasterOutputUnchanged $selection $mode '\\.\DISPLAY2' 'test-monitor-identity'} 'device changed'
Refuses {Assert-RemasterOutputUnchanged $selection $mode '\\.\DISPLAY1' 'replacement-monitor'} 'monitor identity changed'
$envNames=@('DW_REMASTER_OUTPUT_WIDTH','DW_REMASTER_OUTPUT_HEIGHT')
$before=@{};foreach($name in $envNames){$before[$name]=[Environment]::GetEnvironmentVariable($name,'Process')}
$culture=[Threading.Thread]::CurrentThread.CurrentCulture
try{
 [Environment]::SetEnvironmentVariable($envNames[0],'inherited-invalid','Process')
 [Environment]::SetEnvironmentVariable($envNames[1],'999999','Process')
 [Threading.Thread]::CurrentThread.CurrentCulture=New-Object Globalization.CultureInfo 'ar-SA'
 Set-RemasterOutputEnvironment $selection
 Check ($env:DW_REMASTER_OUTPUT_WIDTH -ceq '2560' -and $env:DW_REMASTER_OUTPUT_HEIGHT -ceq '1440') 'canonical invariant decimal pair replaces inherited input'
 $broken=$selection.PSObject.Copy();$broken.height=0
 Refuses {Set-RemasterOutputEnvironment $broken} 'invalid pair refused atomically'
 Check ($env:DW_REMASTER_OUTPUT_WIDTH -ceq '2560' -and $env:DW_REMASTER_OUTPUT_HEIGHT -ceq '1440') 'invalid pair did not partially modify environment'
 $odd=Get-RemasterOutputSelection $m (Mode 1365 767) 'display' 'identity'
 Set-RemasterOutputEnvironment $odd
 Check ($env:DW_REMASTER_OUTPUT_WIDTH -ceq '1365' -and $env:DW_REMASTER_OUTPUT_HEIGHT -ceq '767') 'odd dimensions transmitted without rounding'
 $maximum=Get-RemasterOutputSelection $m (Mode 8192 8192) 'display' 'identity'
 Check (([long]$maximum.width * [long]$maximum.height) -eq 67108864) '64 Mi-pixel boundary admitted exactly'
}finally{
 [Threading.Thread]::CurrentThread.CurrentCulture=$culture
 foreach($name in $envNames){[Environment]::SetEnvironmentVariable($name,$before[$name],'Process')}
}
foreach($name in $envNames){Check ([Environment]::GetEnvironmentVariable($name,'Process') -ceq $before[$name]) 'test restored process environment'}
Write-Host ('PASS: '+$script:passed+' adaptive display contract checks; no game launched or display settings changed.')
