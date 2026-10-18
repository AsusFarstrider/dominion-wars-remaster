[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
Import-Module (Join-Path $root 'game-process-probe-recovery-v1\DominionWarsLauncher.psm1') -Force
$testRoot=Join-Path $root ('recovery-read-tests\'+[guid]::NewGuid().ToString('N'))
$null=New-Item -ItemType Directory -Path $testRoot
$path=Join-Path $testRoot 'state.json'
Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Text;
using System.Threading;
public class RecoveryReadRaceTestV1 {
    public static Thread Hold(string path, int ms) {
        var ready=new ManualResetEvent(false);
        var thread=new Thread(() => {
            using(var file=File.Open(path,FileMode.Open,FileAccess.ReadWrite,FileShare.None)) {
                ready.Set(); Thread.Sleep(ms);
            }
        });
        thread.IsBackground=true; thread.Start(); ready.WaitOne(); ready.Dispose(); return thread;
    }
    public static Thread Replace(string path) {
        var thread=new Thread(() => {
            for(int i=0;i<200;i++) {
                var temp=path+".writer";
                File.WriteAllText(temp,"{\"Ready\":true,\"Sequence\":"+i+"}",new UTF8Encoding(false));
                File.Replace(temp,path,null);
                Thread.Sleep(1);
            }
        });
        thread.IsBackground=true; thread.Start(); return thread;
    }
}
'@
[IO.File]::WriteAllText($path,'{"Ready":true,"Sequence":0}')
if(-not (Read-DominionWarsRecoveryState $path).Ready){throw 'Ordinary read failed'}
$hold=[RecoveryReadRaceTestV1]::Hold($path,300)
$timer=[Diagnostics.Stopwatch]::StartNew()
$state=Read-DominionWarsRecoveryState $path
$hold.Join()
if(-not $state.Ready -or $timer.ElapsedMilliseconds -lt 100){throw 'Transient sharing retry failed'}
$writer=[RecoveryReadRaceTestV1]::Replace($path)
for($i=0;$i -lt 250;$i++){
    $state=Read-DominionWarsRecoveryState $path
    if(-not $state.Ready -or $null -eq $state.Sequence){throw 'Concurrent replacement read failed'}
}
$writer.Join()
if((Read-DominionWarsRecoveryState $path).Sequence -ne 199){throw 'Final publication missing'}
[IO.File]::WriteAllText($path,'not valid JSON')
$failed=$false
try {$null=Read-DominionWarsRecoveryState $path} catch {$failed=$true}
if(-not $failed){throw 'Malformed JSON was accepted'}
$failed=$false;$timer.Restart()
try {$null=Read-DominionWarsRecoveryState (Join-Path $testRoot 'missing.json')} catch {$failed=$true}
if(-not $failed -or $timer.Elapsed.TotalSeconds -gt 5){throw 'Persistent missing file did not fail within bound'}
'PASS: ordinary read, transient sharing retry, 250 concurrent replacement reads, malformed JSON rejection, bounded missing-file failure. No display or game operations.'
