param()
$ErrorActionPreference='Stop'
$game='E:\Codex\remaster-build-tests\adaptive-output-20261030-01'
if(Get-Process ds9dw -ErrorAction SilentlyContinue){throw 'An existing game is running; leave it untouched.'}
$launcher=Join-Path $game 'Launch-Remaster.ps1'
$stamp=[DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
$outerOut=Join-Path $game ('.dw-remaster\outer-launch-'+$stamp+'.log')
$outerError=Join-Path $game ('.dw-remaster\outer-launch-'+$stamp+'-error.log')
$process=Start-Process -FilePath "$env:WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe" -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$launcher+'"')) -WorkingDirectory $game -WindowStyle Hidden -PassThru -RedirectStandardOutput $outerOut -RedirectStandardError $outerError
Write-Output "Managed launcher PID: $($process.Id)"
for($i=0;$i -lt 100;$i++){
 $running=Get-Process ds9dw -ErrorAction SilentlyContinue
 if($running){
  if($running.Count -ne 1 -or $running.Path -ne (Join-Path $game 'ds9dw.exe')){throw 'Unexpected game process; do not attach sampler'}
  $out=Join-Path $game ('.dw-remaster\output-evidence-'+$stamp)
  $sampler=Join-Path $PSScriptRoot 'sample_output.py'
  $arguments='"'+$sampler+'" --pid '+$running.Id+' --game "'+$game+'" --out "'+$out+'"'
  $reader=Start-Process -FilePath 'C:\Python314\python.exe' -ArgumentList $arguments -WorkingDirectory $game -WindowStyle Hidden -PassThru -RedirectStandardOutput ($out+'-sampler.log') -RedirectStandardError ($out+'-sampler-error.log')
  Write-Output "Game PID: $($running.Id); read-only sampler PID: $($reader.Id); evidence: $out";exit 0
 }
 Start-Sleep -Milliseconds 250
}
throw 'No game process observed. Inspect managed launcher logs before retrying.'
