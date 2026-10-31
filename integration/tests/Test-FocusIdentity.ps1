$ErrorActionPreference='Stop'
$root=Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Import-Module (Join-Path $root 'runtime\Remaster.Core.psm1') -Force
$script:passed=0
function Check([bool]$Condition,[string]$Name){if(-not $Condition){throw "FAILED: $Name"};$script:passed++}
function Refuses([scriptblock]$Action,[string]$Name){$caught=$false;try{& $Action}catch{$caught=$true};Check $caught $Name}
$e81='e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023'
$de3='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6'
function Candidate([string]$Profile,[string]$Hash){return [pscustomobject]@{rendering=[pscustomobject]@{profile=$Profile};engine=[pscustomobject]@{output_sha256=$Hash}}}
Check ((Get-RemasterFocusExpectedHash (Candidate 'native-display-v1' $e81)) -ceq $e81.ToUpperInvariant()) 'E81 only explicit adaptive pair'
Check ((Get-RemasterFocusExpectedHash (Candidate 'native-1440-v1' $de3)) -ceq $de3.ToUpperInvariant()) 'legacy DE3 remains accepted'
Check ((Get-RemasterFocusExpectedHash ([pscustomobject]@{})) -ceq $de3.ToUpperInvariant()) 'absent capability keeps original helper pin'
foreach($profile in @('native-1440-v1','unknown','Native-Display-v1')){
 Check ((Get-RemasterFocusExpectedHash (Candidate $profile $e81)) -cne $e81.ToUpperInvariant()) 'E81 cannot leak to wrong capability'
}
foreach($hash in @($de3,('0'*64),'',$e81.ToUpperInvariant())){
 Refuses {Get-RemasterFocusExpectedHash (Candidate 'native-display-v1' $hash)} 'wrong or noncanonical adaptive managed engine rejected'
}
Refuses {Get-RemasterFocusExpectedHash ([pscustomobject]@{rendering=[pscustomobject]@{profile='native-display-v1'}})} 'missing managed engine rejected'
# Execute ONLY the helper initialization before Add-Type/logging/polling. Mock
# file/module/hash access: no actual helper loop, input messages or process open.
$helper=[IO.File]::ReadAllText((Join-Path $root 'runtime\Watch-RemasterFocus.ps1'))
$prefix=$helper.Substring($helper.IndexOf("$"+'ErrorActionPreference='),$helper.IndexOf('Add-Type')-$helper.IndexOf("$"+'ErrorActionPreference='))
function Test-Prefix($Manifest,[string]$Hash,[bool]$HasManifest=$true){
 $script:fixtureManifest=$Manifest;$script:fixtureHash=$Hash;$script:fixtureExists=$HasManifest
 function Test-Path {param($LiteralPath) return $script:fixtureExists}
 function Get-Content {param($LiteralPath,[switch]$Raw) return ($script:fixtureManifest | ConvertTo-Json -Depth 4)}
 function Import-Module {param($Name,[switch]$Force)}
 function Get-FileHash {param($LiteralPath) return [pscustomobject]@{Hash=$script:fixtureHash.ToUpperInvariant()}}
 $GamePath='C:\test-fixture\ds9dw.exe'
 & ([scriptblock]::Create($prefix))
}
Test-Prefix (Candidate 'native-display-v1' $e81) $e81
Check $true 'actual helper preflight accepts adaptiveE81'
Test-Prefix (Candidate 'native-1440-v1' $de3) $de3
Check $true 'actual helper preflight accepts legacyDE3'
Refuses {Test-Prefix (Candidate 'native-1440-v1' $e81) $e81} 'actual helper rejects E81 under fixed1440'
Refuses {Test-Prefix (Candidate 'unknown' $e81) $e81} 'actual helper rejects E81 unknown capability'
Refuses {Test-Prefix (Candidate 'native-display-v1' $e81) $de3} 'actual helper rejects disk/hash mismatch'
Refuses {Test-Prefix (Candidate 'native-display-v1' $de3) $de3} 'actual helper rejects adaptiveDE3'
Test-Prefix $null $de3 $false
Check $true 'actual legacy missing-manifest pin unchanged'
Refuses {Test-Prefix $null $e81 $false} 'unmanaged E81 cannot pass legacy pin'
Write-Host ('PASS: '+$script:passed+' focus identity checks; no game/helper process/input/display action.')
