Set-StrictMode -Version 3.0
$ErrorActionPreference = 'Stop'
$script:InstallLog = $null
$script:InstallStarted = $null

# File I/O and native helpers must keep running even if console output is paused
# by text selection. No PowerShell scriptblocks run on these background threads.
if (-not ('DominionWars.InstallWork' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Diagnostics;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace DominionWars {
    // Per-console state only: no registry/default-console changes. The optional
    // delegates allow deterministic policy tests without changing a real console.
    public sealed class InstallConsoleGuard : IDisposable {
        const uint QuickEdit = 0x0040, Extended = 0x0080;
        readonly Func<uint?> read;
        readonly Func<uint, bool> write;
        bool finished;
        public bool Available { get; private set; }
        public bool Changed { get; private set; }
        public uint OriginalMode { get; private set; }
        public uint AppliedMode { get; private set; }
        public string Status { get; private set; }
        public string RestoreResult { get; private set; }
        [DllImport("kernel32.dll", SetLastError = true)] static extern IntPtr GetStdHandle(int id);
        [DllImport("kernel32.dll", SetLastError = true)] static extern bool GetConsoleMode(IntPtr handle, out uint mode);
        [DllImport("kernel32.dll", SetLastError = true)] static extern bool SetConsoleMode(IntPtr handle, uint mode);
        static uint? ReadNative() {
            uint mode;
            return GetConsoleMode(GetStdHandle(-10), out mode) ? (uint?)mode : null;
        }
        static bool WriteNative(uint mode) { return SetConsoleMode(GetStdHandle(-10), mode); }
        public InstallConsoleGuard() : this(ReadNative, WriteNative) { }
        public InstallConsoleGuard(Func<uint?> reader, Func<uint, bool> writer) {
            read = reader; write = writer;
            uint? original = read();
            if (!original.HasValue) { Status = "No console input handle; QuickEdit safeguard not applicable."; return; }
            Available = true;
            OriginalMode = original.Value;
            AppliedMode = (OriginalMode | Extended) & ~QuickEdit;
            if ((OriginalMode & QuickEdit) == 0) { Status = "Console QuickEdit already disabled; no change."; return; }
            if (!write(AppliedMode)) { Status = "Could not disable console QuickEdit. If selected text pauses output, press Esc."; return; }
            Changed = true;
            Status = "Console QuickEdit temporarily disabled; original=0x" + OriginalMode.ToString("X") + "; applied=0x" + AppliedMode.ToString("X") + ". Ctrl+C input handling unchanged.";
        }
        public string Restore() {
            if (finished) return RestoreResult;
            finished = true;
            if (!Changed) return RestoreResult = "Console mode unchanged; no restoration required.";
            uint? current = read();
            if (!current.HasValue) return RestoreResult = "Console input no longer available; restoration skipped.";
            if (current.Value != AppliedMode) return RestoreResult = "Preserving later console-mode changes; original mode not reapplied.";
            // Extended is required when changing the QuickEdit bit. Restore all
            // original bits afterward if Extended was originally absent.
            if (!write(OriginalMode | Extended)) return RestoreResult = "Console-mode restoration failed.";
            if ((OriginalMode & Extended) == 0 && !write(OriginalMode)) return RestoreResult = "QuickEdit restored, but original Extended flag could not be restored.";
            uint? restored = read();
            return RestoreResult = restored.HasValue && restored.Value == OriginalMode
                ? "Original console input mode restored."
                : "Console host did not report the exact original mode after restoration.";
        }
        public void Dispose() { Restore(); }
    }

    public sealed class InstallWork : IDisposable {
        readonly CancellationTokenSource cancellation = new CancellationTokenSource();
        readonly object gate = new object();
        long bytes, total;
        string current = "Starting";
        Process process;
        public Task Task { get; private set; }
        public string Hash { get; private set; }
        public int ExitCode { get; private set; }
        public int ProcessId { get; private set; }
        public long Bytes { get { return Interlocked.Read(ref bytes); } }
        public long Total { get { return Interlocked.Read(ref total); } }
        public string Current { get { lock (gate) { return current; } } }
        void SetCurrent(string value) { lock (gate) { current = value; } }
        void Check() { cancellation.Token.ThrowIfCancellationRequested(); }

        public static InstallWork HashFile(string path) {
            var work = new InstallWork();
            work.Task = System.Threading.Tasks.Task.Run(() => {
                work.SetCurrent(Path.GetFileName(path));
                using (var input = File.OpenRead(path))
                using (var hash = SHA256.Create()) {
                    Interlocked.Exchange(ref work.total, input.Length);
                    var buffer = new byte[1024 * 1024];
                    int count;
                    while (true) {
                        work.Check();
                        count = input.Read(buffer, 0, buffer.Length);
                        if (count == 0) break;
                        hash.TransformBlock(buffer, 0, count, buffer, 0);
                        Interlocked.Add(ref work.bytes, count);
                    }
                    work.Check();
                    hash.TransformFinalBlock(new byte[0], 0, 0);
                    work.Hash = BitConverter.ToString(hash.Hash).Replace("-", "");
                }
            });
            return work;
        }

        public static InstallWork CopyTree(string source, string destination) {
            var work = new InstallWork();
            work.Task = System.Threading.Tasks.Task.Run(() => {
                var files = new List<string>();
                var pending = new Queue<string>();
                string root = Path.GetFullPath(source).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
                pending.Enqueue(root);
                work.SetCurrent("Counting files");
                while (pending.Count != 0) {
                    work.Check();
                    string directory = pending.Dequeue();
                    if ((File.GetAttributes(directory) & FileAttributes.ReparsePoint) != 0)
                        throw new IOException("Copy source contains a junction or symbolic link: " + directory);
                    string relative = directory.Substring(root.Length);
                    Directory.CreateDirectory(Path.Combine(destination, relative));
                    foreach (string child in Directory.GetDirectories(directory)) pending.Enqueue(child);
                    foreach (string file in Directory.GetFiles(directory)) {
                        if ((File.GetAttributes(file) & FileAttributes.ReparsePoint) != 0)
                            throw new IOException("Copy source contains a symbolic link: " + file);
                        files.Add(file);
                        Interlocked.Add(ref work.total, new FileInfo(file).Length);
                    }
                }
                var buffer = new byte[1024 * 1024];
                foreach (string file in files) {
                    work.Check();
                    string relative = file.Substring(root.Length);
                    work.SetCurrent(relative);
                    string target = Path.Combine(destination, relative);
                    using (var input = File.OpenRead(file))
                    using (var output = new FileStream(target, FileMode.CreateNew, FileAccess.Write, FileShare.Read)) {
                        int count;
                        while (true) {
                            work.Check();
                            count = input.Read(buffer, 0, buffer.Length);
                            if (count == 0) break;
                            output.Write(buffer, 0, count);
                            Interlocked.Add(ref work.bytes, count);
                        }
                        output.Flush();
                    }
                    File.SetLastWriteTimeUtc(target, File.GetLastWriteTimeUtc(file));
                }
                work.Check();
            });
            return work;
        }

        public static InstallWork RunProcess(string executable, string arguments, string stdout, string stderr, string output) {
            var work = new InstallWork();
            var info = new ProcessStartInfo(executable, arguments) {
                WorkingDirectory = Path.GetDirectoryName(Path.GetFullPath(executable)),
                UseShellExecute = false, CreateNoWindow = true,
                RedirectStandardOutput = true, RedirectStandardError = true,
                RedirectStandardInput = true
            };
            var outFile = new FileStream(stdout, FileMode.CreateNew, FileAccess.Write, FileShare.ReadWrite);
            FileStream errFile;
            try { errFile = new FileStream(stderr, FileMode.CreateNew, FileAccess.Write, FileShare.ReadWrite); }
            catch { outFile.Dispose(); throw; }
            var child = new Process { StartInfo = info };
            try {
                if (!child.Start()) throw new IOException("Could not start extraction helper.");
                work.process = child;
                work.ProcessId = child.Id;
                child.StandardInput.Close();
            }
            catch {
                if (work.process != null) {
                    work.KillChild();
                    child.WaitForExit();
                }
                outFile.Dispose(); errFile.Dispose(); child.Dispose(); throw;
            }
            work.SetCurrent("Waiting for native helper output or exit");
            work.Task = System.Threading.Tasks.Task.Run(() => {
                using (outFile)
                using (errFile) {
                    var outTask = child.StandardOutput.BaseStream.CopyToAsync(outFile);
                    var errTask = child.StandardError.BaseStream.CopyToAsync(errFile);
                    // A failed log write must not leave a child blocked on a full pipe.
                    outTask.ContinueWith(t => work.KillChild(), TaskContinuationOptions.OnlyOnFaulted);
                    errTask.ContinueWith(t => work.KillChild(), TaskContinuationOptions.OnlyOnFaulted);
                    while (!child.WaitForExit(200)) work.MeasureOutput(output, stdout, stderr);
                    System.Threading.Tasks.Task.WaitAll(outTask, errTask);
                    outFile.Flush(); errFile.Flush();
                    work.MeasureOutput(output, stdout, stderr);
                    work.ExitCode = child.ExitCode;
                    work.Check();
                }
            });
            return work;
        }

        void MeasureOutput(string output, string stdout, string stderr) {
            long size = 0;
            foreach (string path in new [] { output, stdout, stderr }) {
                if (String.IsNullOrEmpty(path)) continue;
                try { if (File.Exists(path)) size += new FileInfo(path).Length; }
                catch (IOException) { }
            }
            Interlocked.Exchange(ref bytes, size);
        }
        void KillChild() {
            lock (gate) {
                if (process != null && !process.HasExited) {
                    try { process.Kill(); }
                    catch (InvalidOperationException) { if (!process.HasExited) throw; }
                }
            }
        }
        public void Cancel() { cancellation.Cancel(); KillChild(); }
        public void Dispose() {
            // Do not delete staging while a worker could still be writing to it.
            // An outstanding optical-drive read may delay cancellation until Windows returns.
            Exception cancelError = null;
            try { if (Task != null && !Task.IsCompleted) Cancel(); }
            catch (Exception error) { cancelError = error; }
            try {
                if (Task != null) Task.GetAwaiter().GetResult();
                if (cancelError != null) throw cancelError;
            }
            finally {
                if (process != null) process.Dispose();
                cancellation.Dispose();
            }
        }
    }
}
'@
}

function Start-DWInstallLog {
    param([string]$Action, [string]$Root = (Join-Path $env:LOCALAPPDATA 'Dominion Wars Modernizer\InstallLogs'))
    New-Item -ItemType Directory -Path $Root -Force | Out-Null
    $script:InstallStarted = [DateTime]::UtcNow
    $script:InstallLog = Join-Path $Root ($Action + '-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N') + '.log')
    [IO.File]::WriteAllText($script:InstallLog, '')
    Write-DWInstallEvent "$Action started. Installation log: $script:InstallLog"
    return $script:InstallLog
}

