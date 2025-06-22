# =============================================================================
#  Script para Obtener Drivers Vulnerables (Versión Asistente Interactivo Robusto)
#  Maneja correctamente la extracción de drivers y es más resistente a errores.
# =============================================================================

# --- CONFIGURACIÓN DE RUTA ---
# Modifica esta línea con la ruta completa a tu carpeta de drivers.
$driversPathOverride = "C:\Users\mosta\Desktop\belbel\drivers"


# --- Configuración de Software (Enlaces y Lógica Actualizados) ---
$targets = @(
    @{
        Name = "CPU-Z"
        Url = "https://download.cpuid.com/cpu-z/cpu-z_2.15-en.zip"
        DriverPattern = "cpuz*.sys"
        DestinationFile = "cpuz_x64.sys"
        Type = "ExeInZip" 
    },
    @{
        Name = "HWiNFO64"
        Url = "https://sitsa.dl.sourceforge.net/project/hwinfo/Windows_Portable/hwi_826.zip?viasf=1"
        DriverPattern = "HWiNFO*.SYS"
        DestinationFile = "HWiNFO64A.SYS"
        Type = "ExeInZip"
    },
    @{
        Name = "MSI Afterburner"
        Url = "https://us2-dl.techpowerup.com/files/MSIAfterburnerSetup465.zip"
        DriverPattern = "RTCore64.sys"
        DestinationFile = "RTCore64.sys"
        Type = "Zip"
    },
    @{
        Name = "Gigabyte Control Center"
        Url = "https://download.gigabyte.com/FileList/Utility/GCC_25.06.04.01.zip"
        DriverPattern = "gdrv*.sys"
        DestinationFile = "gdrv.sys"
        Type = "Zip"
    },
    @{
        Name = "Process Hacker"
        Url = "https://github.com/processhacker/processhacker/releases/download/v2.39/processhacker-2.39-setup.exe"
        DriverPattern = "kprocesshacker.sys"
        DestinationFile = "kprocesshacker.sys"
        Type = "Exe"
    }
)

# --- Inicio del Script ---

# Habilita el modo estricto para detectar errores de variables no inicializadas.
Set-StrictMode -Version Latest

$colorInfo = "Cyan"; $colorSuccess = "Green"; $colorWarn = "Yellow"; $colorError = "Red"
$baseDir = $PSScriptRoot
$tempDir = Join-Path $baseDir "temp_installers"

if (-not [string]::IsNullOrEmpty($driversPathOverride)) { $driversDir = $driversPathOverride } 
else { $driversDir = Join-Path $baseDir "drivers" }

if (-not (Test-Path $driversDir)) { Write-Host "Creando carpeta '$driversDir'..." -F $colorInfo; New-Item -Path $driversDir -ItemType Directory | Out-Null }
if (-not (Test-Path $tempDir)) { New-Item -Path $tempDir -ItemType Directory | Out-Null }

Write-Host "=================================================" -F $colorInfo
Write-Host "   Asistente de Obtención de Drivers Seguros   "
Write-Host "=================================================" -F $colorInfo; Write-Host

