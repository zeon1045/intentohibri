#include "injection_engine.h"
#include "privilege_manager.h"
#include "utils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <regex>
#include <map>
#include <vector>

#pragma comment(lib, "psapi.lib")

// Helper function
static std::string GenerateRandomString(int length) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, static_cast<int>(chars.size() - 1));
    std::string random_string;
    for (int i = 0; i < length; ++i) {
        random_string += chars[distribution(generator)];
    }
    return random_string;
}

// Constructor
InjectionEngine::InjectionEngine() : currentDriver(nullptr), driverLoaded(false), hasLoadDriverPrivilege(false), hasDebugPrivilege(false), targetProcessId(0) {
    // La inicialización ahora escanea la carpeta de drivers
    InitializeDriverDatabase(); 
    
    // Inicializar privilegios del sistema UNA SOLA VEZ
    std::cout << "[BELZEBUB] Inicializando motor de inyección..." << std::endl;
    bool privilegesOk = InitializeBelzebubPrivileges();
    
    // --- ASIGNACIÓN DIRECTA DE PRIVILEGIOS ---
    if (IsUserAdmin()) {
        this->hasLoadDriverPrivilege = SetPrivilege(L"SeLoadDriverPrivilege", TRUE);
        this->hasDebugPrivilege = SetPrivilege(L"SeDebugPrivilege", TRUE);
        
        std::cout << "[BELZEBUB] ✅ Asignación directa de privilegios completada" << std::endl;
        std::cout << "[BELZEBUB] ✅ SeLoadDriverPrivilege: " << (this->hasLoadDriverPrivilege ? "ACTIVO" : "FALLO") << std::endl;
        std::cout << "[BELZEBUB] ✅ SeDebugPrivilege: " << (this->hasDebugPrivilege ? "ACTIVO" : "FALLO") << std::endl;
    } else {
        this->hasLoadDriverPrivilege = false;
        this->hasDebugPrivilege = false;
        std::cout << "[BELZEBUB] ⚠️  No se ejecuta como administrador - privilegios limitados" << std::endl;
    }
    
    if (this->hasLoadDriverPrivilege && this->hasDebugPrivilege) {
        std::cout << "[BELZEBUB] ✅ Motor inicializado con privilegios completos" << std::endl;
    } else {
        std::cout << "[BELZEBUB] ⚠️  Motor inicializado con privilegios limitados" << std::endl;
    }
}

InjectionEngine::~InjectionEngine() {
    if (IsDriverLoaded()) {
        UnloadDriver();
    }
}

// Escanea la carpeta /drivers y puebla la base de datos de drivers dinámicamente
void InjectionEngine::InitializeDriverDatabase() {
    std::cout << "[DB] Escaneando dinámicamente la carpeta de drivers..." << std::endl;
    knownDrivers.clear(); // Limpiamos la lista por si se llama más de una vez
    
    std::string drivers_dir = get_executable_directory() + "/drivers/";
    int current_id = 0;
    
    const int os_arch = 64; 

    if (!std::filesystem::exists(drivers_dir)) {
        std::cout << "[DB] ⚠️  Carpeta drivers no encontrada. Creando: " << drivers_dir << std::endl;
        std::filesystem::create_directory(drivers_dir);
        std::cout << "[DB] ✅ Carpeta drivers creada. Coloca archivos .sys en esta carpeta." << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(drivers_dir)) {
        if (entry.path().extension() == ".sys") {
            std::string file_path = entry.path().string();
            std::string filename = entry.path().filename().string();
            
            int driver_arch = get_file_architecture(file_path);
            
            if (driver_arch == os_arch) {
                DriverInfo info; // Usando la estructura local
                info.name = filename;
                info.filename = filename;
                info.architecture = driver_arch;
                // Los demás campos pueden dejarse por defecto o llenarse si es necesario
                knownDrivers[current_id++] = info;
                std::cout << "[DB] Driver compatible encontrado: " << filename << " (x" << driver_arch << ")" << std::endl;
            } else {
                std::cout << "[DB] Driver omitido (incompatible): " << filename << " (x" << driver_arch << ")" << std::endl;
            }
        }
    }
    std::cout << "[DB] Escaneo finalizado. " << knownDrivers.size() << " drivers compatibles cargados." << std::endl;
}

