@echo off
setlocal
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-Remaster.ps1"
if errorlevel 1 (
  echo Remaster launch did not complete successfully. Review the logs or run remaster verification.
  pause
  exit /b 1
)
exit /b 0
