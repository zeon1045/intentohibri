# Script para ejecutar Belzebub como administrador
$exePath = ".\Belzebub.exe"

# Verificar si ya se está ejecutando como administrador
$currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
$isAdmin = $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    Write-Host "Ya ejecutándose como administrador. Iniciando Belzebub..."
    & $exePath
} else {
    Write-Host "Elevando privilegios a administrador..."
    Start-Process -FilePath "powershell.exe" -ArgumentList "-Command", "cd '$PWD'; .\Belzebub.exe; Read-Host 'Presiona Enter para salir'" -Verb RunAs
} 