// Intenta detener y eliminar un servicio existente por su nombre.
bool InjectionEngine::CleanupService(const std::string& serviceName) {
    std::cout << "[CLEAN] Intentando limpiar servicio previo: " << serviceName << std::endl;
    SC_HANDLE scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scManager) return false;

    SC_HANDLE service = OpenServiceA(scManager, serviceName.c_str(), SERVICE_STOP | DELETE | SERVICE_QUERY_STATUS);
    if (!service) {
        CloseServiceHandle(scManager);
        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) {
            std::cout << "[CLEAN] El servicio no existe, no se necesita limpieza." << std::endl;
            return true;
        }
        return false;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
        if (ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING) {
            std::cout << "[CLEAN] Deteniendo servicio..." << std::endl;
            ControlService(service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
            Sleep(1000); 
        }
    }

    std::cout << "[CLEAN] Eliminando servicio..." << std::endl;
    bool success = DeleteService(service);
    if (!success) {
        std::cerr << "[CLEAN] Error al eliminar el servicio. Código: " << GetLastError() << std::endl;
    } else {
        std::cout << "[CLEAN] Servicio eliminado exitosamente." << std::endl;
        Sleep(500);
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scManager);
    return success;
}

json InjectionEngine::ListAvailableDrivers() {
    json result = json::array();
    for (const auto& pair : knownDrivers) {
        json driver_info;
        driver_info["id"] = pair.first;
        driver_info["name"] = pair.second.name;
        driver_info["filename"] = pair.second.filename;
        driver_info["architecture"] = pair.second.architecture;
        
        std::string full_path = get_executable_directory() + "/drivers/" + pair.second.filename;
        driver_info["status"] = std::filesystem::exists(full_path) ? "Disponible" : "No Encontrado";
        
        result.push_back(driver_info);
    }
    return result;
}

json InjectionEngine::LoadDriver(int driverIndex) {
    std::cout << "\n[LOAD] --- Iniciando Proceso de Carga de Driver ---" << std::endl;
    if (IsDriverLoaded()) {
        return {{"success", false}, {"message", "Ya hay un driver cargado. Descárgalo primero."}};
    }

    auto it = knownDrivers.find(driverIndex);
    if (it == knownDrivers.end()) {
        return {{"success", false}, {"message", "Índice de driver no válido."}};
    }
    currentDriver = &it->second;

    std::cout << "[LOAD] Driver seleccionado: " << currentDriver->name << " (" << currentDriver->filename << ")" << std::endl;

    if (!this->hasLoadDriverPrivilege) {
        std::cerr << "[LOAD] Error: Privilegios SeLoadDriverPrivilege no disponibles." << std::endl;
        return {{"success", false}, {"message", "Error: El privilegio SeLoadDriverPrivilege no está activo."}};
    }

    std::string baseServiceName = currentDriver->filename;
    std::replace(baseServiceName.begin(), baseServiceName.end(), '.', '_');
    CleanupService(baseServiceName);
    
    currentServiceName = "BEL_" + baseServiceName.substr(0, 8) + "_" + GenerateRandomString(6);
    std::cout << "[CLEAN] Usando nombre de servicio único: " << currentServiceName << std::endl;

    std::filesystem::path driverPath = std::filesystem::path(get_executable_directory()) / "drivers" / currentDriver->filename;
    std::cout << "[LOAD] Ruta absoluta del driver: " << driverPath.string() << std::endl;
    
    if (!std::filesystem::exists(driverPath)) {
        return {{"success", false}, {"message", "El archivo del driver no se encuentra."}};
    }

    SC_HANDLE scManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scManager) {
        return {{"success", false}, {"message", "Error al abrir SCM: " + std::to_string(GetLastError())}};
    }

    SC_HANDLE service = CreateServiceA(
        scManager, currentServiceName.c_str(), currentServiceName.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        driverPath.string().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!service) {
        DWORD lastError = GetLastError();
        CloseServiceHandle(scManager);
        return {{"success", false}, {"message", "Error al crear el servicio: " + std::to_string(lastError)}};
    }

    if (!StartService(service, 0, nullptr)) {
        DWORD lastError = GetLastError();
        DeleteService(service);
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return {{"success", false}, {"message", "Error al iniciar el servicio: " + std::to_string(lastError)}};
    }

    driverLoaded = true;
    CloseServiceHandle(service);
    CloseServiceHandle(scManager);
    std::cout << "[LOAD] --- Proceso de Carga de Driver FINALIZADO con ÉXITO ---" << std::endl;
    return {{"success", true}, {"message", "Driver '" + currentDriver->name + "' cargado correctamente."}};
}

json InjectionEngine::UnloadDriver() {
    if (!IsDriverLoaded()) {
        return {{"success", false}, {"message", "No hay driver cargado actualmente."}};
    }

    SC_HANDLE scManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scManager) {
        return {{"success", false}, {"message", "No se pudo conectar al SCM."}};
    }

    SC_HANDLE service = OpenServiceA(scManager, currentServiceName.c_str(), SERVICE_STOP | DELETE);
    if (service) {
        SERVICE_STATUS serviceStatus;
        ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus);
        Sleep(1000);
        DeleteService(service);
        CloseServiceHandle(service);
    }

    CloseServiceHandle(scManager);
    
    driverLoaded = false;
    currentDriver = nullptr;
    currentServiceName.clear();
    
    return {{"success", true}, {"message", "Driver descargado correctamente."}};
}

