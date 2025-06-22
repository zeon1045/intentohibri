# =============================================================================
#  Script de Instalación del Entorno de Compilación para BYOVD (Corregido)
#  Instala automáticamente MSYS2 y el compilador MinGW-w64 (G++).
#  IMPORTANTE: Debe ejecutarse como Administrador.
# =============================================================================

# --- Configuración ---
$msys2_url = "https://github.com/msys2/msys2-installer/releases/download/2024-01-13/msys2-x86_64-20240113.exe"
$msys2_installer = "msys2-setup.exe"
$msys2_root = "C:\msys64"
$mingw_bin_path = Join-Path $msys2_root "mingw64\bin"

# --- Inicio del Script ---
$colorInfo = "Cyan"
$colorSuccess = "Green"
$colorWarn = "Yellow"
$colorError = "Red"

# 1. Verificar si se está ejecutando como Administrador
Write-Host "Verificando permisos de administrador..." -ForegroundColor $colorInfo
if (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "ERROR: Este script debe ejecutarse como Administrador." -ForegroundColor $colorError
    Write-Host "Por favor, haz clic derecho en PowerShell y selecciona 'Ejecutar como administrador'." -ForegroundColor $colorError
    Read-Host "Presiona Enter para salir."
    exit
}
Write-Host "Permisos de administrador confirmados." -ForegroundColor $colorSuccess
Write-Host

# 2. Descargar el instalador de MSYS2
Write-Host "=================================================" -ForegroundColor $colorInfo
Write-Host "  Paso 1: Descargando MSYS2"
Write-Host "=================================================" -ForegroundColor $colorInfo
if (Test-Path $msys2_installer) {
    Write-Host "El instalador de MSYS2 ya existe. Omitiendo descarga." -ForegroundColor $colorWarn
} else {
    Write-Host "Descargando desde: $msys2_url"
    try {
        Invoke-WebRequest -Uri $msys2_url -OutFile $msys2_installer -UseBasicParsing
        Write-Host "Descarga de MSYS2 completada." -ForegroundColor $colorSuccess
    } catch {
        Write-Host "ERROR: No se pudo descargar MSYS2. Revisa tu conexion a internet o el enlace." -ForegroundColor $colorError
        Read-Host "Presiona Enter para salir."
        exit
    }
}
Write-Host

# 3. Instalar MSYS2 de forma silenciosa
Write-Host "=================================================" -ForegroundColor $colorInfo
Write-Host "  Paso 2: Instalando MSYS2 (puede tardar unos minutos)"
Write-Host "=================================================" -ForegroundColor $colorInfo
if (Test-Path $msys2_root) {
    Write-Host "La carpeta de MSYS2 ya existe en '$msys2_root'. Omitiendo instalacion." -ForegroundColor $colorWarn
} else {
    Write-Host "Ejecutando instalador en modo silencioso..."
    Start-Process -FilePath ".\$msys2_installer" -ArgumentList '--noconfirm --s --needed --root C:\msys64' -Wait
    Write-Host "Instalacion de MSYS2 completada." -ForegroundColor $colorSuccess
}
Write-Host

# 4. Instalar la cadena de herramientas de compilación de MinGW-w64
Write-Host "=================================================" -ForegroundColor $colorInfo
Write-Host "  Paso 3: Instalando el compilador G++ (MinGW-w64)"
Write-Host "=================================================" -ForegroundColor $colorInfo
Write-Host "Esto puede tardar varios minutos dependiendo de tu conexion..."
$msys2_bash = Join-Path $msys2_root "usr\bin\bash.exe"
$pacman_command = "pacman -S --noconfirm --needed base-devel mingw-w64-x86_64-toolchain"
& $msys2_bash -l -c $pacman_command
Write-Host "Instalacion del compilador G++ completada." -ForegroundColor $colorSuccess
Write-Host

# 5. Añadir el compilador al PATH del sistema
Write-Host "=================================================" -ForegroundColor $colorInfo
Write-Host "  Paso 4: Configurando las variables de entorno (PATH)"
Write-Host "=================================================" -ForegroundColor $colorInfo
$oldPath = [Environment]::GetEnvironmentVariable('Path', [System.EnvironmentVariableTarget]::Machine)
if ($oldPath -like "*$mingw_bin_path*") {
    Write-Host "El PATH ya esta configurado correctamente." -ForegroundColor $colorWarn
} else {
    Write-Host "Anadiendo '$mingw_bin_path' al PATH del sistema..."
    $newPath = "$mingw_bin_path;$oldPath"
    [Environment]::SetEnvironmentVariable('Path', $newPath, [System.EnvironmentVariableTarget]::Machine)
    Write-Host "La variable de entorno PATH ha sido actualizada." -ForegroundColor $colorSuccess
    Write-Host "Necesitaras abrir una NUEVA ventana de terminal para que los cambios surtan efecto." -ForegroundColor $colorWarn
}
Write-Host

# --- Finalización ---
Write-Host "=================================================" -ForegroundColor $colorSuccess
Write-Host "  ¡INSTALACION COMPLETADA EXITOSAMENTE!  "
Write-Host "=================================================" -ForegroundColor $colorSuccess
Write-Host
Write-Host "Para verificar la instalacion:"
Write-Host "1. Cierra esta ventana de PowerShell."
Write-Host "2. Abre una NUEVA ventana de PowerShell o CMD."
Write-Host "3. Escribe el comando: g++ --version"
Write-Host "Deberias ver la version del compilador G++ que acabamos de instalar."
Write-Host
Read-Host "Presiona Enter para finalizar el script."
