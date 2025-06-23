@echo off
REM -----------------------------------------------
REM Abrir MSYS2 MinGW 64-bit en la carpeta del proyecto (con entorno correcto)
REM -----------------------------------------------

set "MSYS2_ROOT=C:\msys64"
set "PROJECT_DIR=%~dp0"

REM Convertir ruta a formato Unix (/c/...)
set "PROJECT_UNIX=%PROJECT_DIR:C:\=/c/%"
set "PROJECT_UNIX=%PROJECT_UNIX:\=/%"

REM Lanzar mintty con el shell correcto (MinGW64)
start "" "%MSYS2_ROOT%\usr\bin\mintty.exe" -e "%MSYS2_ROOT%\usr\bin\env.exe" MSYSTEM=MINGW64 /bin/bash --login -c "cd '%PROJECT_UNIX%' && exec bash"