function Write-DWInstallEvent {
    param([Parameter(Mandatory = $true)][string]$Message)
    $line = '[{0}] {1}' -f [DateTime]::UtcNow.ToString('o'), $Message
    if ($script:InstallLog) { [IO.File]::AppendAllText($script:InstallLog, $line + [Environment]::NewLine) }
    Write-Host $Message
}

function Get-DWInstallLogPath { return $script:InstallLog }

function Start-DWInstallConsoleGuard { return New-Object DominionWars.InstallConsoleGuard }

function Stop-DWInstallConsoleGuard {
    param($Guard)
    if ($null -eq $Guard) { return }
    $result = $Guard.Restore()
    # Do not write to the console after re-enabling selection: a fresh click
    # must not block completion. A logging failure must not mask the real result.
    if ($script:InstallLog) {
        try { [IO.File]::AppendAllText($script:InstallLog, ('[{0}] {1}{2}' -f [DateTime]::UtcNow.ToString('o'), $result, [Environment]::NewLine)) }
        catch { }
    }
}

function Wait-DWInstallWork {
    param(
        [Parameter(Mandatory = $true)]$Work,
        [Parameter(Mandatory = $true)][string]$Label,
        [Threading.CancellationToken]$CancellationToken = [Threading.CancellationToken]::None,
        [int]$ReportSeconds = 5,
        [int]$StallSeconds = 60
    )
    $started = [DateTime]::UtcNow; $lastReport = $started; $lastChange = $started
    $lastBytes = $Work.Bytes
    try {
        Write-DWInstallEvent "$Label started."
        while (-not $Work.Task.IsCompleted) {
            $CancellationToken.ThrowIfCancellationRequested()
            [Threading.Thread]::Sleep(200)
            $now = [DateTime]::UtcNow
            $bytes = $Work.Bytes
            if ($bytes -ne $lastBytes) { $lastChange = $now; $lastBytes = $bytes }
            if (($now - $lastReport).TotalSeconds -ge $ReportSeconds) {
                $elapsed = [int]($now - $started).TotalSeconds
                $idle = [int]($now - $lastChange).TotalSeconds
                $detail = if ($Work.Total -gt 0) {
                    '{0:N1}/{1:N1} MiB ({2:0}%)' -f ($bytes / 1MB), ($Work.Total / 1MB), (100.0 * $bytes / $Work.Total)
                } else { '{0:N0} bytes of output/diagnostics observed' -f $bytes }
                $message = "$Label - $($Work.Current); $detail; elapsed ${elapsed}s."
                if ($idle -ge $StallSeconds) {
                    $message += " No measurable byte progress for ${idle}s; a slow/read-retrying disc or stalled helper is possible. No automatic timeout."
                }
                Write-DWInstallEvent $message
                $lastReport = $now
            }
        }
        $CancellationToken.ThrowIfCancellationRequested()
        $Work.Task.GetAwaiter().GetResult()
        Write-DWInstallEvent ("$Label completed in {0:N1}s; {1:N0} bytes processed/observed." -f ([DateTime]::UtcNow - $started).TotalSeconds, $Work.Bytes)
    }
    finally {
        # Ctrl+C/pipeline cancellation also takes this path. Kill only our own
        # native helper and wait for worker I/O to finish before staging cleanup.
        try { $Work.Dispose() }
        catch { Write-DWInstallEvent "$Label worker ended with: $($_.Exception.GetBaseException().Message)" }
    }
}

