# Private recovery-state read fix

Observed failures before display switching/game launch: the launcher polls its
helper's JSON using Get-Content while the helper publishes Ready using atomic
replacement. Reads sometimes reported sharing violations or a missing file.
The watchdog remained enabled; these failures ended Prepared/NotArmed.

Only game-process-probe-recovery-v1 contains the fix. It is a separate copy of
the prepared projection comparison. No public installer, normal installation,
working baseline or prior test launcher is modified.

Read-DominionWarsRecoveryState opens with ReadWrite|Delete sharing and retries
IOException up to 20 attempts, 50 ms between attempts. Complete JSON is parsed
with terminating errors. Malformed JSON and non-I/O errors are not silently
accepted. Existing helper readiness timeout, phase rules, monitor identity,
process identity, display/recovery mutexes and original restoration decisions
remain. Ready acknowledgement also requires matching parent PID/start ticks.
Both parent and helper use the reader; original atomic writer is unchanged.

Changed files: DominionWarsLauncher.psm1 and Restore-DominionWarsDesktop.ps1.
Exact hashes are pinned in Run-ProcessProbeRecovery-v1.ps1. The game executable
remains the exact prior 1.5 probe and the DLL the exact processing observer.

Validation: Test-RecoveryRead-v1.ps1 passes on PowerShell 7 and Windows PowerShell
5.1: ordinary read, transient exclusive lock, 250 reads concurrent with 200 atomic
replacements, invalid JSON rejection and bounded missing-file failure. Real helper
test on Windows PowerShell 5.1 passes five readiness/completion cycles using
private state paths. No game or display Apply calls, no Armed state, unchanged
desktop checked. This is local validation, not a universal recovery guarantee.

Subsequent game run process-probe-recovery-v1-20261017T210326078Z launched on its
first attempt, armed recovery, exited 0 and restored the exact profile and desktop
(AlreadyRestored). One game run does not establish that every recovery race is fixed.

Follow-up for compatibility branch: review/port the reader and tests there; do not
blindly replace current launcher files with this older pinned research snapshot.
