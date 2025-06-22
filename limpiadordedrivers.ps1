# =============================================================================
#  Script de Limpieza de Servicios de Drivers Huérfanos para Belzebub
#  Encuentra, detiene y elimina cualquier servicio asociado a los drivers
#  conocidos, solucionando problemas de carga.
#  IMPORTANTE: Ejecutar como Administrador.
# =============================================================================

$driverFiles = @(
    "gdrv.sys",
    "msio64.sys",
    "AsIO3.sys",
    "RTCore64.sys",
    "cpuz159_x64.sys",
    "kprocesshacker.sys"
)

Write-Host "================================================" -ForegroundColor Cyan
Write-Host " Limpiador de Servicios de Drivers de Belzebub" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host

foreach ($driverFile in $driverFiles) {
    Write-Host "[BUSCANDO] Servicios para el driver '$driverFile'..." -ForegroundColor Yellow
    
    # Usamos 'sc.exe queryex' para encontrar servicios que usen este archivo de driver
    $services = sc.exe queryex type=driver state=all bufsize=8192 | findstr /I /C:"BINARY_PATH_NAME: .*$driverFile"
    
    if ($services) {
        foreach ($serviceLine in $services) {
            # Extraer el nombre del servicio
            $serviceName = ($serviceLine -split ":")[1].Trim()
            
            Write-Host "[ENCONTRADO] Servicio '$serviceName' asociado a '$driverFile'." -ForegroundColor Green
            
            # Intentar detener el servicio
            Write-Host "  -> Intentando detener el servicio '$serviceName'..."
            sc.exe stop $serviceName | Out-Null
            Start-Sleep -Seconds 1 # Dar tiempo a que se detenga
            
            # Intentar eliminar el servicio
            Write-Host "  -> Intentando eliminar el servicio '$serviceName'..."
            sc.exe delete $serviceName
        }
    } else {
        Write-Host "  -> No se encontraron servicios activos para '$driverFile'." -ForegroundColor Gray
    }
    Write-Host
}

Write-Host "================================================" -ForegroundColor Green
Write-Host " Limpieza completada."
Write-Host " Intenta ejecutar Belzebub de nuevo."
Write-Host " Si el problema persiste, reinicia tu PC para forzar la liberación de todos los drivers."
Write-Host "================================================" -ForegroundColor Green
Read-Host "Presiona Enter para salir."