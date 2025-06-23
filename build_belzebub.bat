@echo off
setlocal

set "MSYS2_ROOT=C:\msys64"
set "PROJECT_DIR=%~dp0"

echo ===============================================
echo Iniciando compilación de Belzebub...
echo ===============================================

set "PROJECT_UNIX=%PROJECT_DIR:C:\=/c/%"
set "PROJECT_UNIX=%PROJECT_UNIX:\=/%"

"%MSYS2_ROOT%\mingw64.exe" -c "cd '%PROJECT_UNIX%' && mkdir -p build && cd build && cmake .. -G 'MinGW Makefiles' -DCMAKE_VERBOSE_MAKEFILE=ON && mingw32-make VERBOSE=1 && ls -la"

if %errorlevel% neq 0 (
    echo ERROR: Falló la compilación.
) else (
    echo Compilación completada con éxito.
)

echo ===============================================
echo Proceso completado.
echo ===============================================
pause
