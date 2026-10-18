[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 3.0
$root = Split-Path -Parent $PSScriptRoot
$scratch = Join-Path $root ('.test-installer-activity-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $scratch | Out-Null
Import-Module (Join-Path $root 'payload\DominionWarsInstall.psm1') -Force
$log = Start-DWInstallLog -Action Test -Root $scratch
$script:Checks = 0
function Assert([bool]$Condition, [string]$Message) {
    if (-not $Condition) { throw "FAIL: $Message" }
    $script:Checks++; Write-Host "PASS: $Message"
}
function Reject([scriptblock]$Operation, [string]$Message) {
    $rejected = $false
    try { & $Operation } catch { $rejected = $true }
    Assert $rejected $Message
}

$helper = Join-Path $scratch 'Test helper with spaces.exe'
Add-Type -OutputAssembly $helper -OutputType ConsoleApplication -TypeDefinition @'
using System;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
public class InstallerTestHelper {
    [DllImport("kernel32.dll")] static extern IntPtr GetConsoleWindow();
    public static int Main(string[] args) {
        string mode = args[0], output = args[1];
        if (mode == "fail") { Console.Error.Write("intentional fixture error"); return 23; }
        if (mode == "missing") return 0;
        if (mode == "empty") { File.WriteAllBytes(output, new byte[0]); return 0; }
        if (mode == "hang") { while (true) Thread.Sleep(100); }
        if (mode == "console") {
            File.WriteAllText(output, GetConsoleWindow().ToInt64().ToString() + ":" + Console.In.ReadToEnd().Length);
            return 0;
        }
        if (mode == "quiet") Thread.Sleep(2400);
        if (mode == "flood") {
            Console.Out.Write(new string('O', 2 * 1024 * 1024));
            Console.Error.Write(new string('E', 2 * 1024 * 1024));
        }
        if (mode == "args") {
            string[] result = new string[args.Length - 2];
            Array.Copy(args, 2, result, 0, result.Length);
            File.WriteAllLines(output, result);
        } else File.WriteAllText(output, "fixture output");
        return 0;
    }
}
'@

$source = Join-Path $scratch 'Source with spaces'
$target = Join-Path $scratch 'Copied tree'
New-Item -ItemType Directory -Path (Join-Path $source 'Empty directory'), (Join-Path $source 'Nested') -Force | Out-Null
[IO.File]::WriteAllBytes((Join-Path $source 'Nested\Data.dat'), ([byte[]](0..255) * 8192))
[IO.File]::WriteAllText((Join-Path $source '.hidden-fixture'), 'also copied')
[IO.File]::WriteAllBytes((Join-Path $source 'empty.dat'), [byte[]]@())
$hash = Get-DWInstallFileHash -Path (Join-Path $source 'Nested\Data.dat') -Label 'Fixture hash'
Assert ($hash -eq (Get-FileHash -LiteralPath (Join-Path $source 'Nested\Data.dat')).Hash) 'Streaming SHA-256 matches standard hashing'
Copy-DWInstallTree -Source $source -Destination $target
Assert ((Get-FileHash -LiteralPath (Join-Path $target 'Nested\Data.dat')).Hash -eq $hash) 'Tree copying preserves exact bytes and nested paths'
Assert (Test-Path -LiteralPath (Join-Path $target 'Empty directory') -PathType Container) 'Empty directories are retained'
Assert ((Get-Item -LiteralPath (Join-Path $target 'empty.dat')).Length -eq 0) 'Empty files are retained'
Assert (Test-Path -LiteralPath (Join-Path $target '.hidden-fixture')) 'Dot-prefixed files are retained'
Reject { Copy-DWInstallTree -Source $source -Destination $target } 'Copy never overwrites an existing destination file'
Reject { Get-DWInstallFileHash -Path (Join-Path $scratch 'missing') -Label 'Missing fixture' } 'Hash read failures propagate'

$out = Join-Path $scratch 'extracted file.dat'
Invoke-DWInstallHelper -Executable $helper -Arguments @('flood', $out) -ExpectedOutput $out -Label 'Pipe flood' -LogRoot $scratch
Assert (Test-Path -LiteralPath $out) 'A helper writing more than pipe capacity on both streams completes'
$stdout = @(Get-ChildItem -LiteralPath $scratch -Filter '*.stdout.log')
$stderr = @(Get-ChildItem -LiteralPath $scratch -Filter '*.stderr.log')
Assert ($stdout[0].Length -eq 2MB -and $stderr[0].Length -eq 2MB) 'Both helper streams are retained in full'
Reject { Invoke-DWInstallHelper -Executable $helper -Arguments @('quiet', $out) -ExpectedOutput $out -Label 'Stale output' -LogRoot $scratch } 'Existing output cannot masquerade as successful extraction'

$outArgs = Join-Path $scratch 'quoted arguments.txt'
$arguments = @('space in value', '', 'C:\trailing slash\', 'quote"inside', 'slashes\\"quoted')
Invoke-DWInstallHelper -Executable $helper -Arguments (@('args', $outArgs) + $arguments) -ExpectedOutput $outArgs -Label 'Quoted arguments' -LogRoot $scratch
$received = [IO.File]::ReadAllLines($outArgs)
Assert (($received | ConvertTo-Json -Compress) -ceq ($arguments | ConvertTo-Json -Compress)) 'Windows argument quoting preserves spaces, empty strings, quotes and trailing slashes'

$consoleOutput = Join-Path $scratch 'console.dat'
Invoke-DWInstallHelper -Executable $helper -Arguments @('console', $consoleOutput) -ExpectedOutput $consoleOutput -Label 'Console isolation' -LogRoot $scratch
Assert ((Get-Content -LiteralPath $consoleOutput -Raw) -eq '0:0') 'Helper has no console window and standard input is closed'

foreach ($mode in @('fail', 'missing', 'empty')) {
    $failedOutput = Join-Path $scratch ($mode + '.dat')
    Reject { Invoke-DWInstallHelper -Executable $helper -Arguments @($mode, $failedOutput) -ExpectedOutput $failedOutput -Label $mode -LogRoot $scratch } "Reject helper result: $mode"
}
$capturedError = @(Get-ChildItem -LiteralPath $scratch -Filter '*.stderr.log' | Where-Object { (Get-Content -LiteralPath $_.FullName -Raw) -eq 'intentional fixture error' })
Assert ($capturedError.Count -eq 1) 'Native error details survive a helper failure'

$quietOutput = Join-Path $scratch 'quiet.dat'
Invoke-DWInstallHelper -Executable $helper -Arguments @('quiet', $quietOutput) -ExpectedOutput $quietOutput -Label 'Quiet fixture' -LogRoot $scratch -ReportSeconds 1 -StallSeconds 1
$text = Get-Content -LiteralPath $log -Raw
Assert ($text -match 'No measurable byte progress' -and $text -match 'No automatic timeout') 'Quiet operations report uncertainty rather than invented progress or automatic failure'
Assert (Test-Path -LiteralPath $quietOutput) 'A quiet helper is allowed to finish normally'

# Cancellation is tested through the same wait/finally path used on Ctrl+C.
# A separately started helper proves cancellation is scoped to the owned child.
$unrelated = Start-Process -FilePath $helper -ArgumentList @('hang', 'unused') -PassThru
try {
    $cancel = New-Object Threading.CancellationTokenSource
    try {
        $cancel.CancelAfter(600)
        Reject { Invoke-DWInstallHelper -Executable $helper -Arguments @('hang', 'unused') -Label 'Cancelled helper fixture' -LogRoot $scratch -CancellationToken $cancel.Token } 'Cancellation aborts a stalled helper'
    } finally { $cancel.Dispose() }
    $text = Get-Content -LiteralPath $log -Raw
    $ownedId = [int]([regex]::Matches($text, 'isolated helper PID=(\d+)') | Select-Object -Last 1).Groups[1].Value
    Assert ($null -eq (Get-Process -Id $ownedId -ErrorAction SilentlyContinue)) 'Owned helper has exited before cancellation returns'
    $unrelated.Refresh()
    Assert (-not $unrelated.HasExited) 'Cancellation does not kill an unrelated helper'
} finally {
    if (-not $unrelated.HasExited) { $unrelated.Kill(); $unrelated.WaitForExit() }
    $unrelated.Dispose()
}
Assert (Test-Path -LiteralPath $log) 'Operation logs persist outside temporary game staging'
Write-Host "PASS: $script:Checks installer activity checks. Evidence retained at $scratch"
