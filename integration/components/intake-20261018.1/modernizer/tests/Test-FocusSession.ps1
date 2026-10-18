[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$VerifiedGame)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsFocus.psm1') -Force
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$scratch=Join-Path $root ('.test-focus-session-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Copy-Item -LiteralPath (Join-Path $VerifiedGame 'ds9dw.exe') -Destination $scratch
foreach ($name in @('ddraw.dll','DDrawCompat-Focus.dll','DDrawCompat-ds9dw.ini')) { Copy-Item -LiteralPath (Join-Path $root ('payload\'+$name)) -Destination $scratch }
$wrapper=Join-Path $scratch 'ddraw.dll'; $profile=Join-Path $scratch 'DDrawCompat-ds9dw.ini'
$pending=Join-Path $scratch '.dw-launcher\focus-session.json'
$baseline=Get-DominionWarsFileHash $wrapper; $original=Get-DominionWarsFileHash $profile
$script:checks=0
function Assert([bool]$Pass,[string]$Message) { if (-not $Pass) { throw "FAIL: $Message" }; $script:checks++; Write-Host "PASS: $Message" }
$record=Start-DominionWarsFocusSession $scratch
Assert ((Get-DominionWarsFileHash $wrapper) -eq $record.AppliedWrapperHash) 'Opt-in uses the exact passed focus wrapper'
Assert ([IO.File]::ReadAllText($profile) -match 'AltTabFix = noactivateapp\(0\)') 'Required activation profile applied'
Assert ($record.Phase -eq 'Applied' -and (Test-Path -LiteralPath $pending)) 'Recovery record committed'
$rejected=$false; try { $null=Start-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert $rejected 'Nested opt-in refuses to replace originals'
$null=Restore-DominionWarsFocusSession $scratch
Assert ((Get-DominionWarsFileHash $wrapper) -eq $baseline -and (Get-DominionWarsFileHash $profile) -eq $original) 'Exact stock wrapper and profile restored'
Assert (-not (Test-Path -LiteralPath $pending)) 'Completed record removed'
Assert ($null -eq (Restore-DominionWarsFocusSession $scratch)) 'Repeated cleanup is harmless'
$record=Start-DominionWarsFocusSession $scratch
$record.Phase='Prepared'; Write-DominionWarsRecoveryState -Path $pending -State $record
$null=Restore-DominionWarsFocusSession $scratch
Assert ((Get-DominionWarsFileHash $wrapper) -eq $baseline) 'Interrupted prepared transaction recovers'
$record=Start-DominionWarsFocusSession $scratch
[IO.File]::AppendAllText($profile,"`r`n# later user change")
$later=Get-DominionWarsFileHash $profile
$rejected=$false; try { $null=Restore-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert ($rejected -and (Get-DominionWarsFileHash $profile) -eq $later) 'Later profile edit preserved and launch recovery blocked'
Assert ((Get-DominionWarsFileHash $wrapper) -eq $baseline) 'Unchanged wrapper still restored on profile conflict'
$conflict=Restore-DominionWarsFocusSession $scratch -AllowConflicts
Assert ($conflict.PreservedChanges -and (Test-Path -LiteralPath (Join-Path $conflict.BackupPath 'conflict.json'))) 'Explicit repair/removal cleanup archives conflict evidence'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $profile -Force
$record=Start-DominionWarsFocusSession $scratch
$backup=Join-Path $scratch ('.dw-launcher\focus-backups\'+$record.BackupId+'\original.ini')
[IO.File]::AppendAllText($backup,'tamper')
$before=Get-DominionWarsFileHash $wrapper
$rejected=$false; try { $null=Restore-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert ($rejected -and (Get-DominionWarsFileHash $wrapper) -eq $before) 'All backups checked before any restoration'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $backup -Force
$record.BackupId='..\escape'; Write-DominionWarsRecoveryState -Path $pending -State $record
$rejected=$false; try { $null=Restore-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert $rejected 'Recovery cannot reference a backup outside this installation'
$record.BackupId=Split-Path -Leaf (Split-Path -Parent $backup)
Write-DominionWarsRecoveryState -Path $pending -State $record
$null=Restore-DominionWarsFocusSession $scratch
$focus=Join-Path $scratch 'DDrawCompat-Focus.dll'
Copy-Item -LiteralPath $profile -Destination $focus -Force
$rejected=$false; try { $null=Start-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert ($rejected -and (Get-DominionWarsFileHash $wrapper) -eq $baseline) 'Tampered optional binary refused without changing stock wrapper'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-Focus.dll') -Destination $focus -Force
$record=Start-DominionWarsFocusSession $scratch
Remove-Item -LiteralPath $profile
$rejected=$false; try { $null=Restore-DominionWarsFocusSession $scratch } catch { $rejected=$true }
Assert ($rejected -and -not (Test-Path -LiteralPath $profile)) 'Deliberate profile deletion preserved'
$null=Restore-DominionWarsFocusSession $scratch -AllowConflicts
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $profile
Initialize-DominionWarsDisplay
$mode=[DominionWarsDisplayMode]::GetCurrent([DominionWarsDisplayMode]::GetPrimaryDevice())
Assert ((ConvertTo-DominionWarsFocusSnapshot $mode).Length -eq 440) 'Actual display snapshot marshals without changing the display'
Write-Host "PASS: $script:checks focus transaction checks; no game or display change. $scratch"
