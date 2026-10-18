param([string]$TestRoot='E:\Codex\remaster-build-tests')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
$directory=Join-Path $TestRoot ('core-'+[guid]::NewGuid().ToString('N'))
$null=New-Item -ItemType Directory -Path $directory -Force
$checks=0
function Check([bool]$Value){if(-not $Value){throw 'Check failed'};$script:checks++}
function Refused([scriptblock]$Call){$failed=$false;try{& $Call}catch{$failed=$true};Check $failed}
$inputPath=Join-Path $directory 'source.bin'
[IO.File]::WriteAllBytes($inputPath,[Text.Encoding]::UTF8.GetBytes('abc'))
$expected=Join-Path $directory 'expected.bin'
[IO.File]::WriteAllBytes($expected,[Text.Encoding]::UTF8.GetBytes('aZc!'))
$delta=[ordered]@{schema=1;input_sha256=(Get-RemasterHash $inputPath);input_size=3;output_sha256=(Get-RemasterHash $expected);output_size=4;operations=@(@{kind='copy';offset=0;length=1},@{kind='bytes';data='Wg=='},@{kind='copy';offset=2;length=1},@{kind='bytes';data='IQ=='})}
$deltaPath=Join-Path $directory 'delta.json';Write-RemasterJson $deltaPath $delta
$output=Join-Path $directory 'output.bin'
Invoke-RemasterDelta $inputPath $deltaPath $output
Check ((Get-RemasterHash $output) -eq (Get-RemasterHash $expected))
Refused {Invoke-RemasterDelta $inputPath $deltaPath $output}
Refused {Get-RemasterChild $directory '..\outside'}
Refused {Get-RemasterChild $directory 'C:\outside'}
Refused {Get-RemasterChild $directory 'bad:stream'}
Refused {Assert-RemasterPlainPath 'C:\'}
Refused {Assert-RemasterHash $inputPath ('0'*64)}
$delta.operations[0].offset=-1;Write-RemasterJson $deltaPath $delta
Refused {Invoke-RemasterDelta $inputPath $deltaPath (Join-Path $directory 'bad.bin')}
Check (-not(Test-Path -LiteralPath (Join-Path $directory 'bad.bin')))
$delta.operations[0].offset=0;$delta.output_sha256='0'*64;Write-RemasterJson $deltaPath $delta
Refused {Invoke-RemasterDelta $inputPath $deltaPath (Join-Path $directory 'bad.bin')}
Check (-not(Test-Path -LiteralPath (Join-Path $directory 'bad.bin')))
$badManifest=Join-Path $directory 'package';$null=New-Item -ItemType Directory $badManifest
Write-RemasterJson (Join-Path $badManifest 'manifest.json') @{schema=1;product='Dominion Wars Remaster';files=@(@{path='..\source.bin';size=3;sha256=(Get-RemasterHash $inputPath)})}
Refused {Assert-RemasterPackage $badManifest}
$goodPackage=Join-Path $directory 'valid-package';$null=New-Item -ItemType Directory $goodPackage
$payload=Join-Path $goodPackage 'payload.txt'
[IO.File]::WriteAllText($payload,'verified fixture')
Write-RemasterJson (Join-Path $goodPackage 'manifest.json') @{schema=1;product='Dominion Wars Remaster';files=@(@{path='payload.txt';size=(Get-Item -LiteralPath $payload).Length;sha256=(Get-RemasterHash $payload)})}
$null=Assert-RemasterPackage $goodPackage;$checks++
[IO.File]::WriteAllText((Join-Path $goodPackage 'unexpected.txt'),'not part of package')
Refused {Assert-RemasterPackage $goodPackage}
[IO.File]::WriteAllText($payload,'changed fixture')
Refused {Assert-RemasterPackage $goodPackage}
Write-Host "PASS: $checks core checks. Fixtures retained at $directory"
