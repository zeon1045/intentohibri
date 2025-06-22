@echo off
cls
echo =============================================
echo    BYOVD Professional Suite v3.0 - VISUAL STUDIO
echo =============================================
echo.

REM --- Configuración del Entorno de Visual Studio ---
set "VS_BAT_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if exist "%VS_BAT_PATH%" (
    call "%VS_BAT_PATH%"
) else (
    echo ERROR: No se encontro vcvars64.bat. Asegurate de que Visual Studio 2022 Community este instalado.
    goto :error
)
if errorlevel 1 (
    echo ERROR: No se pudo configurar el entorno de Visual Studio.
    goto :error
)
echo ✓ Entorno de Visual Studio configurado.

REM --- Preparación del Directorio de Compilación ---
set "BUILD_DIR=%~dp0build"
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

echo.
echo Compilando con Visual Studio...

REM --- Compilación de Archivos Objeto ---
cl /std:c++17 /O2 /W3 /EHsc /MD /I"%~dp0backend" /c "%~dp0backend\main.cpp" /Fo:"%BUILD_DIR%\main.obj"
if errorlevel 1 (
    echo ERROR: No se pudo compilar main.cpp
    goto :error
)
echo ✓ main.cpp compilado exitosamente.

cl /std:c++17 /O2 /W3 /EHsc /MD /I"%~dp0backend" /c "%~dp0backend\injection_engine.cpp" /Fo:"%BUILD_DIR%\injection_engine.obj"
if errorlevel 1 (
    echo ERROR: No se pudo compilar injection_engine.cpp
    goto :error
)
echo ✓ injection_engine.cpp compilado exitosamente.

REM --- Enlazado del Ejecutable ---
echo.
echo Enlazando ejecutable...

REM =================================================================================
REM CORRECCIÓN FINAL Y DEFINITIVA:
REM Buscamos la ubicación de vswhere.exe para encontrar la ruta de instalación de VS.
REM Luego, construimos la ruta a las librerías de forma manual y explícita.
REM Esto es mucho más robusto que depender de las variables de entorno.
REM =================================================================================

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
    set "VS_INSTALL_PATH=%%i"
)

if not defined VS_INSTALL_PATH (
    echo ERROR: No se pudo encontrar la ruta de instalacion de Visual Studio con vswhere.exe.
    goto :error
)

REM Construir la ruta a las librerías de MSVC
set "MSVC_LIB_PATH=%VS_INSTALL_PATH%\VC\Tools\MSVC"
for /d %%d in ("%MSVC_LIB_PATH%\*") do set "MSVC_LIB_PATH=%%d\lib\x64"

link /OUT:"%BUILD_DIR%\BYOVD_Professional.exe" ^
     "%BUILD_DIR%\main.obj" "%BUILD_DIR%\injection_engine.obj" ^
     /SUBSYSTEM:CONSOLE ^
     /LIBPATH:"%MSVC_LIB_PATH%" ^
     /LIBPATH:"%UniversalCRTSdkDir%Lib\%UCRTVersion%\ucrt\x64" ^
     /LIBPATH:"%UniversalCRTSdkDir%Lib\%UCRTVersion%\um\x64" ^
     shell32.lib advapi32.lib user32.lib kernel32.lib ws2_32.lib msvcprt.lib

if errorlevel 1 (
    echo ERROR: No se pudo crear el ejecutable. El enlazador no encontro las librerias necesarias.
    goto :error
)
echo ✓ Ejecutable creado exitosamente.

REM --- Copia de Recursos ---
echo.
echo Copiando recursos a la carpeta 'build'...
if exist "%~dp0core_dlls" (
    xcopy /s /y /q "%~dp0core_dlls" "%BUILD_DIR%\core_dlls\"
    echo ✓ Componentes de Cheat Engine copiados.
)
if exist "%~dp0frontend" (
    xcopy /s /y /q "%~dp0frontend" "%BUILD_DIR%\frontend\"
    echo ✓ Interfaz web copiada.
)
if exist "%~dp0drivers" (
    xcopy /s /y /q "%~dp0drivers" "%BUILD_DIR%\drivers\"
    echo ✓ Drivers copiados.
)

REM --- Finalización ---
echo.
echo =============================================
echo    COMPILACION COMPLETADA CON EXITO
echo =============================================
echo.
echo Tu aplicacion esta lista en la carpeta 'build'.
echo Ejecutable: %BUILD_DIR%\BYOVD_Professional.exe
echo.
goto :end

:error
echo.
echo La compilacion ha fallado.
pause

:end
