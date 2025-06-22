# =====================================================================================
# BYOVD Professional Suite v3.0 - Script de Descarga de Drivers
# Este script ayuda a obtener drivers vulnerables de fuentes LEGALES y OFICIALES
# =====================================================================================

Write-Host "================================================" -ForegroundColor Cyan
Write-Host " BYOVD Professional Suite v3.0 - Driver Setup" -ForegroundColor Cyan  
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "[INFO] Este script te guiará para obtener drivers vulnerables LEGALMENTE." -ForegroundColor Yellow
Write-Host ""

# Crear carpeta drivers si no existe
if (-not (Test-Path "drivers")) {
    New-Item -ItemType Directory -Path "drivers" | Out-Null
    Write-Host "[CREATED] Carpeta 'drivers' creada." -ForegroundColor Green
}

Write-Host "=== DRIVERS DISPONIBLES Y SUS FUENTES OFICIALES ===" -ForegroundColor White
Write-Host ""

$drivers = @(
    @{
        Name = "Gigabyte GDrv (Premium)"
        File = "gdrv.sys"
        CVE = "CVE-2018-19320"
        Source = "Gigabyte App Center / RGB Fusion"
        URL = "https://www.gigabyte.com/Support/Utility"
        Risk = "BAJO - Muy compatible"
        Color = "Green"
    },
    @{
        Name = "MSI MSIo (Premium)"
        File = "msio64.sys"
        CVE = "CVE-2019-16098"
        Source = "MSI Dragon Center / Afterburner"
        URL = "https://www.msi.com/Landing/afterburner"
        Risk = "BAJO - Excelente con anti-cheats"
        Color = "Green"  
    },
    @{
        Name = "MSI RTCore (Standard)"
        File = "RTCore64.sys"
        CVE = "CVE-2019-16098"
        Source = "MSI Afterburner"
        URL = "https://www.msi.com/Landing/afterburner"
        Risk = "MEDIO - Bien soportado"
        Color = "Yellow"
    },
    @{
        Name = "CPU-Z (Standard)"
        File = "cpuz159_x64.sys"
        CVE = "CVE-2017-15302"
        Source = "CPU-Z Official"
        URL = "https://www.cpuid.com/softwares/cpu-z.html"
        Risk = "MEDIO - Ampliamente disponible"
        Color = "Yellow"
    },
    @{
        Name = "HWiNFO64 (Standard)"
        File = "HWiNFO_x64_205.sys"
        CVE = "CVE-2018-8960"
        Source = "HWiNFO Official"
        URL = "https://www.hwinfo.com/download/"
        Risk = "MEDIO - Estable"
        Color = "Yellow"
    },
    @{
        Name = "Process Hacker (High-Risk)"
        File = "kprocesshacker.sys"
        CVE = "CVE-2020-13833"
        Source = "Process Hacker Official"
        URL = "https://processhacker.sourceforge.io/"
        Risk = "ALTO - Fácilmente detectado"
        Color = "Red"
    }
)

$index = 1
foreach ($driver in $drivers) {
    Write-Host "[$index] $($driver.Name)" -ForegroundColor White
    Write-Host "    Archivo: $($driver.File)" -ForegroundColor Gray
    Write-Host "    CVE: $($driver.CVE)" -ForegroundColor Gray
    Write-Host "    Fuente: $($driver.Source)" -ForegroundColor Gray
    Write-Host "    URL: $($driver.URL)" -ForegroundColor Blue
    Write-Host "    Riesgo: $($driver.Risk)" -ForegroundColor $driver.Color
    Write-Host ""
    $index++
}

Write-Host "=== INSTRUCCIONES DE OBTENCIÓN LEGAL ===" -ForegroundColor White
Write-Host ""
Write-Host "1. DESCARGA OFICIAL: Descarga el software de la URL oficial proporcionada" -ForegroundColor Green
Write-Host "2. INSTALACIÓN: Instala el software normalmente" -ForegroundColor Green  
Write-Host "3. EXTRACCIÓN: El driver .sys se instalará en System32/drivers o en la carpeta de instalación" -ForegroundColor Green
Write-Host "4. COPIA: Copia el archivo .sys a la carpeta 'drivers' de este proyecto" -ForegroundColor Green
Write-Host ""

