param([Parameter(Mandatory=$true)][string]$PackageRoot,
      [string]$VerifiedGame='E:\Codex\research\installed-1.04',
      [string]$TestRoot='E:\Codex\remaster-build-tests')
$ErrorActionPreference='Stop'
Import-Module (Join-Path $PackageRoot 'Remaster.Core.psm1') -Force
$manifest=Assert-RemasterPackage $PackageRoot
$root=Join-Path $TestRoot ('lifecycle-'+[guid]::NewGuid().ToString('N'))
$null=New-Item -ItemType Directory -Path $root
$engine=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
$checks=0
function Check([bool]$Value){if(-not $Value){throw 'Lifecycle check failed'};$script:checks++}
function Run-Installer([string[]]$Extra,[bool]$ExpectSuccess=$true) {
    & $engine -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PackageRoot 'Remaster.ps1') @Extra -NonInteractive -NoDesktopShortcut
    if($ExpectSuccess){Check ($LASTEXITCODE -eq 0)}else{Check ($LASTEXITCODE -ne 0)}
}
$a=Join-Path $root 'clone-a';$b=Join-Path $root 'clone-b'
$sourceExeHash=Get-RemasterHash (Join-Path $VerifiedGame 'ds9dw.exe')
Run-Installer @('-Action','Install','-SourcePath',$VerifiedGame,'-Destination',$a,'-Backend','D3D9On12')
$null=Test-RemasterInstalled $a;$checks++
$savePath=Join-Path $a 'integration-save-preservation.sav'
[IO.File]::WriteAllBytes($savePath,[Text.Encoding]::UTF8.GetBytes('private lifecycle fixture, not a real player save'))
$saveHash=Get-RemasterHash $savePath
Run-Installer @('-Action','Install','-SourcePath',$VerifiedGame,'-Destination',$a) $false
Check ((Get-RemasterHash $savePath) -eq $saveHash)
Run-Installer @('-Action','Repair','-Destination',$a)
$null=Test-RemasterInstalled $a;$checks++
Check ((Get-RemasterHash $savePath) -eq $saveHash)
Check (@(Get-ChildItem -LiteralPath $root -Directory -Filter '.dw-remaster-previous-*').Count -eq 1)
Run-Installer @('-Action','Repair','-Destination',$a,'-Backend','Native')
$nativeState=Test-RemasterInstalled $a
Check ($nativeState.profile -eq 'native')
Run-Installer @('-Action','Repair','-Destination',$a,'-Backend','D3D9On12')
$restoredState=Test-RemasterInstalled $a
Check ($restoredState.profile -eq 'd3d9on12')
Run-Installer @('-Action','Install','-SourcePath',$a,'-Destination',$b)
$null=Test-RemasterInstalled $b;$checks++
Check ((Get-RemasterHash (Join-Path $b 'integration-save-preservation.sav')) -eq $saveHash)
Run-Installer @('-Action','Remove','-Destination',$b)
Check ((Get-RemasterHash (Join-Path $b 'ds9dw.exe')) -eq $manifest.engine.input_sha256)
Check ((Get-RemasterHash (Join-Path $b 'integration-save-preservation.sav')) -eq $saveHash)
Check (-not(Test-Path -LiteralPath (Join-Path $b 'Launch-Remaster.ps1')))
Check (Test-Path -LiteralPath (Join-Path $b 'Play Dominion Wars.cmd'))
Check ((Get-RemasterHash (Join-Path $VerifiedGame 'ds9dw.exe')) -eq $sourceExeHash)
# Simulate only the commit-boundary interruption on our own clone, not a game.
$transaction=[guid]::NewGuid().ToString('N')
$backup=Join-Path $root ('.dw-remaster-previous-'+$transaction)
$journal=Join-Path $root ('.dw-remaster-transaction-'+$transaction+'.json')
$null=Assert-RemasterPlainPath $a;$null=Assert-RemasterPlainPath $backup
Write-RemasterJson $journal @{schema=1;id=$transaction;destination=$a;previous=$backup;phase='PreviousPreserved'}
Move-Item -LiteralPath $a -Destination $backup
Run-Installer @('-Action','Recover','-Destination',$a,'-RecoveryPath',$journal)
$null=Test-RemasterInstalled $a;$checks++
Check ((Get-RemasterHash (Join-Path $a 'integration-save-preservation.sav')) -eq $saveHash)
# Source nesting and unsupported incomplete input must fail without a final tree.
$beforeNested=@(Get-ChildItem -LiteralPath $a -Force | ForEach-Object {$_.Name})
Run-Installer @('-SourcePath',$a,'-Destination',(Join-Path $a 'bad-nested')) $false
Check (-not(Test-Path -LiteralPath (Join-Path $a 'bad-nested')))
$afterNested=@(Get-ChildItem -LiteralPath $a -Force | ForEach-Object {$_.Name})
Check (@(Compare-Object $beforeNested $afterNested).Count -eq 0)
$badSource=Join-Path $root 'unsupported-input';$null=New-Item -ItemType Directory -Path $badSource
[IO.File]::WriteAllBytes((Join-Path $badSource 'ds9dw.exe'),[Text.Encoding]::UTF8.GetBytes('unsupported'))
$badDestination=Join-Path $root 'unsupported-output'
Run-Installer @('-SourcePath',$badSource,'-Destination',$badDestination) $false
Check (-not(Test-Path -LiteralPath $badDestination))
Write-RemasterJson (Join-Path $root 'result.json') @{status='pass';checks=$checks;package_manifest_sha256=(Get-RemasterHash (Join-Path $PackageRoot 'manifest.json'));build_id=$manifest.build_id;verified_original_unchanged=$true;save_preserved=$true;gameplay_tested=$false}
Write-Host "PASS: $checks lifecycle checks. Fixtures/backups retained at $root"
