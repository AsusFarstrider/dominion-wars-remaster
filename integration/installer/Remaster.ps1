[CmdletBinding()]
param(
    [ValidateSet('Install','Verify','Repair','Remove','Recover')][string]$Action='Install',
    [string]$SourcePath,[string]$MediaPath,[string]$PatchPath,[string]$Destination,
    [string]$RecoveryPath,[string]$CompletionMarker,[switch]$NonInteractive,[switch]$NoDesktopShortcut,
    [ValidateSet('Auto','D3D9On12','Native')][string]$Backend='Auto'
)
$ErrorActionPreference='Stop'
if(-not $Destination){$Destination=Join-Path $env:LOCALAPPDATA 'Dominion Wars Remaster'}
$packageRoot=Split-Path -Parent $MyInvocation.MyCommand.Path
Import-Module (Join-Path $packageRoot 'Remaster.Core.psm1') -Force
$guard=$null;$stage=$null;$exitCode=1;$mutex=$null;$owns=$false

function Install-RemasterShortcut([string]$Root,[string]$BuildId,$Identity=$null) {
    $desktop=[Environment]::GetFolderPath('Desktop')
    $shell=New-Object -ComObject WScript.Shell
    $name="Dominion Wars Remaster ($BuildId)";$index=0
    if($Identity -and $Identity.remaster_version){$name='Dominion Wars Remastered '+$Identity.remaster_version+' ('+$Identity.release_stage+')'}
    do {
        $suffix=if($index){" ($index)"}else{''}
        $path=Join-Path $desktop ($name+$suffix+'.lnk')
        if(Test-Path -LiteralPath $path) {
            $link=$shell.CreateShortcut($path)
            if($link.TargetPath -eq (Join-Path $Root 'Play Dominion Wars Remaster.cmd')){return $path}
        } else {break}
        $index++
        if($index -gt 100){throw 'Too many conflicting shortcut names; use NoDesktopShortcut'}
    } while($true)
    $link=$shell.CreateShortcut($path)
    $link.TargetPath=Join-Path $Root 'Play Dominion Wars Remaster.cmd'
    $link.WorkingDirectory=$Root;$link.IconLocation=(Join-Path $Root 'ds9dw.exe')+',0'
    $link.Description="$name; build $BuildId; official base 1.04"
    $link.Save();return $path
}