Write-Host "=== RUTAS COMUNES DE EXTRACCIÓN ===" -ForegroundColor White
Write-Host ""
Write-Host "• Gigabyte (gdrv.sys):" -ForegroundColor Cyan
Write-Host "  C:\Windows\System32\drivers\" -ForegroundColor Gray
Write-Host "  C:\Program Files (x86)\GIGABYTE\AppCenter\" -ForegroundColor Gray
Write-Host ""
Write-Host "• MSI Afterburner (RTCore64.sys, msio64.sys):" -ForegroundColor Cyan
Write-Host "  C:\Program Files (x86)\MSI Afterburner\" -ForegroundColor Gray
Write-Host "  C:\Windows\System32\drivers\" -ForegroundColor Gray
Write-Host ""
Write-Host "• CPU-Z (cpuz159_x64.sys):" -ForegroundColor Cyan
Write-Host "  C:\Program Files\CPUID\CPU-Z\" -ForegroundColor Gray
Write-Host ""
Write-Host "• HWiNFO64 (HWiNFO_x64_205.sys):" -ForegroundColor Cyan
Write-Host "  C:\Program Files\HWiNFO64\" -ForegroundColor Gray
Write-Host ""

Write-Host "=== SCRIPT DE BÚSQUEDA AUTOMÁTICA ===" -ForegroundColor White
Write-Host ""
$autoSearch = Read-Host "¿Deseas buscar automáticamente drivers ya instalados en tu sistema? (s/n)"

if ($autoSearch -eq "s" -or $autoSearch -eq "S") {
    Write-Host ""
    Write-Host "[SCANNING] Buscando drivers en ubicaciones comunes..." -ForegroundColor Yellow
    Write-Host ""
    
    $foundDrivers = @()
    
    # Definir rutas de búsqueda
    $searchPaths = @(
        "C:\Windows\System32\drivers",
        "C:\Program Files (x86)\MSI Afterburner",
        "C:\Program Files (x86)\GIGABYTE",
        "C:\Program Files\CPUID\CPU-Z",
        "C:\Program Files\HWiNFO64",
        "C:\Program Files (x86)\Process Hacker 2"
    )
    
    foreach ($path in $searchPaths) {
        if (Test-Path $path) {
            foreach ($driver in $drivers) {
                $driverPath = Join-Path $path $driver.File
                if (Test-Path $driverPath) {
                    $foundDrivers += @{
                        Name = $driver.Name
                        File = $driver.File
                        Path = $driverPath
                        Size = (Get-Item $driverPath).Length
                    }
                    Write-Host "[FOUND] $($driver.Name): $driverPath" -ForegroundColor Green
                }
            }
        }
    }
    
    if ($foundDrivers.Count -eq 0) {
        Write-Host "[INFO] No se encontraron drivers en ubicaciones comunes." -ForegroundColor Yellow
        Write-Host "[INFO] Instala primero el software oficial correspondiente." -ForegroundColor Yellow
    } else {
        Write-Host ""
        $copyDrivers = Read-Host "¿Deseas copiar los drivers encontrados a la carpeta 'drivers'? (s/n)"
        
        if ($copyDrivers -eq "s" -or $copyDrivers -eq "S") {
            foreach ($found in $foundDrivers) {
                $destPath = Join-Path "drivers" $found.File
                try {
                    Copy-Item $found.Path $destPath -Force
                    Write-Host "[COPIED] $($found.File) -> drivers\" -ForegroundColor Green
                } catch {
                    Write-Host "[ERROR] No se pudo copiar $($found.File): $($_.Exception.Message)" -ForegroundColor Red
                }
            }
        }
    }
}

Write-Host ""
Write-Host "=== ESTADO ACTUAL ===" -ForegroundColor White
$driverFiles = Get-ChildItem "drivers\*.sys" -ErrorAction SilentlyContinue
if ($driverFiles) {
    Write-Host ""
    Write-Host "Drivers disponibles en 'drivers\':" -ForegroundColor Green
    foreach ($file in $driverFiles) {
        $size = [math]::Round($file.Length / 1KB, 2)
        Write-Host " ✓ $($file.Name) ($size KB)" -ForegroundColor Green
    }
} else {
    Write-Host ""
    Write-Host "No hay drivers .sys en la carpeta 'drivers'." -ForegroundColor Red
    Write-Host "Necesitas obtener al menos un driver para usar el sistema BYOVD." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== PRÓXIMOS PASOS ===" -ForegroundColor White
Write-Host ""
Write-Host "1. Obtén drivers de fuentes oficiales mostradas arriba" -ForegroundColor Cyan
Write-Host "2. Ejecuta 'build.bat' para compilar el proyecto" -ForegroundColor Cyan
Write-Host "3. Ejecuta 'build\BYOVD_Professional_v3.exe' como administrador" -ForegroundColor Cyan
Write-Host "4. Accede a http://localhost:12345 para usar la interfaz web" -ForegroundColor Cyan
Write-Host ""

Write-Host "⚠️  RECORDATORIO LEGAL:" -ForegroundColor Red
Write-Host "• Solo usa este software en sistemas que poseas completamente" -ForegroundColor Yellow
Write-Host "• Solo descarga drivers de fuentes oficiales y legítimas" -ForegroundColor Yellow
Write-Host "• Este proyecto es exclusivamente para fines educativos" -ForegroundColor Yellow
Write-Host ""

Read-Host "Presiona Enter para salir"
