param([Parameter(Mandatory=$true)][string]$GamePath,[Parameter(Mandatory=$true)][string]$LogPath)
$ErrorActionPreference='Stop'
$expected='DE3E3EDE7A9B4CE65A891201547F36413F3A880E4DF3F28FB8A81E51F687ABC6'
$gameFull=[IO.Path]::GetFullPath($GamePath)
$gameRoot=Split-Path -Parent $gameFull
$manifestPath=Join-Path $gameRoot '.dw-remaster\manifest.json'
if(Test-Path -LiteralPath $manifestPath){
 $focusManifest=Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
 if($focusManifest.PSObject.Properties['rendering'] -and $focusManifest.rendering.profile -ceq 'native-display-v1'){
  Import-Module (Join-Path $gameRoot 'Remaster.Core.psm1') -Force
  $expected=Get-RemasterFocusExpectedHash $focusManifest
 }
}
if((Get-FileHash -LiteralPath $gameFull).Hash -ne $expected){throw 'Unexpected game image'}
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class DwAltReleaseProbe {
 [DllImport("kernel32.dll",SetLastError=true)] public static extern IntPtr OpenProcess(uint access,bool inherit,int pid);
 [DllImport("kernel32.dll",SetLastError=true)] public static extern bool ReadProcessMemory(IntPtr h,IntPtr address,byte[] bytes,UIntPtr size,out UIntPtr read);
 [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr h);
 [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll")] public static extern short GetAsyncKeyState(int key);
 [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr h,int command);
 [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
 [DllImport("user32.dll",SetLastError=true)] public static extern bool PostMessage(IntPtr h,uint msg,IntPtr wp,IntPtr lp);
}
'@
function Write-AltLog([string]$Message){Add-Content -LiteralPath $LogPath -Value ([DateTime]::UtcNow.ToString('o')+' '+$Message)}

function Repair-StaleWinKey([IntPtr]$Window,[int]$GamePid){
 # This game uses shared bit 0x80 for VK_LWIN and VK_RWIN.
 if(([DwAltReleaseProbe]::GetAsyncKeyState(0x5b) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0x5c) -band 0x8000)){return}
 [uint32]$foregroundPid=0
 $null=[DwAltReleaseProbe]::GetWindowThreadProcessId([DwAltReleaseProbe]::GetForegroundWindow(),[ref]$foregroundPid)
 if($foregroundPid -ne $GamePid){return}
 $handle=[DwAltReleaseProbe]::OpenProcess(0x1010,$false,$GamePid)
 if($handle -eq [IntPtr]::Zero){Write-AltLog 'Win-state read-only open failed; no repair';return}
 try{
  $bytes=New-Object byte[] 4;$read=[UIntPtr]::Zero;$size=New-Object UIntPtr ([uint32]4)
  if(-not [DwAltReleaseProbe]::ReadProcessMemory($handle,[IntPtr]0x719618,$bytes,$size,[ref]$read) -or $read.ToUInt64() -ne 4){Write-AltLog 'Win-state read failed; no repair';return}
  $mask=[BitConverter]::ToUInt32($bytes,0)
  Write-AltLog ('Return modifier mask=0x{0:X}; physical Windows keys up' -f $mask)
  if(-not ($mask -band 0x80)){return}
  # Recheck immediately before posting, never send global input.
  [uint32]$checkPid=0
  $null=[DwAltReleaseProbe]::GetWindowThreadProcessId([DwAltReleaseProbe]::GetForegroundWindow(),[ref]$checkPid)
  if($checkPid -ne $GamePid -or ([DwAltReleaseProbe]::GetAsyncKeyState(0x5b) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0x5c) -band 0x8000)){return}
  if((Get-FileHash -LiteralPath $gameFull).Hash -ne $expected){throw 'Executable identity changed'}
  $left=[DwAltReleaseProbe]::PostMessage($Window,0x101,[IntPtr]0x5b,[IntPtr](-1051000831))
  $right=[DwAltReleaseProbe]::PostMessage($Window,0x101,[IntPtr]0x5c,[IntPtr](-1050935295))
  Write-AltLog ('Stale Win bit observed; posted game-only LWin/RWin releases: '+$left+','+$right)
 }finally{$null=[DwAltReleaseProbe]::CloseHandle($handle)}
}

