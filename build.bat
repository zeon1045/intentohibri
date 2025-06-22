@echo off
echo =========================================
echo   BYOVD Professional Suite v2.0
echo   Sistema de Seleccion Multiple
echo =========================================
echo.

REM Verificar si g++ está disponible
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] g++ no encontrado. Instale MinGW-w64 o MSYS2 primero.
    echo.
    echo Descarga MinGW-w64 desde: https://www.mingw-w64.org/downloads/
    echo O instala MSYS2 desde: https://www.msys2.org/
    echo.
    pause
    exit /b 1
)

echo [INFO] Compilador g++ encontrado
echo [INFO] Navegando a la carpeta del backend...
cd backend

echo [INFO] Compilando backend con sistema de seleccion multiple...
echo.

REM Compilar con soporte para filesystem (C++17)
g++ -std=c++17 main.cpp injection_engine.cpp -o ../BYOVD_Backend_v2.exe -lws2_32 -lwsock32 -lshell32 -ladvapi32 -static-libgcc -static-libstdc++

REM Verificar si la compilación fue exitosa
if %errorlevel% equ 0 (
    echo.
    echo [EXITO] Compilacion completada exitosamente!
    echo [INFO] Se ha creado 'BYOVD_Backend_v2.exe'
    echo.
    echo Caracteristicas del nuevo sistema:
    echo   - Deteccion automatica de drivers disponibles
    echo   - Seleccion dinamica de drivers vulnerables  
    echo   - Interfaz web mejorada con informacion detallada
    echo   - Sistema de logs avanzado
    echo   - Comandos de consola interactivos
    echo.
) else (
    echo.
    echo [ERROR] La compilacion ha fallado.
    echo [INFO] Errores comunes y soluciones:
    echo   - Falta std::filesystem: Actualice a g++ 8.0 o superior
    echo   - Error de librerias: Instale las librerias de desarrollo
    echo   - Permisos: Ejecute como administrador
    echo.
)

REM Volver a la carpeta raiz
cd ..

echo.
echo [INFO] Para ejecutar el sistema:
echo   1. Obtenga drivers vulnerables REALES de software legítimo
echo      - Gigabyte: App Center, RGB Fusion (para gdrv.sys)
echo      - MSI: Dragon Center, Afterburner (para msio64.sys, RTCore64.sys)
echo      - ASUS: Armoury Crate (para AsIO.sys)
echo      - CPU-Z, HWiNFO64 (para cpuz_x64.sys, HWiNFO64A.SYS)
echo   2. Coloque los archivos .sys en la carpeta 'drivers/'
echo   3. Coloque las DLLs de CE desde instalación oficial en 'core_dlls/'  
echo   4. Ejecute 'BYOVD_Backend_v2.exe' como ADMINISTRADOR
echo   5. NUNCA use drivers de fuentes no oficiales
echo.
echo [ADVERTENCIA] Este software es solo para fines educativos
echo              Use únicamente en sistemas que posea completamente
echo.
pause 