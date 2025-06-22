@echo off
cls
echo ========================================================
echo   Belzebub - Professional Suite - Compilador Visual Studio
echo ========================================================
echo.

REM --- Paso 1: Encontrar y Configurar el Entorno de Visual Studio ---
echo [INFO] Buscando la instalacion de Visual Studio...

set "VSWHERE_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE_PATH%" (
    echo [ERROR] No se encontro 'vswhere.exe' en la ruta esperada.
    echo          Asegurate de que el Instalador de Visual Studio este presente.
    goto :error
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE_PATH%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL_PATH=%%i"
)

if not defined VS_INSTALL_PATH (
    echo [ERROR] No se pudo encontrar una instalacion de Visual Studio con las herramientas de C++.
    echo          Abre "Visual Studio Installer" y asegurate de que la carga de trabajo
    echo          "Desarrollo de escritorio con C++" este instalada.
    goto :error
)

set "VS_BAT_PATH=%VS_INSTALL_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VS_BAT_PATH%" (
    echo [ERROR] No se encontro el archivo vcvars64.bat en la ruta esperada.
    echo          Ruta buscada: %VS_BAT_PATH%
    goto :error
)

echo [INFO] Configurando el entorno desde: "%VS_BAT_PATH%"
call "%VS_BAT_PATH%" > nul
if errorlevel 1 (
    echo [ERROR] Fallo al ejecutar vcvars64.bat. El entorno no se pudo configurar.
    goto :error
)
echo [SUCCESS] Entorno de Visual Studio configurado exitosamente.
echo.

REM --- Paso 2: Preparar Directorios y Entorno de Trabajo ---
set "ROOT_DIR=%~dp0"
set "BUILD_DIR=%ROOT_DIR%build"

pushd "%ROOT_DIR%"

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    echo [INFO] Directorio 'build' creado.
)
echo.
echo [INFO] Compilando archivos fuente...

REM --- Paso 3: Compilacion de Archivos Objeto ---
cl /std:c++17 /O2 /W3 /EHsc /MD /I"." /c "backend\main.cpp" /Fo:"%BUILD_DIR%\main.obj"
if errorlevel 1 (
    echo [ERROR] No se pudo compilar main.cpp
    goto :error
)
cl /std:c++17 /O2 /W3 /EHsc /MD /I"." /c "backend\injection_engine.cpp" /Fo:"%BUILD_DIR%\injection_engine.obj"
if errorlevel 1 (
    echo [ERROR] No se pudo compilar injection_engine.cpp
    goto :error
)
echo [SUCCESS] Fuentes compiladas a archivos objeto.
echo.

REM --- Paso 4: Enlazado del Ejecutable (Metodo Robusto con Rutas Explícitas) ---
echo [INFO] Enlazando para crear el ejecutable...

REM Construir la ruta a las librerías de MSVC manualmente
set "MSVC_LIB_PATH=%VS_INSTALL_PATH%\VC\Tools\MSVC"
for /d %%d in ("%MSVC_LIB_PATH%\*") do set "MSVC_LIB_PATH=%%d\lib\x64"

link /OUT:"%BUILD_DIR%\Belzebub.exe" ^
     "%BUILD_DIR%\main.obj" "%BUILD_DIR%\injection_engine.obj" ^
     /SUBSYSTEM:CONSOLE ^
     /LIBPATH:"%MSVC_LIB_PATH%" ^
     /LIBPATH:"%UniversalCRTSdkDir%Lib\%UCRTVersion%\ucrt\x64" ^
     /LIBPATH:"%UniversalCRTSdkDir%Lib\%UCRTVersion%\um\x64" ^
     shell32.lib user32.lib kernel32.lib ws2_32.lib advapi32.lib msvcprt.lib pdh.lib
     
if errorlevel 1 (
    echo [ERROR] El enlazado ha fallado.
    goto :error
)
echo [SUCCESS] Ejecutable 'Belzebub.exe' creado en la carpeta 'build'.
echo.

REM --- Paso 5: Copia de Recursos Esenciales ---
echo [INFO] Copiando recursos necesarios a la carpeta 'build'...
if exist "core_dlls" (
    xcopy /s /y /q "core_dlls" "%BUILD_DIR%\core_dlls\" > nul
)
if exist "drivers" (
    xcopy /s /y /q "drivers" "%BUILD_DIR%\drivers\" > nul
)
if exist "frontend" (
    xcopy /s /y /q "frontend" "%BUILD_DIR%\frontend\" > nul
)
echo [SUCCESS] Recursos copiados.

REM --- Finalizacion ---
echo.
echo ========================================================
echo      COMPILACION COMPLETADA CON EXITO
echo ========================================================
echo.
echo Tu aplicacion esta lista en la carpeta 'build'.
echo Ejecuta 'Belzebub.exe' como Administrador para iniciar.
goto :end

:error
echo.
echo [FATAL] La compilacion ha fallado. Revisa los mensajes de error.

:end
echo.
pause