Write-AltLog 'Ready; waiting for exact latest research game; no input sent on initial focus.'
$gameProcess=$null;$deadline=[DateTime]::UtcNow.AddSeconds(60)
while(-not $gameProcess -and [DateTime]::UtcNow -lt $deadline){
 foreach($candidate in @(Get-Process -Name ds9dw -ErrorAction SilentlyContinue)){
  if($candidate.Path -and [string]::Equals($candidate.Path,$gameFull,[StringComparison]::OrdinalIgnoreCase)){$gameProcess=$candidate;break}
 }
 if(-not $gameProcess){Start-Sleep -Milliseconds 100}
}
if(-not $gameProcess){throw 'Exact game did not start within 60 seconds'}
$seenForeground=$false;$lostFocus=$false;$releasedTicks=0;$actions=0
$lossAt=[DateTime]::MinValue;$minimizedThisCycle=$false
Write-AltLog ('Watching PID='+$gameProcess.Id)
while(-not $gameProcess.HasExited){
 $foreground=[DwAltReleaseProbe]::GetForegroundWindow();[uint32]$foregroundPid=0
 if($foreground -ne [IntPtr]::Zero){$null=[DwAltReleaseProbe]::GetWindowThreadProcessId($foreground,[ref]$foregroundPid)}
 if($foregroundPid -eq $gameProcess.Id){
  if($seenForeground -and $lostFocus){
   $altDown=([DwAltReleaseProbe]::GetAsyncKeyState(0x12) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0xa4) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0xa5) -band 0x8000)
   if($altDown){$releasedTicks=0}else{$releasedTicks++}
   if($releasedTicks -ge 2){
    $gameProcess.Refresh();$hwnd=$gameProcess.MainWindowHandle;[uint32]$windowPid=0
    if($hwnd -eq [IntPtr]::Zero){throw 'No main game window'}
    $null=[DwAltReleaseProbe]::GetWindowThreadProcessId($hwnd,[ref]$windowPid)
    if($windowPid -ne $gameProcess.Id -or (Get-FileHash -LiteralPath $gameFull).Hash -ne $expected){throw 'Window or file identity changed'}
    [uint32]$checkPid=0
    $null=[DwAltReleaseProbe]::GetWindowThreadProcessId([DwAltReleaseProbe]::GetForegroundWindow(),[ref]$checkPid)
    if($checkPid -ne $gameProcess.Id -or ([DwAltReleaseProbe]::GetAsyncKeyState(0x12) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0xa4) -band 0x8000) -or ([DwAltReleaseProbe]::GetAsyncKeyState(0xa5) -band 0x8000)){$releasedTicks=0;continue}
    # WM_SYSKEYUP/VK_MENU, scan 0x38, previous/transition bits, no Alt-context bit.
    # Local window message only: no SendInput, global hook or physical key change.
    if(-not [DwAltReleaseProbe]::PostMessage($hwnd,0x105,[IntPtr]0x12,[IntPtr](-1070071807))){throw 'Alt-release PostMessage failed'}
    $actions++;Write-AltLog ('Posted game-only Alt release after focus return; count='+$actions)
    Repair-StaleWinKey -Window $hwnd -GamePid $gameProcess.Id
    $lostFocus=$false;$releasedTicks=0
    if($actions -ge 32){Write-AltLog 'Bounded action cap reached; helper stopping';break}
   }
  }
  $seenForeground=$true
  $lossAt=[DateTime]::MinValue;$minimizedThisCycle=$false
 }elseif($seenForeground){
  $lostFocus=$true;$releasedTicks=0
  if($lossAt -eq [DateTime]::MinValue){$lossAt=[DateTime]::UtcNow}
  if(-not $minimizedThisCycle -and $foreground -ne [IntPtr]::Zero -and ([DateTime]::UtcNow-$lossAt).TotalMilliseconds -ge 450){
   $altDown=([DwAltReleaseProbe]::GetAsyncKeyState(0x12) -band 0x8000)
   $foregroundProcess=Get-Process -Id $foregroundPid -ErrorAction SilentlyContinue
   $captureHost=$foregroundProcess -and $foregroundProcess.ProcessName -in @('ScreenClippingHost','SnippingTool')
   if(-not $altDown -and -not $captureHost -and $foregroundProcess){
    $gameProcess.Refresh();$hwnd=$gameProcess.MainWindowHandle;[uint32]$windowPid=0
    $null=[DwAltReleaseProbe]::GetWindowThreadProcessId($hwnd,[ref]$windowPid)
    [uint32]$checkPid=0
    $null=[DwAltReleaseProbe]::GetWindowThreadProcessId([DwAltReleaseProbe]::GetForegroundWindow(),[ref]$checkPid)
    if($hwnd -ne [IntPtr]::Zero -and $windowPid -eq $gameProcess.Id -and $checkPid -eq $foregroundPid -and $checkPid -ne $gameProcess.Id -and (Get-FileHash -LiteralPath $gameFull).Hash -eq $expected){
     if(-not [DwAltReleaseProbe]::IsIconic($hwnd)){
      $posted=[DwAltReleaseProbe]::ShowWindowAsync($hwnd,7)
      Write-AltLog ('Requested game-only minimization; queued='+$posted+'; foreground='+$foregroundProcess.ProcessName+'; no display API used')
     }
     $minimizedThisCycle=$true
    }
   }
  }
 }
 Start-Sleep -Milliseconds 100
}
Write-AltLog 'Helper completed.'
