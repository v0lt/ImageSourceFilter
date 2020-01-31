@echo off
echo.
echo.
title Restore MPC Image Source default settings...
start /min reg delete "HKEY_CURRENT_USER\Software\MPC-BE Filters\MPC Image Source" /f
echo    settings were reset to default
echo.
pause >NUL