try {
    $manifest=Assert-RemasterPackage $packageRoot
    $destinationFull=Assert-RemasterPlainPath $Destination
    if($CompletionMarker -and (Test-Path -LiteralPath $CompletionMarker)){throw 'Completion marker must be new'}
    if($Action -eq 'Verify') {
        $state=Test-RemasterInstalled $destinationFull
        Write-Host "Verified installed build $($state.build_id).";$exitCode=0
    } else {
        Assert-RemasterIdle
        $mutex=New-Object Threading.Mutex($false,'Local\DominionWarsRemaster.Install')
        try {$owns=$mutex.WaitOne(0)}catch [Threading.AbandonedMutexException]{$owns=$true}
        if(-not $owns){throw 'Another remaster installation operation is active'}
        if($Action -eq 'Recover') {
            if(-not $RecoveryPath){throw 'Supply the retained transaction JSON using RecoveryPath'}
            $journalPath=[IO.Path]::GetFullPath($RecoveryPath)
            $parent=Split-Path -Parent $destinationFull
            if((Split-Path -Parent $journalPath) -ne $parent -or (Split-Path -Leaf $journalPath) -notmatch '^\.dw-remaster-transaction-[a-f0-9]{32}\.json$'){throw 'Recovery journal must be the generated sibling transaction file'}
            $journal=Read-RemasterJson $journalPath
            if($journal.destination -ne $destinationFull -or $journal.previous -ne (Join-Path $parent ('.dw-remaster-previous-'+$journal.id))){throw 'Journal target identity mismatch'}
            if(Test-Path -LiteralPath $destinationFull){throw 'Destination exists; preserve it. Verify or repair instead of automatic recovery.'}
            $null=Assert-RemasterPlainPath $journal.previous
            $null=Test-RemasterInstalled $journal.previous
            Move-Item -LiteralPath $journal.previous -Destination $destinationFull
            Write-Host 'Previous remaster restored. Failed staging data was retained.';$exitCode=0
        } else {
            if($Action -eq 'Install' -and (Test-Path -LiteralPath $destinationFull)){throw 'Destination exists. Use a new directory, or Repair an owned remaster. Nothing overwritten.'}
            if($Action -in @('Repair','Remove')) {
                Assert-RemasterTree $destinationFull
                $oldState=Read-RemasterJson (Join-Path $destinationFull '.dw-remaster\installed.json')
                if($oldState.schema -ne 1){throw 'Destination is not an owned remaster installation'}
                if($SourcePath -or $MediaPath){throw 'Repair/removal rebuild from verified local base backups; use a fresh Install for another source'}
                $SourcePath=$destinationFull
                if(-not $PSBoundParameters.ContainsKey('Backend')) {
                    $Backend=if($oldState.profile -eq 'native'){'Native'}else{'D3D9On12'}
                }
            }
            if($SourcePath -and $MediaPath){throw 'Select existing installation OR original media, not both'}
            if($Action -eq 'Install' -and -not $SourcePath -and -not $MediaPath -and -not $NonInteractive) {
                $existing=Join-Path $env:LOCALAPPDATA 'Dominion Wars'
                if(Test-Path -LiteralPath (Join-Path $existing 'ds9dw.exe')) {
                    Write-Host "Existing original/Modernizer installation: $existing"
                    $choice=Read-Host 'Enter 1 to create a separate remaster from it, or 2 to use original media (default 1)'
                    if(-not $choice -or $choice -eq '1'){$SourcePath=$existing}
                    elseif($choice -ne '2'){throw 'Unknown choice; nothing changed'}
                }
            }
            $parent=Split-Path -Parent $destinationFull
            $sourceFull=$null
            if($SourcePath) {
                $sourceFull=Assert-RemasterPlainPath $SourcePath
                if($Action -eq 'Install' -and ($destinationFull.StartsWith($sourceFull+'\',[StringComparison]::OrdinalIgnoreCase) -or $sourceFull.StartsWith($destinationFull+'\',[StringComparison]::OrdinalIgnoreCase) -or $sourceFull -eq $destinationFull)){throw 'Source and new destination must be separate, nonnested directories'}
            }
            $null=New-Item -ItemType Directory -Path $parent -Force
            $transaction=[guid]::NewGuid().ToString('N')
            $stage=Join-Path $parent ('.dw-remaster-stage-'+$transaction)
            $previous=Join-Path $parent ('.dw-remaster-previous-'+$transaction)
            $journalPath=Join-Path $parent ('.dw-remaster-transaction-'+$transaction+'.json')
            $null=Assert-RemasterPlainPath $stage;$null=Assert-RemasterPlainPath $previous
            $null=New-Item -ItemType Directory -Path $stage
            $journal=[ordered]@{schema=1;id=$transaction;action=$Action;destination=$destinationFull;previous=$previous;stage=$stage;phase='Prepared';build_id=$manifest.build_id}
            Write-RemasterJson $journalPath $journal
            Import-Module (Join-Path $packageRoot 'foundation\payload\DominionWarsInstall.psm1') -Force
            $guard=Start-DWInstallConsoleGuard
            Write-Host "Preparing $($manifest.build_id) in separate staging. Persistent logs: $stage"
            $game=Join-Path $stage 'game'
            $engine=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
            $foundationArgs=@('-NoLogo','-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $packageRoot 'foundation\DominionWarsModernizer.ps1'),'-Action','Install','-Destination',$game,'-NoDesktopShortcut','-LogDirectory',(Join-Path $stage 'foundation-logs'))
            if($SourcePath) {
                Write-Host 'Verifying and copying original game data; saves are preserved separately.'
                $cleanBase=Join-Path $stage 'verified-source'
                Copy-RemasterOriginal -Source $sourceFull -Target $cleanBase -Manifest $manifest
                $foundationArgs+=@('-SourcePath',$cleanBase)
            } else {
                if($MediaPath){$foundationArgs+=@('-MediaPath',$MediaPath)}
                if($PatchPath){$foundationArgs+=@('-PatchPath',$PatchPath)}
            }
            if($NonInteractive){$foundationArgs+='-NonInteractive'}
            & $engine @foundationArgs
            if($LASTEXITCODE -ne 0){throw 'Modernizer foundation did not complete; staging retained'}
            $foundationStatePath=Join-Path $game '.dw-modernizer\state.json'
            $foundationState=Read-RemasterJson $foundationStatePath
            $foundationState.Destination=$destinationFull
            Write-RemasterJson $foundationStatePath $foundationState
            # Verify all original inputs, not just the executable or file presence.
            foreach($record in $manifest.original_files){Assert-RemasterHash (Get-RemasterChild $game $record.path) $record.sha256}
            if($Action -ne 'Remove') {
                $meta=Join-Path $game '.dw-remaster'
                $null=New-Item -ItemType Directory -Path (Join-Path $meta 'base') -Force
                Copy-Item -LiteralPath (Join-Path $game 'ds9dw.exe') -Destination (Join-Path $meta 'base\ds9dw.exe')
                Invoke-RemasterDelta -InputPath (Join-Path $game 'ds9dw.exe') -DeltaPath (Get-RemasterChild $packageRoot $manifest.engine.path) -OutputPath (Join-Path $meta 'engine.new')
                Copy-Item -LiteralPath (Join-Path $meta 'engine.new') -Destination (Join-Path $game 'ds9dw.exe') -Force
                foreach($assetPatch in @(Get-RemasterAssetPatches $manifest)) {
                    if($assetPatch.target -notin @($manifest.original_files | ForEach-Object {$_.path})){throw 'Asset component must name a verified original file'}
                    $bolt=Get-RemasterChild $game $assetPatch.target
                    $assetBackup=Get-RemasterChild (Join-Path $meta 'base\assets') $assetPatch.target
                    $null=New-Item -ItemType Directory -Path (Split-Path -Parent $assetBackup) -Force
                    Copy-Item -LiteralPath $bolt -Destination $assetBackup
                    $assetOutput=Join-Path $meta ('asset-'+[guid]::NewGuid().ToString('N')+'.new')
                    Invoke-RemasterDelta -InputPath $bolt -DeltaPath (Get-RemasterChild $packageRoot $assetPatch.path) -OutputPath $assetOutput
                    Copy-Item -LiteralPath $assetOutput -Destination $bolt -Force
                }
                $null=New-Item -ItemType Directory -Path (Join-Path $meta 'foundation-entrypoints')
                foreach($entry in Get-ChildItem -LiteralPath $game -Filter '*.cmd' -File) {
                    Move-Item -LiteralPath $entry.FullName -Destination (Join-Path $meta 'foundation-entrypoints')
                }
                foreach($record in $manifest.runtime_files) {
                    $target=Get-RemasterChild $game $record.target
                    $null=New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force
                    Copy-Item -LiteralPath (Get-RemasterChild $packageRoot $record.source) -Destination $target -Force
                }
                if($Backend -eq 'Auto') {
                    Import-Module (Join-Path $game 'DominionWarsLauncher.psm1') -Force
                    Initialize-DominionWarsDisplay
                    $Backend=if([DominionWarsDisplayMode]::CanCreateD3D12Device()){'D3D9On12'}else{'Native'}
                }
                $profileName=if($Backend -eq 'Native'){'native'}else{'d3d9on12'}
                $profile=@($manifest.profiles | Where-Object {$_.name -eq $profileName})[0]
                Copy-Item -LiteralPath (Get-RemasterChild $packageRoot $profile.path) -Destination (Join-Path $game 'DDrawCompat-ds9dw.ini') -Force
                Copy-Item -LiteralPath (Join-Path $packageRoot 'manifest.json') -Destination (Join-Path $meta 'manifest.json')
                Write-RemasterJson (Join-Path $meta 'installed.json') ([ordered]@{schema=1;build_id=$manifest.build_id;source_revision=$manifest.source_revision;profile=$profileName;destination=$destinationFull;installed_utc=[DateTime]::UtcNow.ToString('o');status='integration-candidate';shortcut=$null;shortcut_sha256=$null})
                Write-Host 'Verifying integrated files before committing the destination.'
                $null=Test-RemasterInstalled $game
            }
            $journal.phase='ReadyToCommit';Write-RemasterJson $journalPath $journal
            Assert-RemasterIdle
            if(Test-Path -LiteralPath $destinationFull) {
                $null=Assert-RemasterPlainPath $destinationFull
                Move-Item -LiteralPath $destinationFull -Destination $previous
                $journal.phase='PreviousPreserved';Write-RemasterJson $journalPath $journal
            }
            try {Move-Item -LiteralPath $game -Destination $destinationFull}
            catch {
                if((Test-Path -LiteralPath $previous) -and -not(Test-Path -LiteralPath $destinationFull)) {Move-Item -LiteralPath $previous -Destination $destinationFull}
                throw
            }
            if($Action -ne 'Remove') {
                $state=Test-RemasterInstalled $destinationFull
                if(-not $NoDesktopShortcut) {
                    $state.shortcut=Install-RemasterShortcut $destinationFull $manifest.build_id (Get-RemasterIdentity $destinationFull $manifest)
                    $state.shortcut_sha256=Get-RemasterHash $state.shortcut
                    Write-RemasterJson (Join-Path $destinationFull '.dw-remaster\installed.json') $state
                } elseif($Action -eq 'Repair' -and $oldState.shortcut) {
                    # Keep ownership of an unchanged shortcut when repair intentionally
                    # skips creating one; otherwise later removal would strand it.
                    if((Test-Path -LiteralPath $oldState.shortcut) -and (Get-RemasterHash $oldState.shortcut) -eq $oldState.shortcut_sha256) {
                        $state.shortcut=$oldState.shortcut;$state.shortcut_sha256=$oldState.shortcut_sha256
                        Write-RemasterJson (Join-Path $destinationFull '.dw-remaster\installed.json') $state
                    }
                }
            } elseif($oldState.shortcut) {
                $desktop=[Environment]::GetFolderPath('Desktop')
                if((Split-Path -Parent $oldState.shortcut) -eq $desktop -and (Test-Path -LiteralPath $oldState.shortcut) -and (Get-RemasterHash $oldState.shortcut) -eq $oldState.shortcut_sha256) {
                    Move-Item -LiteralPath $oldState.shortcut -Destination (Join-Path $previous '.dw-remaster\removed-shortcut.lnk')
                } else {Write-Warning 'Modified/missing shortcut preserved; remove any obsolete shortcut manually.'}
            }
            $journal.phase='Complete';Write-RemasterJson $journalPath $journal
            if($CompletionMarker){[IO.File]::WriteAllText($CompletionMarker,'Remaster operation verified and completed.',(New-Object Text.UTF8Encoding($false)))}
            Write-Host "$Action completed: $destinationFull"
            if(Test-Path -LiteralPath $previous){Write-Host "Previous installation and saves retained for rollback: $previous"}
            Write-Host "Logs and transaction retained: $stage ; $journalPath"
            $exitCode=0
        }
    }
} catch {
    Write-Error -ErrorAction Continue $_
    if($stage){Write-Warning "Operation incomplete. Staging/logs retained at $stage. Do not treat it as installed."}
} finally {
    if($guard){Stop-DWInstallConsoleGuard $guard}
    if($owns){$mutex.ReleaseMutex()};if($mutex){$mutex.Dispose()}
}
exit $exitCode
