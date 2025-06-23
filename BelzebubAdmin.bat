@echo off
title Belzebub Professional Suite - Administrator Mode
echo.
echo ================================================
echo   🔥 Belzebub Professional Suite - Admin Mode 🔥
echo ================================================
echo.
echo Ejecutando con privilegios de administrador...
echo.

REM Cambiar al directorio del script
cd /d "%~dp0"

REM Ejecutar Belzebub
Belzebub.exe

echo.
echo Belzebub ha terminado. Presiona cualquier tecla para salir...
pause >nul 