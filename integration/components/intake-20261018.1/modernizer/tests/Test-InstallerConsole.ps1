[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $root 'payload\DominionWarsInstall.psm1') -Force
$checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }; $script:checks++; Write-Host "PASS: $Message"
}
$script:FakeMode = [uint32]0x1F7
$script:Writes = 0
$reader = [Func[Nullable[uint32]]]{ return $script:FakeMode }
$writer = [Func[uint32,bool]]{ param([uint32]$Mode); $script:Writes++; $script:FakeMode = $Mode; return $true }
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
Assert ($guard.Available -and $guard.Changed -and ($script:FakeMode -band 0x40) -eq 0) 'QuickEdit is disabled for the guarded console'
Assert (($script:FakeMode -band 0x80) -ne 0) 'Extended flag is set as required for QuickEdit changes'
Assert (($script:FakeMode -band 1) -eq (0x1F7 -band 1)) 'Ctrl+C processed-input behavior is preserved'
Assert (($script:FakeMode -band 0xFFFFFF3F) -eq (0x1F7 -band 0xFFFFFF3F)) 'Unrelated input flags remain unchanged'
Assert ($guard.Restore() -eq 'Original console input mode restored.' -and $script:FakeMode -eq 0x1F7) 'Original console input mode is restored'
$writeCount = $script:Writes; $null = $guard.Restore()
Assert ($script:Writes -eq $writeCount) 'Repeated restoration does not rewrite console state'

$script:FakeMode = [uint32]0x1B7; $script:Writes = 0
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
Assert (-not $guard.Changed -and $script:Writes -eq 0) 'An already-disabled QuickEdit mode is untouched'
$null = $guard.Restore()
Assert ($script:FakeMode -eq 0x1B7 -and $script:Writes -eq 0) 'No unnecessary restore on an unchanged console'

$script:FakeMode = [uint32]0x177; $script:Writes = 0
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
$null = $guard.Restore()
Assert ($script:FakeMode -eq 0x177) 'Original mode without Extended can also be restored'

$script:FakeMode = [uint32]0x1F7
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
$script:FakeMode = $script:FakeMode -bxor 0x10
$laterMode = $script:FakeMode; $writeCount = $script:Writes
Assert ($guard.Restore() -like 'Preserving later*' -and $script:FakeMode -eq $laterMode -and $script:Writes -eq $writeCount) 'Later user/host console-mode changes are preserved'

$noConsole = [Func[Nullable[uint32]]]{ return $null }
$script:Writes = 0
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $noConsole,$writer
Assert (-not $guard.Available -and -not $guard.Changed -and $script:Writes -eq 0) 'Redirected/non-console input is a harmless no-op'
$null = $guard.Restore()
Assert ($script:Writes -eq 0) 'Non-console cleanup does not change anything'

$script:FakeMode = [uint32]0x1F7
$refused = [Func[uint32,bool]]{ param([uint32]$Mode); return $false }
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$refused
Assert (-not $guard.Changed -and $guard.Status -like 'Could not disable*' -and $script:FakeMode -eq 0x1F7) 'A refused console-mode change does not break installation'
$null = $guard.Restore()

$script:FakeMode = [uint32]0x1F6
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
Assert (($script:FakeMode -band 1) -eq 0) 'Originally disabled processed-input handling is not enabled'
$null = $guard.Restore()
$script:RefuseRestore = $true
# A second fixture selectively refuses restoration while accepting setup.
$conditionalWriter = [Func[uint32,bool]]{ param([uint32]$Mode); if ($script:RefuseRestore) { return $false }; $script:FakeMode = $Mode; return $true }
$script:RefuseRestore = $false; $script:FakeMode = [uint32]0x1F7
$restoreGuard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$conditionalWriter
$script:RefuseRestore = $true
Assert ($restoreGuard.Restore() -eq 'Console-mode restoration failed.') 'A restoration API failure is reported without throwing over the installer result'

$script:FakeMode = [uint32]0x1F7
$guard = New-Object DominionWars.InstallConsoleGuard -ArgumentList $reader,$writer
$script:FakeMode = $null
Assert ($guard.Restore() -like '*no longer available*') 'A disconnected console is not blindly restored'

# The real backend is also exercised without opening a window or changing global
# defaults. Automated hosts may have no console input handle, which is supported.
$nativeGuard = Start-DWInstallConsoleGuard
try { Assert ($null -ne $nativeGuard.Status) 'Native backend reports guarded, already-disabled or non-console status' }
finally { Stop-DWInstallConsoleGuard -Guard $nativeGuard }
Write-Host "PASS: $checks console-guard checks. Policy tests use simulated modes; real mouse selection still needs manual testing."