json InjectionEngine::GetSystemStatus() {
    json status;
    status["runningAsAdmin"] = IsUserAdmin() ? 1 : 0;
    
    this->hasLoadDriverPrivilege = IsPrivilegeEnabled(L"SeLoadDriverPrivilege");
    this->hasDebugPrivilege = IsPrivilegeEnabled(L"SeDebugPrivilege");
    
    status["loadDriverPrivilege"] = this->hasLoadDriverPrivilege ? 1 : 0;
    status["debugPrivilege"] = this->hasDebugPrivilege ? 1 : 0;
    status["driverLoaded"] = IsDriverLoaded();
    status["driverCount"] = knownDrivers.size();
    status["currentDriver"] = GetCurrentDriverName();
    status["serviceName"] = IsDriverLoaded() ? currentServiceName : "N/A";
    
    return status;
}

json InjectionEngine::GetProcessList() {
    json processList = json::array();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "No se pudo crear snapshot"}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            json procInfo;
            procInfo["pid"] = pe32.th32ProcessID;
            procInfo["name"] = pe32.szExeFile;
            processList.push_back(procInfo);
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return {{"success", true}, {"processes", processList}};
}

json InjectionEngine::GetCheatTableEntries(const std::string& ctFilePath) {
    if (ctFilePath.empty() || !std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "Ruta de archivo .CT no valida."}};
    }
    
    CTLoader::CTParser parser;
    if (!parser.loadFromFile(ctFilePath)) {
        return {{"success", false}, {"message", "Error al cargar el archivo .CT."}};
    }

    CTLoader::CheatTable& table = parser.getTable();
    cheatTableCache[ctFilePath] = table;
    
    json j_entries = json::array();
    for (size_t i = 0; i < table.entries.size(); i++) {
        const auto& entry = table.entries[i];
        j_entries.push_back({
            {"id", i},
            {"description", entry.description},
            {"type", entry.type},
            {"address", entry.address},
            {"value", entry.value}
        });
    }

    std::cout << "[CT] Archivo cargado exitosamente. " << table.entries.size() << " entradas encontradas." << std::endl;
    return {{"success", true}, {"entries", j_entries}};
}

json InjectionEngine::GetCheatTableEntriesFromContent(const std::string& ctContent) {
    if (ctContent.empty()) {
        return {{"success", false}, {"message", "El contenido de la tabla .CT esta vacio."}};
    }
    
    CTLoader::CTParser parser;
    if (!parser.loadFromString(ctContent)) {
        return {{"success", false}, {"message", "Error al parsear el contenido .CT."}};
    }

    CTLoader::CheatTable& table = parser.getTable();
    
    json j_entries = json::array();
    for (size_t i = 0; i < table.entries.size(); i++) {
        const auto& entry = table.entries[i];
        j_entries.push_back({
            {"id", i},
            {"description", entry.description},
            {"type", entry.type},
            {"address", entry.address},
            {"value", entry.value}
        });
    }

    std::cout << "[CT] Parseo de contenido exitoso. " << table.entries.size() << " entradas encontradas." << std::endl;
    return {{"success", true}, {"entries", j_entries}};
}

json InjectionEngine::ActivateCheatEntry(const std::string& ctFilePath, int entryId, bool activate) {
    if (targetProcessId == 0) {
        return {{"success", false}, {"message", "No hay un proceso objetivo seleccionado."}};
    }
    if (cheatTableCache.find(ctFilePath) == cheatTableCache.end()) {
        return {{"success", false}, {"message", "La tabla de cheats no esta en cache."}};
    }

    auto& table = cheatTableCache[ctFilePath];
    if (entryId < 0 || entryId >= static_cast<int>(table.entries.size())) {
        return {{"success", false}, {"message", "ID de entrada no válido."}};
    }

    CTLoader::MemoryEntry& targetEntry = table.entries[entryId];

    if (targetEntry.type != "Auto Assembler Script") {
        std::cout << "Activando entrada de tipo simple: " << targetEntry.description << std::endl;
        return {{"success", false}, {"message", "La escritura de memoria para tipos simples aun no esta implementada."}};
    } else {
        std::cout << "Activando entrada de Auto-Ensamblador: " << targetEntry.description << std::endl;
        return {{"success", false}, {"message", "La ejecucion de scripts de Auto-Ensamblador no esta implementada."}};
    }
    
    targetEntry.enabled = activate;
    return {{"success", true}, {"message", "Estado de la entrada actualizado (simulado)."}};
}

void InjectionEngine::SelectProcess(DWORD pid) {
    this->targetProcessId = pid;
    if (pid > 0) {
        std::cout << "[PID] Proceso objetivo establecido en: " << pid << std::endl;
    } else {
        std::cout << "[PID] Proceso objetivo deseleccionado." << std::endl;
    }
}