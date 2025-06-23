@echo off
REM --------------------------------------------------
REM install_dependencies.bat - Instalación automatizada de MSYS2 y dependencias para Belzebub
REM --------------------------------------------------

:: Forzar ejecución como administrador
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Elevando permisos de administrador...
    powershell -Command "Start-Process '%~f0' -Verb runAs"
    exit /b
)

REM Definir ruta de MSYS2
set "MSYS2_ROOT=C:\msys64"
set "MSYS2_INSTALLER=%TEMP%\msys2-installer.exe"
set "MSYS2_URL=https://github.com/msys2/msys2-installer/releases/latest/download/msys2-x86_64-latest.exe"

echo ===============================================
echo Iniciando instalación automatizada de dependencias...
echo ===============================================

if not exist "%MSYS2_ROOT%\usr\bin\bash.exe" (
    echo Descargando MSYS2 desde %MSYS2_URL%...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri '%MSYS2_URL%' -OutFile '%MSYS2_INSTALLER%'"
    if not exist "%MSYS2_INSTALLER%" (
        echo ERROR: No se pudo descargar el instalador de MSYS2.
        pause
        exit /b 1
    )
    echo Instalando MSYS2...
    start /wait "" "%MSYS2_INSTALLER%" /S /D=%MSYS2_ROOT%
    if %errorlevel% neq 0 (
        echo ERROR: Falló la instalación silenciosa de MSYS2.
        pause
        exit /b 1
    )
) else (
    echo MSYS2 ya está instalado en %MSYS2_ROOT%.
)

REM Actualizar paquetes base
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -Sy --noconfirm && pacman -Su --noconfirm"
if %errorlevel% neq 0 (
    echo ERROR: Falló la actualización de MSYS2.
    pause
    exit /b 1
)

REM Instalar herramientas necesarias
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -S --needed --noconfirm mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-make git"
if %errorlevel% neq 0 (
    echo ERROR: Falló la instalación de herramientas.
    pause
    exit /b 1
)

echo ===============================================
echo Dependencias instaladas correctamente.
echo ===============================================

pause