@echo off
call "%~dp0Play Dominion Wars.cmd" -ConfigureOnly -RestoreCompatibilityProfile
exit /b %ERRORLEVEL%
