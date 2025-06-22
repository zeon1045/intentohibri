@echo off
cls
echo ========================================================
echo   Belzebub - Compilador Forzado con CMake + MinGW
echo ========================================================
echo.

:: --- CONFIGURACIÓN ---
:: Establece la ruta raíz de MSYS2. Ajústala si lo instalaste en otro lugar.
set "MSYS2_ROOT=C:\msys64"
set "MINGW_PATH=%MSYS2_ROOT%\mingw64\bin"

:: Define explícitamente las rutas a los compiladores de C y CXX de MinGW
set "C_COMPILER=%MINGW_PATH%\gcc.exe"
set "CXX_COMPILER=%MINGW_PATH%\g++.exe"

:: --- EJECUCIÓN ---

:: 1. Añadir MinGW-w64 al PATH para esta sesión
echo [INFO] Configurando el PATH para incluir MinGW: %MINGW_PATH%
set "PATH=%MINGW_PATH%;%PATH%"
echo.

:: 2. Verificar que los compiladores y herramientas existan
echo [INFO] Verificando herramientas de compilacion...
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [FATAL] El comando 'cmake' no se encuentra. Asegurate de que este instalado y en el PATH del sistema.
    goto :fail
)
where mingw32-make >nul 2>nul
if %errorlevel% neq 0 (
    echo [FATAL] El comando 'mingw32-make' no se encuentra. Revisa tu instalacion de MinGW-w64.
    goto :fail
)
if not exist "%CXX_COMPILER%" (
    echo [FATAL] No se encuentra el compilador de C++ g++.exe en: %CXX_COMPILER%
    goto :fail
)
echo [SUCCESS] Herramientas de compilacion encontradas.
echo.

:: 3. Preparar el directorio de build (empezando desde cero)
set "SCRIPT_DIR=%~dp0"
echo [INFO] Preparando directorio de compilacion 'build' (limpiando)...
if exist "%SCRIPT_DIR%build" (
    rmdir /s /q "%SCRIPT_DIR%build"
)
mkdir "%SCRIPT_DIR%build"
cd /d "%SCRIPT_DIR%build"
echo.

:: 4. Ejecutar CMake para generar los Makefiles de MinGW, forzando los compiladores
echo [INFO] Ejecutando CMake para generar los archivos de proyecto...
cmake .. -G "MinGW Makefiles" ^
    -D CMAKE_C_COMPILER="%C_COMPILER%" ^
    -D CMAKE_CXX_COMPILER="%CXX_COMPILER%"
    
if %errorlevel% neq 0 (
    echo [ERROR] CMake falló al generar los Makefiles.
    goto :fail
)
echo [SUCCESS] Makefiles generados correctamente.
echo.

:: 5. Compilar el proyecto con mingw32-make
echo [INFO] Compilando el proyecto...
mingw32-make
if %errorlevel% neq 0 (
    echo [ERROR] La compilación con mingw32-make ha fallado.
    goto :fail
)

echo.
echo =================================================
echo   COMPILACION COMPLETADA CON EXITO
echo =================================================
echo.
echo   El ejecutable 'Belzebub.exe' y sus recursos
echo   se encuentran en la carpeta 'build'.
echo.
goto :eof

:fail
echo.
echo [FATAL] La compilacion ha fallado. Revisa los mensajes de error.
echo.
pause
exit /b 1