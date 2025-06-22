#include "injection_engine.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

// Constructor
InjectionEngine::InjectionEngine() : currentDriverIndex(-1) {
    InitializeDriverDatabase();
}

// Destructor
InjectionEngine::~InjectionEngine() {
    if (currentDriverIndex >= 0) {
        UnloadDriver();
    }
}

// Implementación de inyección usando Cheat Engine real
bool InjectionEngine::InjectDLL(DWORD processId, const std::string& dllPath) {
    std::cout << "[INJECTION] Iniciando inyección real con Cheat Engine...\n";
    
    std::string ce64Path = "core_dlls\\cheatengine-x86_64.exe";
    std::string ce32Path = "core_dlls\\cheatengine-i386.exe";
    
    if (!std::filesystem::exists(ce64Path) || !std::filesystem::exists(ce32Path)) {
        std::cerr << "[ERROR] Ejecutables de Cheat Engine no encontrados!\n";
        return false;
    }
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[ERROR] No se puede acceder al proceso ID: " << processId << std::endl;
        return false;
    }
    
    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess, &isWow64);
    bool isProcess32Bit = isWow64;
    CloseHandle(hProcess);
    
    std::string ceExecutable = isProcess32Bit ? ce32Path : ce64Path;
    std::cout << "[INFO] Usando " << (isProcess32Bit ? "CE 32-bit" : "CE 64-bit") << " para proceso " << processId << std::endl;
    
    std::stringstream cmd;
    cmd << "\"" << ceExecutable << "\" --inject-dll=" << processId << "=" << dllPath;
    
    std::cout << "[CMD] " << cmd.str() << std::endl;
    
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    
    bool success = CreateProcessA(
        NULL,
        const_cast<char*>(cmd.str().c_str()),
        NULL, NULL, FALSE,
        CREATE_NO_WINDOW,
        NULL, NULL, &si, &pi
    );
    
    if (success) {
        std::cout << "[SUCCESS] Proceso de inyección iniciado (PID: " << pi.dwProcessId << ")\n";
        
        WaitForSingleObject(pi.hProcess, 30000);
        
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        bool injectionSuccess = (exitCode == 0);
        if (injectionSuccess) {
            std::cout << "[SUCCESS] Inyección completada exitosamente!\n";
        } else {
            std::cout << "[WARNING] Cheat Engine terminó con código: " << exitCode << std::endl;
        }
        
        return injectionSuccess;
    } else {
        std::cerr << "[ERROR] No se pudo ejecutar Cheat Engine. Error: " << GetLastError() << std::endl;
        return false;
    }
}

// Scanner de memoria usando Cheat Engine
bool InjectionEngine::ScanMemory(DWORD processId, const std::string& value, const std::string& valueType) {
    std::cout << "[MEMORY_SCAN] Iniciando escaneo con Cheat Engine...\n";
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) return false;
    
    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess, &isWow64);
    bool isProcess32Bit = isWow64;
    CloseHandle(hProcess);
    
    std::string ceExecutable = isProcess32Bit ? 
        "core_dlls\\cheatengine-i386.exe" : 
        "core_dlls\\cheatengine-x86_64.exe";
    
    std::string scriptPath = "temp_scan_script.lua";
    std::ofstream script(scriptPath);
    script << "-- Script de escaneo automático\n";
    script << "openProcess('" << processId << "')\n";
    script << "memScan = createMemScan()\n";
    script << "memScan.firstScan('" << value << "', vtDword, '" << valueType << "')\n";
    script << "print('Escaneo completado: ' .. memScan.getCount() .. ' resultados')\n";
    script.close();
    
    std::stringstream cmd;
    cmd << "\"" << ceExecutable << "\" --script=" << scriptPath;
    
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    
    bool success = CreateProcessA(NULL, const_cast<char*>(cmd.str().c_str()), 
                                 NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                                 NULL, NULL, &si, &pi);
    
    if (success) {
        WaitForSingleObject(pi.hProcess, 60000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        std::filesystem::remove(scriptPath);
        
        std::cout << "[SUCCESS] Escaneo de memoria completado\n";
        return true;
    }
    
    std::filesystem::remove(scriptPath);
    return false;
}