function Get-DWInstallFileHash {
    param([string]$Path, [string]$Label)
    $work = [DominionWars.InstallWork]::HashFile($Path)
    Wait-DWInstallWork -Work $work -Label $Label
    return $work.Hash
}

function Copy-DWInstallTree {
    param([string]$Source, [string]$Destination, [string]$Label = 'Copying game files')
    $work = [DominionWars.InstallWork]::CopyTree($Source, $Destination)
    Wait-DWInstallWork -Work $work -Label $Label
}

function ConvertTo-DWNativeArgument {
    param([AllowEmptyString()][string]$Value)
    # Windows CommandLineToArgvW/CRT quoting, including trailing backslashes.
    return '"' + (($Value -replace '(\\*)"', '$1$1\"') -replace '(\\+)$', '$1$1') + '"'
}

function Invoke-DWInstallHelper {
    param(
        [string]$Executable, [string[]]$Arguments, [string]$ExpectedOutput,
        [string]$Label, [string]$LogRoot,
        [Threading.CancellationToken]$CancellationToken = [Threading.CancellationToken]::None,
        [int]$ReportSeconds = 5, [int]$StallSeconds = 60
    )
    if (-not $LogRoot) {
        if (-not $script:InstallLog) { throw 'Start the installation log before launching a helper.' }
        $LogRoot = Split-Path -Parent $script:InstallLog
    }
    if ($ExpectedOutput -and (Test-Path -LiteralPath $ExpectedOutput)) { throw "Extraction output already exists: '$ExpectedOutput'." }
    $prefix = Join-Path $LogRoot ('extract-' + [guid]::NewGuid().ToString('N'))
    Write-DWInstallEvent "$Label diagnostics: $prefix.stdout.log and $prefix.stderr.log"
    $quoted = ($Arguments | ForEach-Object { ConvertTo-DWNativeArgument $_ }) -join ' '
    $work = [DominionWars.InstallWork]::RunProcess($Executable, $quoted, "$prefix.stdout.log", "$prefix.stderr.log", $ExpectedOutput)
    Wait-DWInstallWork -Work $work -Label "$Label (isolated helper PID=$($work.ProcessId))" -CancellationToken $CancellationToken -ReportSeconds $ReportSeconds -StallSeconds $StallSeconds
    if ($work.ExitCode -ne 0) { throw "$Label failed (exit code $($work.ExitCode)). See $prefix.stderr.log" }
    if ($ExpectedOutput -and (-not (Test-Path -LiteralPath $ExpectedOutput -PathType Leaf) -or
        (Get-Item -LiteralPath $ExpectedOutput).Length -eq 0)) {
        throw "$Label returned success without a nonempty output file. See $prefix.stderr.log"
    }
}

Export-ModuleMember -Function *-DW*
