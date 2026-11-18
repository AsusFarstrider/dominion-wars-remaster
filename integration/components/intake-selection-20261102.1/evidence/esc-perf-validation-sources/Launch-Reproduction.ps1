param([string]$GameRoot='E:\Codex\remaster-build-tests\esc-main-menu-reentry-20261029-01')
$ErrorActionPreference='Stop'
$game=[IO.Path]::GetFullPath($GameRoot)
if(Get-Process ds9dw -ErrorAction SilentlyContinue){throw 'An existing game is running; leave it untouched.'}
$launcher=Join-Path $game 'Launch-Remaster.ps1'
$process=Start-Process -FilePath "$env:WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$launcher+'"')) -WorkingDirectory $game -WindowStyle Hidden -PassThru
Write-Output "Managed launcher PID: $($process.Id)"
for($i=0;$i -lt 40;$i++){
 $running=Get-Process ds9dw -ErrorAction SilentlyContinue
 if($running){$running | Select-Object Id,Path;exit 0}
 Start-Sleep -Milliseconds 250
}
Write-Output 'No game process observed yet; inspect the managed run logs.'
