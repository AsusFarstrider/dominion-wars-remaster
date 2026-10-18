[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$scratch = Join-Path $root ('.test-shortcuts-' + [guid]::NewGuid().ToString('N'))
$desktop = Join-Path $scratch 'Fake desktop'
$game = Join-Path $scratch 'Fake game'
New-Item -ItemType Directory -Path $desktop,$game -Force | Out-Null
. (Join-Path $root 'DominionWarsModernizer.ps1') Status -Destination $game | Out-Null
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }; Write-Host "PASS: $Message"
}
$shell = New-Object -ComObject WScript.Shell
$path = Join-Path $desktop 'Dominion Wars.lnk'
$original = $shell.CreateShortcut($path)
$original.TargetPath = Join-Path $env:SystemRoot 'notepad.exe'; $original.Description = 'Existing user shortcut'; $original.Save()
$originalHash = Get-Sha256 $path
$installed = Install-DesktopShortcut -GameRoot $game -DesktopDirectory $desktop
$state = [pscustomobject]@{ DesktopShortcut = $installed.Path; DesktopShortcutHash = $installed.Hash; DesktopShortcutBackupName = $installed.BackupName }
Assert ($installed.BackupName -eq 'DesktopShortcut.original.lnk') 'Existing same-named shortcut is backed up'
Assert (Test-OwnedDesktopShortcut -Path $path -GameRoot $game -State $state) 'Created shortcut has verifiable ownership'
Remove-DesktopShortcut -GameRoot $game -State $state
Assert ((Get-Sha256 $path) -eq $originalHash) 'Removal restores original shortcut bytes'
$installed = Install-DesktopShortcut -GameRoot $game -DesktopDirectory $desktop
$state.DesktopShortcutHash = $installed.Hash
$changed = $shell.CreateShortcut($path); $changed.Description = 'User changed this'; $changed.Save()
$changedHash = Get-Sha256 $path
Assert (-not (Test-OwnedDesktopShortcut -Path $path -GameRoot $game -State $state)) 'User-edited shortcut no longer matches owned hash'
Remove-DesktopShortcut -GameRoot $game -State $state
Assert ((Get-Sha256 $path) -eq $changedHash) 'Removal preserves modified shortcut'
$recovered = @(Get-ChildItem -LiteralPath $desktop -Filter '*before modernizer*.lnk')
Assert ($recovered.Count -eq 1 -and (Get-Sha256 $recovered[0].FullName) -eq $originalHash) 'Original shortcut is recoverable under a separate name'
$script:fakeRunningGamePath = Join-Path $game 'ds9dw.exe'
function Get-Process {
    [CmdletBinding()]
    param([string]$Name)
    [pscustomobject]@{ Path = $script:fakeRunningGamePath }
}
$rejected = $false
try { Assert-GameNotRunning -GameRoot $game } catch { $rejected = $_.Exception.Message -match 'Close Dominion Wars' }
Assert $rejected 'A running target game blocks compatibility-file changes'
Assert-GameNotRunning -GameRoot (Join-Path $scratch 'Other game')
Assert $true 'Unrelated game installation does not block the requested target'
Write-Host "Shortcut checks passed without touching the real desktop. Disposable output: $scratch"
