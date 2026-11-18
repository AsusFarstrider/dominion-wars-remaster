[CmdletBinding()]
param([string]$GameRoot,[switch]$VerifyOnly)
$ErrorActionPreference='Stop'
if(-not $GameRoot){$GameRoot=Split-Path -Parent $MyInvocation.MyCommand.Path}
$GameRoot=[IO.Path]::GetFullPath($GameRoot)
Import-Module (Join-Path $GameRoot 'Remaster.Core.psm1') -Force
$helper=$null;$child=$null;$mutex=$null;$owns=$false;$code=1
$prior=@{}
$outputSelection=$null
try {
    if(-not $VerifyOnly){Assert-RemasterIdle}
    $state=Test-RemasterInstalled $GameRoot
    $manifest=Read-RemasterJson (Join-Path $GameRoot '.dw-remaster\manifest.json')
    $identity=Get-RemasterIdentity $GameRoot $manifest
    $displayName=if($identity -and $identity.remaster_version){'Dominion Wars Remastered '+$identity.remaster_version+' '+$identity.release_stage+' ('+$state.build_id+')'}else{'remaster '+$state.build_id}
    if($VerifyOnly){Write-Host "Verified $displayName.";exit 0}
    if($manifest.PSObject.Properties['rendering']) {
        Import-Module (Join-Path $GameRoot 'DominionWarsLauncher.psm1') -Force
        Initialize-DominionWarsDisplay
        $physicalMode=[DominionWarsDisplayMode]::GetCurrent([DominionWarsDisplayMode]::GetPrimaryDevice())
        Assert-RemasterOutput $manifest $physicalMode
        if($manifest.rendering.profile -ceq 'native-display-v1') {
            $outputDevice=[DominionWarsDisplayMode]::GetPrimaryDevice()
            $outputMode=[DominionWarsDisplayMode]::GetCurrent($outputDevice)
            $outputSelection=Get-RemasterOutputSelection $manifest $outputMode $outputDevice ([DominionWarsDisplayMode]::GetDeviceIdentity($outputDevice))
        }
    }
    # The native recenter patch is validated only for the primary origin (0,0).
    Add-Type -AssemblyName System.Windows.Forms
    $primary=[Windows.Forms.Screen]::PrimaryScreen
    if($primary.Bounds.X -ne 0 -or $primary.Bounds.Y -ne 0){throw 'This candidate requires the primary display at desktop origin (0,0).'}
    foreach($config in @((Join-Path $env:PROGRAMDATA 'DDrawCompat\DDrawCompat.ini'),(Join-Path $env:LOCALAPPDATA 'DDrawCompat\DDrawCompat.ini'),(Join-Path $GameRoot 'DDrawCompat.ini'),(Join-Path $GameRoot 'DDrawCompatOverlay-ds9dw.ini'))) {
        if(Test-Path -LiteralPath $config){throw "Additional wrapper configuration requires review: $config"}
    }
    $mutex=New-Object Threading.Mutex($false,'Local\DominionWarsRemaster.Launch')
    try {$owns=$mutex.WaitOne(0)}catch [Threading.AbandonedMutexException]{$owns=$true}
    if(-not $owns){throw 'Another remaster launcher is active.'}
    $run=Join-Path $GameRoot ('.dw-remaster\runs\'+[DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')+'-'+[guid]::NewGuid().ToString('N'))
    $null=New-Item -ItemType Directory -Path $run
    $clearNames=@('PSModulePath','DW_EXPERIMENTAL_FOCUS_GUARD','DW_EXPERIMENTAL_FOCUS_DESKTOP','DW_FOCUS_DEVICE','DW_FOCUS_DEVICE_IDENTITY','DW_FOCUS_ORIGINAL_MODE','DW_REMASTER_OUTPUT_WIDTH','DW_REMASTER_OUTPUT_HEIGHT','DW_RESOLUTION_TELEMETRY','DW_DRAW_TELEMETRY','DW_PROJECTION_TELEMETRY','DW_COORDINATE_PROBE')+@([Environment]::GetEnvironmentVariables('Process').Keys | Where-Object {$_ -like 'QFR_*'})
    foreach($name in ($clearNames | Select-Object -Unique)) {
        $prior[$name]=[Environment]::GetEnvironmentVariable($name,'Process')
        [Environment]::SetEnvironmentVariable($name,$null,'Process')
    }
    $env:PSModulePath=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\Modules'
    $env:DW_EXPERIMENTAL_FOCUS_GUARD='1'
    if($outputSelection){
        Set-RemasterOutputEnvironment $outputSelection
        Write-RemasterJson (Join-Path $run 'output-selection.json') $outputSelection
        Write-Host ("Selected primary physical output {0}x{1}; no desktop mode change or live resize." -f $outputSelection.width,$outputSelection.height)
    }
    $engine=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
    $helperArgs='-NoProfile -ExecutionPolicy Bypass -File "'+(Join-Path $GameRoot 'Watch-RemasterFocus.ps1')+'" -GamePath "'+(Join-Path $GameRoot 'ds9dw.exe')+'" -LogPath "'+(Join-Path $run 'focus.log')+'"'
    $helper=Start-Process -FilePath $engine -ArgumentList $helperArgs -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $run 'helper-output.log') -RedirectStandardError (Join-Path $run 'helper-error.log')
    $null=$helper.Handle
    $deadline=[DateTime]::UtcNow.AddSeconds(20)
    while(-not(Test-Path -LiteralPath (Join-Path $run 'focus.log'))) {
        if($helper.HasExited -or [DateTime]::UtcNow -ge $deadline){throw 'Focus helper failed preflight; game not launched.'}
        Start-Sleep -Milliseconds 100
    }
    Write-Host "Launching $displayName; no startup desktop mode change. Pause manually before changing focus."
    $argsChild='-NoProfile -ExecutionPolicy Bypass -File "'+(Join-Path $GameRoot 'Launch-NoDesktopSwitch.ps1')+'" -GamePath "'+(Join-Path $GameRoot 'ds9dw.exe')+'" -GraphicsBackend Profile -VisualPreset Baseline'
    if($outputSelection){
        $recheckDevice=[DominionWarsDisplayMode]::GetPrimaryDevice()
        $recheckMode=[DominionWarsDisplayMode]::GetCurrent($recheckDevice)
        Assert-RemasterOutputUnchanged $outputSelection $recheckMode $recheckDevice ([DominionWarsDisplayMode]::GetDeviceIdentity($recheckDevice))
    }
    $child=Start-Process -FilePath $engine -ArgumentList $argsChild -WorkingDirectory $GameRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $run 'launcher-output.log') -RedirectStandardError (Join-Path $run 'launcher-error.log')
    $null=$child.Handle;$child.WaitForExit();$child.Refresh();$code=$child.ExitCode
    if(-not $helper.WaitForExit(5000)){Stop-Process -Id $helper.Id -ErrorAction SilentlyContinue}
    $null=Test-RemasterInstalled $GameRoot
    $result=[ordered]@{build_id=$state.build_id;launcher_exit=$code;managed_files_verified=$true;recorded_utc=[DateTime]::UtcNow.ToString('o');automatic_pause=$false}
    if($outputSelection){$result.output_selection=$outputSelection}
    if($identity){$result.version_identity=$identity;$result.version_identity_sha256=$manifest.version_identity.sha256}
    Write-RemasterJson (Join-Path $run 'result.json') $result
    Write-Host "Session complete (exit $code). Logs: $run"
} catch {Write-Error -ErrorAction Continue $_; $code=1}
finally {
    if($helper -and -not $helper.HasExited){Stop-Process -Id $helper.Id -ErrorAction SilentlyContinue}
    foreach($name in $prior.Keys){[Environment]::SetEnvironmentVariable($name,$prior[$name],'Process')}
    if($owns){$mutex.ReleaseMutex()};if($mutex){$mutex.Dispose()}
}
exit $code
