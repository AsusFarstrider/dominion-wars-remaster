@echo off
setlocal
set "DW_COMPLETION=%TEMP%\dw-remaster-%RANDOM%-%RANDOM%.complete"
if exist "%DW_COMPLETION%" (
  echo Completion marker conflict. Please run this installer again.
  pause
  exit /b 1
)
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Remaster.ps1" -Action Install -CompletionMarker "%DW_COMPLETION%" %*
if errorlevel 1 goto failed
if not exist "%DW_COMPLETION%" goto failed
echo Remaster installation verified. Use the Dominion Wars Remaster desktop shortcut.
pause
exit /b 0
:failed
echo Installation did not complete successfully. Read the error above. Existing installations were not overwritten by a fresh install.
pause
exit /b 1