// Búsqueda de proceso mejorada
DWORD InjectionEngine::FindProcess(const std::string& processName) {
    std::cout << "[PROCESS_SEARCH] Buscando proceso: " << processName << std::endl;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] No se pudo crear snapshot de procesos\n";
        return 0;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    std::vector<DWORD> matchingProcesses;
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string currentProcess = pe32.szExeFile;
            
            if (currentProcess == processName || 
                currentProcess.find(processName) != std::string::npos) {
                matchingProcesses.push_back(pe32.th32ProcessID);
                std::cout << "[FOUND] " << currentProcess << " (PID: " << pe32.th32ProcessID << ")\n";
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    if (matchingProcesses.empty()) {
        std::cout << "[INFO] No se encontraron procesos con nombre: " << processName << std::endl;
        return 0;
    } else if (matchingProcesses.size() == 1) {
        std::cout << "[SUCCESS] Proceso encontrado: PID " << matchingProcesses[0] << std::endl;
        return matchingProcesses[0];
    } else {
        std::cout << "[INFO] Múltiples procesos encontrados. Usando el primero: PID " << matchingProcesses[0] << std::endl;
        return matchingProcesses[0];
    }
}

// Inicialización de la base de datos de drivers
void InjectionEngine::InitializeDriverDatabase() {
    drivers.clear();
    
    drivers.push_back({
        "Gigabyte GDrv", "gdrv.sys", "Premium",
        "CVE-2018-19320", "Gigabyte App Center",
        "Excelente compatibilidad con todos los anti-cheats"
    });
    
    drivers.push_back({
        "MSI MSIo", "msio64.sys", "Premium", 
        "CVE-2019-16098", "MSI Afterburner/Dragon Center",
        "Alto éxito con BattlEye y EAC"
    });
    
    drivers.push_back({
        "ASUS AsIO", "AsIO3.sys", "Premium",
        "CVE-2020-15368", "ASUS Armoury Crate/AI Suite",
        "Compatible con Vanguard y VAC"
    });
    
    drivers.push_back({
        "MSI RTCore", "RTCore64.sys", "Standard",
        "CVE-2019-16098", "MSI Afterburner",
        "Funciona bien con la mayoría de juegos"
    });
    
    drivers.push_back({
        "CPU-Z", "cpuz159_x64.sys", "Standard",
        "CVE-2017-15302", "CPU-Z Official",
        "Driver estable y ampliamente disponible"
    });
    
    drivers.push_back({
        "HWiNFO", "HWiNFO_x64_205.sys", "Standard",
        "CVE-2018-8960", "HWiNFO Official",
        "Buena compatibilidad general"
    });
    
    drivers.push_back({
        "Cheat Engine DBK", "dbk64.sys", "High-Risk",
        "CVE-2019-12769", "Cheat Engine",
        "⚠️ ALTO RIESGO - Conocido por anti-cheats"
    });
    
    drivers.push_back({
        "Process Hacker", "kprocesshacker.sys", "High-Risk",
        "CVE-2020-13833", "Process Hacker",
        "⚠️ ALTO RIESGO - Detectado fácilmente"
    });
    
    std::cout << "[INIT] Base de datos de drivers inicializada: " << drivers.size() << " drivers disponibles\n";
}