$headers = @{ "User-Agent" = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36" }

foreach ($target in $targets) {
    Write-Host "--- Procesando: $($target.Name) ---" -ForegroundColor "White"
    
    $driverDestPath = Join-Path $driversDir $target.DestinationFile
    if (Test-Path $driverDestPath) {
        Write-Host "El driver '$($target.DestinationFile)' ya existe. Omitiendo." -F $colorSuccess
        Write-Host; continue
    }

    # Lógica mejorada y más segura para obtener el nombre del archivo
    try {
        $uri = [System.Uri]$target.Url
        $fileName = [System.IO.Path]::GetFileName($uri.AbsolutePath)
    } catch {
        $fileName = $target.Url.Split('/')[-1].Split('?')[0]
    }
    
    if ([string]::IsNullOrEmpty($fileName)) {
        Write-Host "No se pudo determinar el nombre del archivo para $($target.Name). Omitiendo." -F $colorError
        continue
    }

    $downloadPath = Join-Path $tempDir $fileName

    try {
        Write-Host "Descargando instalador..." -F $colorInfo
        Invoke-WebRequest -Uri $target.Url -OutFile $downloadPath -UseBasicParsing -Headers $headers -TimeoutSec 300
        Write-Host "Descarga completa: $fileName" -F $colorSuccess
    } catch {
        Write-Host "La descarga automática falló. El enlace puede estar roto." -F $colorError
        # ... (código para descarga manual) ...
        continue
    }

    try {
        if ($target.Type -eq "Zip") {
            Write-Host "Extrayendo archivos del ZIP..." -F $colorInfo
            Expand-Archive -Path $downloadPath -DestinationPath $tempDir -Force
            
            Write-Host "Buscando patrón '$($target.DriverPattern)'..." -F $colorInfo
            $foundDriver = Get-ChildItem -Path $tempDir -Recurse -Filter $target.DriverPattern | Select-Object -First 1
            
            if ($foundDriver) {
                Write-Host "¡Driver encontrado!: $($foundDriver.Name)" -F $colorSuccess
                Copy-Item -Path $foundDriver.FullName -Destination $driverDestPath
                Write-Host "'$($target.DestinationFile)' ha sido copiado a '$driversDir'." -F $colorSuccess
            } else {
                 Write-Host "No se pudo encontrar '$($target.DriverPattern)' automáticamente en el ZIP." -F $colorWarn
            }

        } elseif ($target.Type -eq "Exe" -or $target.Type -eq "ExeInZip") {
            $exePath = $downloadPath
            if ($target.Type -eq "ExeInZip") {
                Write-Host "Extrayendo archivos del ZIP..." -F $colorInfo
                Expand-Archive -Path $downloadPath -DestinationPath $tempDir -Force
                $foundExe = Get-ChildItem -Path $tempDir -Recurse -Filter "*.exe" | Where-Object { $_.Name -like "*64.exe" -or $_.Name -like "*x64.exe" } | Select-Object -First 1
                if ($foundExe) { $exePath = $foundExe.FullName } 
                else { throw "No se encontró el ejecutable x64 dentro del ZIP para $($target.Name)." }
            }

            Write-Host "El driver para '$($target.Name)' se carga al ejecutar el programa." -F $colorWarn
            Write-Host "1. Por favor, ejecuta el siguiente programa:" -F $colorWarn
            Write-Host "   $exePath"
            Write-Host "2. Mantén el programa abierto y vuelve a esta ventana."
            Read-Host "Presiona Enter cuando el programa esté en ejecución..."
            
            $searchPaths = @(
                (Join-Path $env:SystemRoot "System32\drivers"),
                (Join-Path $env:TEMP),
                (Join-Path ${env:ProgramFiles} "Process Hacker 2"),
                (Join-Path ${env:ProgramFiles} "CPUID\CPU-Z"),
                (Join-Path ${env:ProgramFiles} "HWiNFO64")
            )

            $foundSystemDriver = $false
            while (-not $foundSystemDriver) {
                foreach($path in $searchPaths) {
                    $actualDriver = Get-ChildItem -Path $path -Filter $target.DriverPattern -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
                    if ($actualDriver) {
                        Write-Host "¡Driver encontrado en '$($actualDriver.FullName)'!" -F $colorSuccess
                        Copy-Item -Path $actualDriver.FullName -Destination $driverDestPath
                        Write-Host "'$($target.DestinationFile)' ha sido copiado a '$driversDir'." -F $colorSuccess
                        $foundSystemDriver = $true; break
                    }
                }
                if (-not $foundSystemDriver) {
                    Write-Host "No se pudo encontrar '$($target.DriverPattern)' en las rutas comunes." -F $colorError
                    $userPath = Read-Host "Por favor, pega la ruta donde se está ejecutando '$($target.Name)' (o presiona Enter para omitir)"
                    if ([string]::IsNullOrWhiteSpace($userPath)) { break }
                    if (Test-Path $userPath) { $searchPaths += $userPath }
                    else { Write-Host "Ruta no válida." -F $colorError }
                }
            }
        }
    } catch {
        Write-Host "Ocurrió un error al procesar el archivo para '$($target.Name)':" -F $colorError
        Write-Host $_.Exception.Message -F $colorError
    }
    Write-Host
}

Write-Host "Limpiando archivos temporales..." -F $colorInfo
Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "¡Proceso completado!" -ForegroundColor "White"
