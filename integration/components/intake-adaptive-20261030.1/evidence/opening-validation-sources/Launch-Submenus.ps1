[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Run)
$ErrorActionPreference='Stop'
$env:PSModulePath=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\Modules'
$Run=[IO.Path]::GetFullPath($Run)
$inputs=Get-Content -LiteralPath (Join-Path $Run 'inputs.json') -Raw | ConvertFrom-Json
$game=$inputs.game
function FileDigest([string]$Path) {
    $stream=[IO.File]::OpenRead($Path);$digest=[Security.Cryptography.SHA256]::Create()
    try{return ([BitConverter]::ToString($digest.ComputeHash($stream))).Replace('-','').ToLowerInvariant()}
    finally{$digest.Dispose();$stream.Dispose()}
}
if($inputs.submenus.schema -ne 'quantum-opening-submenus-run-1'){throw 'Expected the separately prepared submenu candidate'}
if([IO.Path]::GetFullPath($game) -eq [IO.Path]::GetFullPath($inputs.baseline.game_root)){throw 'Refusing Current Build'}
if(Get-Process -Name ds9dw -ErrorAction SilentlyContinue){throw 'Another game is running; leaving it alone'}
foreach($pin in @(
    @($PSCommandPath,$inputs.submenus.launcher_sha256),
    @((Join-Path $Run 'source-manifest.json'),$inputs.source_manifest_sha256),
    @((Join-Path $game 'ds9dw.exe'),$inputs.exe_sha256),
    @((Join-Path $game 'ddraw.dll'),$inputs.recorder_dll_sha256),
    @((Join-Path $game 'DDrawCompat-ds9dw.ini'),$inputs.config_sha256),
    @((Join-Path $game 'Watch-RemasterFocus.ps1'),$inputs.focus_helper_sha256))) {
    if((FileDigest $pin[0]) -ne $pin[1]){throw "Candidate identity changed: $($pin[0])"}
}
$prior=@{};$helper=$null
try {
    foreach($name in @('QFR_DIR','QFR_NATIVE_PROBE','QFR_UI_PROBE','QFR_UI_CANVAS','QFR_UI_MAIN_CANVAS','QFR_UI_TRANSITIONS','QFR_OPENING_FLOW','QFR_OPENING_SUBMENUS','QFR_READBACK','DW_EXPERIMENTAL_FOCUS_GUARD','QFR_TEST_FAIL_AFTER_EVENTS','QFR_FONT_PROBE','DW_EXPERIMENTAL_FOCUS_DESKTOP','DW_FOCUS_DEVICE','DW_FOCUS_DEVICE_IDENTITY','DW_FOCUS_ORIGINAL_MODE','DW_RESOLUTION_TELEMETRY','DW_DRAW_TELEMETRY','DW_PROJECTION_TELEMETRY','DW_COORDINATE_PROBE')) {
        $prior[$name]=[Environment]::GetEnvironmentVariable($name,'Process')
        [Environment]::SetEnvironmentVariable($name,$null,'Process')
    }
    $env:QFR_DIR=$Run;$env:QFR_READBACK='0';$env:DW_EXPERIMENTAL_FOCUS_GUARD='1'
    foreach($name in @('QFR_NATIVE_PROBE','QFR_UI_PROBE','QFR_UI_CANVAS','QFR_UI_MAIN_CANVAS','QFR_UI_TRANSITIONS','QFR_OPENING_FLOW','QFR_OPENING_SUBMENUS')){[Environment]::SetEnvironmentVariable($name,'1','Process')}
    [ordered]@{schema='opening-submenus-launch-1';utc=[DateTime]::UtcNow.ToString('o');source_manifest_sha256=$inputs.source_manifest_sha256;capture='compact nonblocking opening profile';readback='off';current_modified=$false} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Run 'launch.json') -Encoding UTF8
    $engine=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
    $argsFocus='-NoProfile -ExecutionPolicy Bypass -File "'+(Join-Path $game 'Watch-RemasterFocus.ps1')+'" -GamePath "'+(Join-Path $game 'ds9dw.exe')+'" -LogPath "'+(Join-Path $Run 'focus.log')+'"'
    $helper=Start-Process -FilePath $engine -ArgumentList $argsFocus -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $Run 'helper-output.txt') -RedirectStandardError (Join-Path $Run 'helper-error.txt')
    $null=$helper.Handle
    $deadline=[DateTime]::UtcNow.AddSeconds(20)
    while(-not(Test-Path -LiteralPath (Join-Path $Run 'focus.log'))){if($helper.HasExited -or [DateTime]::UtcNow -gt $deadline){throw 'Focus helper did not become ready'};Start-Sleep -Milliseconds 100}
    $argsGame='-NoProfile -ExecutionPolicy Bypass -File "'+(Join-Path $game 'Launch-NoDesktopSwitch.ps1')+'" -GamePath "'+(Join-Path $game 'ds9dw.exe')+'" -GraphicsBackend Profile -VisualPreset Baseline'
    $child=Start-Process -FilePath $engine -ArgumentList $argsGame -WorkingDirectory $game -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $Run 'launcher-output.txt') -RedirectStandardError (Join-Path $Run 'launcher-error.txt')
    $null=$child.Handle
    $child.WaitForExit();$child.Refresh()
    [ordered]@{schema='opening-submenus-exit-1';launcher_exit=$child.ExitCode;utc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Run 'exit.json') -Encoding UTF8
}finally{
    if($helper -and -not $helper.HasExited){if(-not $helper.WaitForExit(5000)){Stop-Process -Id $helper.Id -ErrorAction SilentlyContinue}}
    foreach($name in $prior.Keys){[Environment]::SetEnvironmentVariable($name,$prior[$name],'Process')}
}