// Lista drivers disponibles en la carpeta
std::vector<std::string> InjectionEngine::ListAvailableDrivers() {
    std::vector<std::string> availableDrivers;
    std::cout << "\n[DRIVER_SCAN] Escaneando carpeta 'drivers' para archivos .sys...\n";
    
    try {
        if (std::filesystem::exists("drivers") && std::filesystem::is_directory("drivers")) {
            for (const auto& entry : std::filesystem::directory_iterator("drivers")) {
                if (entry.is_regular_file() && entry.path().extension() == ".sys") {
                    std::string filename = entry.path().filename().string();
                    
                    std::string driverInfo = "Desconocido";
                    for (const auto& driver : drivers) {
                        if (driver.filename == filename) {
                            driverInfo = driver.name + " (" + driver.tier + ") - " + driver.description;
                            break;
                        }
                    }
                    
                    availableDrivers.push_back(filename + " - " + driverInfo);
                    std::cout << "[FOUND] " << filename << " - " << driverInfo << std::endl;
                }
            }
        } else {
            std::cout << "[WARNING] Carpeta 'drivers' no encontrada\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Error escaneando drivers: " << e.what() << std::endl;
    }
    
    if (availableDrivers.empty()) {
        std::cout << "[INFO] No se encontraron drivers .sys en la carpeta 'drivers'\n";
        std::cout << "[HELP] Coloca archivos .sys de drivers vulnerables en la carpeta 'drivers'\n";
    } else {
        std::cout << "[SUCCESS] " << availableDrivers.size() << " drivers encontrados\n";
    }
    
    return availableDrivers;
}

// Carga un driver específico
bool InjectionEngine::LoadDriver(int driverIndex) {
    if (driverIndex < 0 || driverIndex >= static_cast<int>(drivers.size())) {
        std::cerr << "[ERROR] Índice de driver inválido: " << driverIndex << std::endl;
        return false;
    }
    
    const DriverInfo& driver = drivers[driverIndex];
    std::string driverPath = "drivers\\" + driver.filename;
    
    std::cout << "[DRIVER_LOAD] Intentando cargar: " << driver.name << std::endl;
    std::cout << "[PATH] " << driverPath << std::endl;
    
    if (!std::filesystem::exists(driverPath)) {
        std::cerr << "[ERROR] Archivo de driver no encontrado: " << driverPath << std::endl;
        return false;
    }
    
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        std::cerr << "[ERROR] No se pudo abrir Service Control Manager. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    std::string serviceName = "BYOVD_" + driver.name;
    std::replace(serviceName.begin(), serviceName.end(), ' ', '_');
    
    char absolutePath[MAX_PATH];
    GetFullPathNameA(driverPath.c_str(), MAX_PATH, absolutePath, NULL);
    
    SC_HANDLE hService = CreateServiceA(
        hSCM,
        serviceName.c_str(),
        serviceName.c_str(),
        SERVICE_START | DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        absolutePath,
        NULL, NULL, NULL, NULL, NULL
    );
    
    if (!hService) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_EXISTS) {
            hService = OpenServiceA(hSCM, serviceName.c_str(), SERVICE_START | DELETE | SERVICE_STOP);
            if (!hService) {
                std::cerr << "[ERROR] No se pudo abrir servicio existente\n";
                CloseServiceHandle(hSCM);
                return false;
            }
        } else {
            std::cerr << "[ERROR] No se pudo crear servicio. Error: " << error << std::endl;
            CloseServiceHandle(hSCM);
            return false;
        }
    }
    
    bool startResult = StartService(hService, 0, NULL);
    if (!startResult) {
        DWORD error = GetLastError();
        if (error != ERROR_SERVICE_ALREADY_RUNNING) {
            std::cerr << "[ERROR] No se pudo iniciar driver. Error: " << error << std::endl;
            
            DeleteService(hService);
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return false;
        }
    }
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    currentDriverIndex = driverIndex;
    currentServiceName = serviceName;
    
    std::cout << "[SUCCESS] Driver cargado exitosamente: " << driver.name << std::endl;
    std::cout << "[INFO] Servicio: " << serviceName << std::endl;
    std::cout << "[WARNING] Tier: " << driver.tier << " - " << driver.description << std::endl;
    
    return true;
}

// Descarga el driver actual
bool InjectionEngine::UnloadDriver() {
    if (currentDriverIndex < 0 || currentServiceName.empty()) {
        std::cout << "[INFO] No hay driver cargado para descargar\n";
        return true;
    }
    
    std::cout << "[DRIVER_UNLOAD] Descargando driver: " << drivers[currentDriverIndex].name << std::endl;
    
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM) {
        std::cerr << "[ERROR] No se pudo abrir Service Control Manager\n";
        return false;
    }
    
    SC_HANDLE hService = OpenServiceA(hSCM, currentServiceName.c_str(), 
                                     SERVICE_STOP | DELETE | SERVICE_QUERY_STATUS);
    if (!hService) {
        std::cerr << "[ERROR] No se pudo abrir servicio: " << currentServiceName << std::endl;
        CloseServiceHandle(hSCM);
        return false;
    }
    
    SERVICE_STATUS status;
    ControlService(hService, SERVICE_CONTROL_STOP, &status);
    
    do {
        Sleep(500);
        QueryServiceStatus(hService, &status);
    } while (status.dwCurrentState == SERVICE_STOP_PENDING);
    
    bool deleteResult = DeleteService(hService);
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    if (deleteResult) {
        std::cout << "[SUCCESS] Driver descargado exitosamente\n";
        currentDriverIndex = -1;
        currentServiceName.clear();
        return true;
    } else {
        std::cerr << "[WARNING] Driver detenido pero no eliminado completamente\n";
        return false;
    }
}

