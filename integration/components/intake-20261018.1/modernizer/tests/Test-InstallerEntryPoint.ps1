[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$VerifiedGame)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
$scratch = Join-Path $root ('.test-installer-entry-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Import-Module (Join-Path $root 'payload\DominionWarsInstall.psm1') -Force
$null = Start-DWInstallLog -Action Test -Root $scratch
$checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }; $script:checks++; Write-Host "PASS: $Message"
}
$fixture = Join-Path $scratch 'Verified structural fixture'
New-Item -ItemType Directory -Path $fixture | Out-Null
Copy-Item -LiteralPath (Join-Path $VerifiedGame 'ds9dw.exe') -Destination $fixture
foreach ($name in @('binkw32.dll','rs\Bolts\ds9blt.blt','rs\Bolts\ds9dat.blt', 'rs\Bolts\ds9media.blt','rs\Missions\shipcapt.mis','rs\Video\IntroDS9.bik')) {
    $path = Join-Path $fixture $name
    New-Item -ItemType Directory -Path (Split-Path -Parent $path) -Force | Out-Null
    [IO.File]::WriteAllText($path, 'structural fixture, not playable')
}
$marker = Join-Path $scratch 'successful-install.marker'
$logs = Join-Path $scratch 'Persistent install logs'
$target = Join-Path $scratch 'Fresh installation with spaces'
$modernizer = Join-Path $root 'DominionWarsModernizer.ps1'
$powershell = Join-Path $PSHOME 'powershell.exe'
$arguments = @('-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass', '-File', $modernizer, 'Install', '-SourcePath', $fixture, '-Destination', $target, '-NoDesktopShortcut', '-LogDirectory', $logs, '-CompletionMarker', $marker)
Invoke-DWInstallHelper -Executable $powershell -Arguments $arguments -ExpectedOutput $marker -Label 'Installer entry success' -LogRoot $scratch
Assert ((Get-Content -LiteralPath $marker -Raw).TrimEnd() -eq 'Dominion Wars installation completed successfully.') 'Success marker is written only after the install workflow finishes'
Assert (Test-Path -LiteralPath (Join-Path $target '.dw-modernizer\state.json')) 'A copied-source install commits compatibility state'
$installLog = Get-ChildItem -LiteralPath $logs -Filter 'Install-*.log' | Select-Object -First 1
Assert ((Get-Content -LiteralPath $installLog.FullName -Raw) -match 'Install completed successfully') 'Successful operation has a persistent final log result'

$badMarker = Join-Path $scratch 'failed-install.marker'
$badArguments = @('-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-File',$modernizer,'Install','-SourcePath',(Join-Path $scratch 'missing source'),'-Destination',(Join-Path $scratch 'must not be installed'),'-NoDesktopShortcut','-LogDirectory',$logs,'-CompletionMarker',$badMarker)
$failed = $false
try { Invoke-DWInstallHelper -Executable $powershell -Arguments $badArguments -Label 'Installer entry failure' -LogRoot $scratch } catch { $failed = $true }
Assert ($failed -and -not (Test-Path -LiteralPath $badMarker)) 'Failed installation returns failure without a success marker'
$failedLogs = @(Get-ChildItem -LiteralPath $logs -Filter 'Install-*.log' | Where-Object { (Get-Content -LiteralPath $_.FullName -Raw) -match 'FAILED:' })
Assert ($failedLogs.Count -eq 1 -and (Get-Content -LiteralPath $failedLogs[0].FullName -Raw) -match 'did not complete') 'Errors retain cause and unsuccessful final state outside game staging'

# Test the actual batch entry point against a no-op script, not the real installer.
# An exit-zero interruption simulation must not be advertised as installation success.
$batchFixture = Join-Path $scratch 'Batch fixture'
New-Item -ItemType Directory -Path $batchFixture | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'Install Dominion Wars.cmd') -Destination $batchFixture
$stub = @'
param([string]$Action, [string]$CompletionMarker)
if ($env:DW_TEST_ENTRY_MODE -eq 'success') { [IO.File]::WriteAllText($CompletionMarker, 'Dominion Wars installation completed successfully.' + [Environment]::NewLine) }
if ($env:DW_TEST_ENTRY_MODE -eq 'partial') { [IO.File]::WriteAllText($CompletionMarker, 'incomplete fixture marker') }
exit 0
'@
[IO.File]::WriteAllText((Join-Path $batchFixture 'DominionWarsModernizer.ps1'), $stub)
foreach ($mode in @('success', 'interrupted', 'partial')) {
    $info = New-Object Diagnostics.ProcessStartInfo
    $info.FileName = Join-Path $env:SystemRoot 'System32\cmd.exe'
    $info.Arguments = '/d /c ""' + (Join-Path $batchFixture 'Install Dominion Wars.cmd') + '""'
    $info.UseShellExecute = $false; $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true; $info.RedirectStandardError = $true; $info.RedirectStandardInput = $true
    $info.EnvironmentVariables['DW_TEST_ENTRY_MODE'] = $mode
    $info.EnvironmentVariables['TEMP'] = $scratch
    $child = New-Object Diagnostics.Process
    $child.StartInfo = $info
    try {
        $null = $child.Start(); $child.StandardInput.Close()
        $stdoutTask = $child.StandardOutput.ReadToEndAsync(); $stderrTask = $child.StandardError.ReadToEndAsync()
        if (-not $child.WaitForExit(15000)) { throw 'Batch fixture exceeded 15 seconds.' }
        $stdout = $stdoutTask.Result; $stderr = $stderrTask.Result
        [IO.File]::WriteAllText((Join-Path $scratch ("batch-$mode.stdout.log")), $stdout)
        [IO.File]::WriteAllText((Join-Path $scratch ("batch-$mode.stderr.log")), $stderr)
        if ($mode -eq 'success') {
            Assert ($child.ExitCode -eq 0 -and $stdout -match 'Installation completed successfully') 'Double-click entry accepts explicit completion'
        } else {
            Assert ($child.ExitCode -ne 0 -and $stdout -notmatch 'Installation completed successfully' -and $stdout -match 'Installation did not complete') "Exit zero without valid completion cannot print a false success: $mode"
        }
        Assert (@(Get-ChildItem -LiteralPath $scratch -Filter 'dw-install-*.success').Count -eq 0) "Batch result file is cleaned up: $mode"
    } finally {
        if (-not $child.HasExited) { $child.Kill(); $child.WaitForExit() }
        $child.Dispose()
    }
}
Write-Host "PASS: $checks installer entry checks. Evidence retained at $scratch"
