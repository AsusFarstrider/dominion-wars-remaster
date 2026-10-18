[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$CabinetDirectory,
    [Parameter(Mandatory = $true)][string]$VerifiedGame
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
$scratch = Join-Path $root ('.test-installer-media-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
. (Join-Path $root 'DominionWarsModernizer.ps1') Status -Destination (Join-Path $scratch 'unused') | Out-Null
$log = Start-DWInstallLog -Action Test -Root $scratch
$checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }; $script:checks++; Write-Host "PASS: $Message"
}
$media = Join-Path $scratch 'Owner media fixture'
New-Item -ItemType Directory -Path $media | Out-Null
foreach ($name in @('data1.cab','data1.hdr','data2.cab')) {
    Copy-Item -LiteralPath (Join-Path $CabinetDirectory $name) -Destination $media
}
# Real owner-supplied cabinet bytes, but placeholder resource files. No gameplay
# claim: this test intentionally replaces the interactive official updater.
foreach ($name in @('rs\Bolts\ds9blt.blt','rs\Bolts\ds9dat.blt','rs\Bolts\ds9media.blt','rs\Video\IntroDS9.bik')) {
    $path = Join-Path $media $name
    New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
    [IO.File]::WriteAllText($path, 'media structural fixture, not playable')
}
$extractor = Resolve-CabExtractorPath
$script:FixtureVerifiedGame = $VerifiedGame
function Invoke-OfficialPatch {
    param([string]$Patch, [string]$GameRoot)
    Write-DWInstallEvent 'TEST ONLY: substitute verified owner executable; official patch UI is not tested.'
    Copy-Item -LiteralPath (Join-Path $script:FixtureVerifiedGame 'ds9dw.exe') -Destination (Join-Path $GameRoot 'ds9dw.exe') -Force
}
$target = Join-Path $scratch 'Reconstructed fixture'
New-GameTreeFromMedia -MediaRoot $media -Patch 'test stub' -Extractor $extractor -Target $target
Assert ((Get-GameVersion $target).Version -eq '1.04') 'Media reconstruction reaches verified output with explicit patch substitution'
Assert ((Get-Item -LiteralPath (Join-Path $target 'binkw32.dll')).Length -gt 0) 'Real isolated extractor reconstructs supporting DLL'
Assert ((Get-Item -LiteralPath (Join-Path $target 'rs\Missions\shipcapt.mis')).Length -gt 0) 'Real isolated extractor reconstructs mission data'
Assert (@(Get-ChildItem -LiteralPath $scratch -Directory -Filter '.dw-media-stage-*').Count -eq 0) 'Successful reconstruction leaves no temporary stage'
Assert (@(Get-ChildItem -LiteralPath $scratch -Filter '*.stderr.log').Count -eq 5) 'Each of the five real extractions has persistent diagnostics'

function Invoke-OfficialPatch { param([string]$Patch, [string]$GameRoot); throw 'Intentional patch failure fixture' }
$failedTarget = Join-Path $scratch 'Failed patch target'
$rejected = $false
try { New-GameTreeFromMedia -MediaRoot $media -Patch 'test stub' -Extractor $extractor -Target $failedTarget } catch { $rejected = $true }
Assert ($rejected -and -not (Test-Path -LiteralPath $failedTarget)) 'Patch failure cannot commit the destination'
Assert (@(Get-ChildItem -LiteralPath $scratch -Directory -Filter '.dw-media-stage-*').Count -eq 0) 'Failed patch stage is removed after extraction workers finish'
Assert (Test-Path -LiteralPath $log) 'Staging cleanup does not erase diagnostic evidence'

function Invoke-CabExtraction { param($Extractor,$Cabinet,$Index,$OutputPath); throw 'Intentional extraction failure fixture' }
$rejected = $false
try { New-GameTreeFromMedia -MediaRoot $media -Patch 'test stub' -Extractor $extractor -Target $failedTarget } catch { $rejected = $true }
Assert ($rejected -and -not (Test-Path -LiteralPath $failedTarget)) 'Extraction failure cannot commit the destination'
Assert (@(Get-ChildItem -LiteralPath $scratch -Directory -Filter '.dw-media-stage-*').Count -eq 0) 'Extraction failure also removes staging'
$damagedFixture = Join-Path $media 'data1.cab'
# Disc-derived copies may retain the read-only attribute. Change only our own
# disposable copy, never the supplied cabinet directory.
[IO.File]::SetAttributes($damagedFixture, [IO.FileAttributes]::Normal)
[IO.File]::WriteAllText($damagedFixture, 'intentionally damaged fixture')
$rejected = $false
try { New-GameTreeFromMedia -MediaRoot $media -Patch 'test stub' -Extractor $extractor -Target $failedTarget } catch { $rejected = $_.Exception.Message -match 'unexpected SHA-256' }
Assert $rejected 'Cabinet corruption is rejected before extraction'
Write-Host "PASS: $checks installer media checks. Private media evidence retained at $scratch"
