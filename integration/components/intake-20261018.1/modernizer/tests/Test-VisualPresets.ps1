[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsLauncher.psm1') -Force
$scratch = Join-Path $root ('.test-visual-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
$profile = Join-Path $scratch 'DDrawCompat-ds9dw.ini'
Copy-Item -LiteralPath (Join-Path $root 'payload\DDrawCompat-ds9dw.ini') -Destination $profile
$null = Set-DominionWarsGraphicsBackend -ProfilePath $profile -Backend Native
Add-Content -LiteralPath $profile -Value "# User setting retained`r`nTextureFilter = app`r`nAntialiasing = msaa2x" -Encoding UTF8
$before = Get-DominionWarsFileHash $profile
$checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    $script:checks++; Write-Host "PASS: $Message"
}
$baseline = Get-DominionWarsVisualSettings Baseline
foreach ($preset in @('Sharper', 'Enhanced3D', 'ResourceSaving')) {
    $settings = Get-DominionWarsVisualSettings $preset
    Assert (@($settings.Keys | Where-Object { $settings[$_] -ne $baseline[$_] }).Count -eq 1) "$preset isolates one enhancement"
    $record = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset $preset
    $text = [IO.File]::ReadAllText($profile)
    foreach ($entry in $settings.GetEnumerator()) {
        $matches = @([regex]::Matches($text, '(?im)^[\t ]*' + [regex]::Escape($entry.Key) + '[\t ]*=[\t ]*([^\r\n]*)'))
        Assert ($matches.Count -gt 0 -and @($matches | Where-Object { $_.Groups[1].Value.Trim() -ne $entry.Value }).Count -eq 0) "$preset applies $($entry.Key) without conflicting duplicate values"
    }
    Assert ($text -match 'ForceD3D9On12 = off' -and $text -match 'PalettizedTextures = on' -and
        $text -match 'CapsPatches = D3DDEVICEDESC.dpcTriCaps.dwTextureCaps\|=0x2' -and
        $text -match 'CompatFixes = singlemonitor' -and $text -match '# User setting retained') "$preset preserves backend, core fixes and unrelated content"
    $rejected = $false
    try { $null = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Sharper } catch { $rejected = $true }
    Assert $rejected 'Pending recovery cannot be overwritten by another preset'
    $result = Restore-DominionWarsVisualPreset $scratch
    Assert (-not $result.PreservedChanges -and (Get-DominionWarsFileHash $profile) -eq $before) "$preset restores exact original profile bytes"
    Assert (Test-Path -LiteralPath $record.BackupPath) 'Original backup remains available'
}
$null = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Baseline
Assert ((Get-DominionWarsFileHash $profile) -eq $before -and
    -not (Test-Path -LiteralPath (Join-Path $scratch '.dw-launcher\visual-preset.json'))) 'Baseline does not edit a customized profile or create recovery state'

$record = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Enhanced3D
Add-Content -LiteralPath $profile -Value '# Later user edit' -Encoding UTF8
$editedHash = Get-DominionWarsFileHash $profile
$result = Restore-DominionWarsVisualPreset $scratch
Assert ($result.PreservedChanges -and (Get-DominionWarsFileHash $profile) -eq $editedHash) 'Recovery preserves later user changes'
Assert (Test-Path -LiteralPath (Join-Path (Split-Path -Parent $record.BackupPath) 'conflict.json')) 'Conflicting recovery record is archived alongside original backup'
Copy-Item -LiteralPath $record.BackupPath -Destination $profile -Force

$record = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Sharper
Copy-Item -LiteralPath $record.BackupPath -Destination $profile -Force
$result = Restore-DominionWarsVisualPreset $scratch
Assert (-not $result.PreservedChanges -and (Get-DominionWarsFileHash $profile) -eq $before) 'Already-restored profile safely closes a stale recovery record'

$record = Start-DominionWarsVisualPreset -GameRoot $scratch -Preset Sharper
Copy-Item -LiteralPath $profile -Destination $record.BackupPath -Force
$rejected = $false
try { $null = Restore-DominionWarsVisualPreset $scratch } catch { $rejected = $_.Exception.Message -match 'backup has changed' }
Assert $rejected 'Tampered original backup is rejected before restoration'
Write-Host "$checks visual-preset checks passed without game or display changes. Disposable output: $scratch"