// Obtiene información de un driver
std::string InjectionEngine::GetDriverInfo(int driverIndex) {
    if (driverIndex < 0 || driverIndex >= static_cast<int>(drivers.size())) {
        return "Índice de driver inválido";
    }
    
    const DriverInfo& driver = drivers[driverIndex];
    std::stringstream info;
    
    info << "=== " << driver.name << " ===\n";
    info << "Archivo: " << driver.filename << "\n";
    info << "Tier: " << driver.tier << "\n";
    info << "CVE: " << driver.cve << "\n";
    info << "Fuente: " << driver.source << "\n";
    info << "Descripción: " << driver.description << "\n";
    
    std::string driverPath = "drivers\\" + driver.filename;
    if (std::filesystem::exists(driverPath)) {
        auto fileSize = std::filesystem::file_size(driverPath);
        info << "Estado: ✅ DISPONIBLE (" << fileSize << " bytes)\n";
    } else {
        info << "Estado: ❌ NO ENCONTRADO\n";
    }
    
    return info.str();
}

// **🎯 NUEVA FUNCIÓN - Carga de Cheat Tables (.CT)**
bool InjectionEngine::LoadCheatTable(const std::string& ctFilePath, DWORD processId) {
    std::cout << "[CHEAT_TABLE] Cargando archivo .CT: " << ctFilePath << std::endl;
    
    // Verificar que el archivo .CT existe
    if (!std::filesystem::exists(ctFilePath)) {
        std::cerr << "[ERROR] Archivo .CT no encontrado: " << ctFilePath << std::endl;
        return false;
    }
    
    // Verificar que los ejecutables de CE existan
    std::string ce64Path = "core_dlls\\cheatengine-x86_64.exe";
    std::string ce32Path = "core_dlls\\cheatengine-i386.exe";
    
    if (!std::filesystem::exists(ce64Path) || !std::filesystem::exists(ce32Path)) {
        std::cerr << "[ERROR] Ejecutables de Cheat Engine no encontrados!\n";
        return false;
    }
    
    // Determinar arquitectura del proceso
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) {
        std::cerr << "[ERROR] No se puede acceder al proceso ID: " << processId << std::endl;
        return false;
    }
    
    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess, &isWow64);
    bool isProcess32Bit = isWow64;
    CloseHandle(hProcess);
    
    // Seleccionar ejecutable apropiado
    std::string ceExecutable = isProcess32Bit ? ce32Path : ce64Path;
    std::cout << "[INFO] Usando " << (isProcess32Bit ? "CE 32-bit" : "CE 64-bit") << " para cargar .CT" << std::endl;
    
    // Construir comando para cargar .CT en CE
    std::stringstream cmd;
    cmd << "\"" << ceExecutable << "\" \"" << ctFilePath << "\"";
    
    std::cout << "[CMD] " << cmd.str() << std::endl;
    
    // Ejecutar Cheat Engine con el archivo .CT
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    
    bool success = CreateProcessA(
        NULL,
        const_cast<char*>(cmd.str().c_str()),
        NULL, NULL, FALSE,
        0,  // Mostrar ventana para que el usuario pueda interactuar
        NULL, NULL, &si, &pi
    );
    
    if (success) {
        std::cout << "[SUCCESS] Cheat Engine iniciado con tabla: " << ctFilePath << std::endl;
        std::cout << "[INFO] CE se abrirá con el cheat table cargado" << std::endl;
        std::cout << "[INFO] El usuario puede activar/desactivar cheats desde la interfaz de CE" << std::endl;
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    } else {
        std::cerr << "[ERROR] No se pudo cargar el cheat table. Error: " << GetLastError() << std::endl;
        return false;
    }
}