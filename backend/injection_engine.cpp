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

// Constructor
InjectionEngine::InjectionEngine() : currentDriver(nullptr), driverLoaded(false), hasLoadDriverPrivilege(false), hasDebugPrivilege(false) {
    // La inicialización ahora escanea la carpeta de drivers
    InitializeDriverDatabase(); 
    
    // Inicializar privilegios del sistema
    std::cout << "[BELZEBUB] Inicializando motor de inyección..." << std::endl;
    bool privilegesOk = InitializeBelzebubPrivileges();
    
    if (privilegesOk) {
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
    
    // Asumimos que nuestro SO es de 64 bits, que es lo estándar
    const int os_arch = 64; 

    // Verificar que la carpeta drivers existe
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
            
            // Solo añadimos drivers que sean compatibles con nuestro SO de 64 bits
            if (driver_arch == os_arch) {
                DriverInfo info;
                info.name = filename; // El nombre ahora es el nombre del archivo
                info.filename = filename;
                info.architecture = driver_arch;
                info.tier = "Auto-Detectado";
                info.cve = "N/A";
                info.source = "Escaneado";
                info.description = "Driver detectado automáticamente en carpeta local";
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
        // Si no existe, no es un error, la limpieza no es necesaria.
        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) {
            std::cout << "[CLEAN] El servicio no existe, no se necesita limpieza." << std::endl;
            return true;
        }
        return false;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    // Comprobar si el servicio se está ejecutando
    if (QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
        if (ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING) {
            std::cout << "[CLEAN] Deteniendo servicio..." << std::endl;
            ControlService(service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
            // Darle un momento para que se detenga
            Sleep(1000); 
        }
    }

    std::cout << "[CLEAN] Eliminando servicio..." << std::endl;
    bool success = DeleteService(service);
    if (!success) {
        std::cerr << "[CLEAN] Error al eliminar el servicio. Código: " << GetLastError() << std::endl;
    } else {
        std::cout << "[CLEAN] Servicio eliminado exitosamente." << std::endl;
        Sleep(500); // Esperar un poco después de eliminar
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scManager);
    return success;
}

std::string InjectionEngine::GenerateRandomString(int length) {
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

json InjectionEngine::ListAvailableDrivers() {
    json result = json::array();
    for (const auto& pair : knownDrivers) {
        json driver_info = pair.second;
        driver_info["id"] = pair.first;
        // Verificar si el archivo realmente existe
        std::string full_path = get_executable_directory() + "/drivers/" + pair.second.filename;
        if (std::filesystem::exists(full_path)) {
            driver_info["status"] = "Disponible";
        } else {
            driver_info["status"] = "No Encontrado";
        }
        result.push_back(driver_info);
    }
    return result;
}

// Carga un driver basado en su índice
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

    // --- VERIFICACIÓN DE PRIVILEGIOS PERSISTENTES ---
    if (!this->hasLoadDriverPrivilege) {
        std::cerr << "[LOAD] Error: Privilegios SeLoadDriverPrivilege no disponibles." << std::endl;
        return {{"success", false}, {"message", "Error: El privilegio SeLoadDriverPrivilege no está activo. Refresca el estado del sistema o reinicia como administrador."}};
    }

    // --- MEJORA: LIMPIEZA ROBUSTA ANTES DE CARGAR ---
    std::string baseServiceName = currentDriver->filename;
    std::cout << "[CLEAN] Realizando limpieza preventiva de servicios existentes..." << std::endl;
    
    // Intentar limpiar tanto el nombre base como posibles variaciones
    CleanupService(baseServiceName);
    CleanupService(baseServiceName.substr(0, baseServiceName.find('.'))); // Sin extensión
    
    // Generar nombre único para el servicio
    currentServiceName = "BEL_" + baseServiceName.substr(0, 8) + "_" + GenerateRandomString(6);
    std::cout << "[CLEAN] Usando nombre de servicio único: " << currentServiceName << std::endl;

    std::cout << "[PRIV] ✅ Usando privilegios previamente obtenidos y activos." << std::endl;
    
    std::filesystem::path driverPath = std::filesystem::path(get_executable_directory()) / "drivers" / currentDriver->filename;
    std::cout << "[LOAD] Ruta absoluta y segura del driver: " << driverPath.string() << std::endl;
    
    if (!std::filesystem::exists(driverPath)) {
        return {{"success", false}, {"message", "El archivo del driver '" + driverPath.string() + "' no se encuentra."}};
    }

    std::cout << "[SCM] Abriendo el Service Control Manager..." << std::endl;
    SC_HANDLE scManager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scManager) {
        DWORD lastError = GetLastError();
        std::cerr << "[SCM] Error: No se pudo abrir el SCM. Codigo: " << lastError << std::endl;
        
        std::string errorMsg = "No se pudo conectar al Service Control Manager.\nError: " + std::to_string(lastError);
        if (lastError == ERROR_ACCESS_DENIED) {
            errorMsg += "\n💡 SOLUCIÓN: Ejecutar como 'Administrador' (clic derecho → Ejecutar como administrador)";
        }
        return {{"success", false}, {"message", errorMsg}};
    }

    std::cout << "[SCM] Creando el servicio '" << currentServiceName << "'..." << std::endl;
    
    SC_HANDLE service = CreateServiceA(
        scManager, currentServiceName.c_str(), currentServiceName.c_str(),
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        driverPath.string().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!service) {
        DWORD lastError = GetLastError();
        std::cerr << "[SCM] Error al crear el servicio del driver. Codigo: " << lastError << std::endl;
        CloseServiceHandle(scManager);
        
        std::string errorMsg = "Error al crear el servicio del driver.\nCódigo: " + std::to_string(lastError);
        if (lastError == ERROR_ACCESS_DENIED) {
            errorMsg += "\n💡 SOLUCIÓN: Ejecutar como 'Administrador'";
        } else if (lastError == ERROR_SERVICE_EXISTS) {
            errorMsg += "\n💡 SOLUCIÓN: El servicio ya existe. Reinicia el programa o usa otro driver.";
        }
        return {{"success", false}, {"message", errorMsg}};
    }

    std::cout << "[SCM] Iniciando el servicio..." << std::endl;
    if (!StartService(service, 0, nullptr)) {
        DWORD lastError = GetLastError();
        std::cerr << "[SCM] Error al iniciar el servicio del driver. Codigo: " << lastError << std::endl;
        
        // Limpieza: eliminar el servicio creado
        DeleteService(service);
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        
        std::string errorMsg = "No se pudo iniciar el servicio del driver.\nCódigo: " + std::to_string(lastError);
        if (lastError == ERROR_INVALID_IMAGE_HASH) {
            errorMsg += "\n💡 CAUSA: Firma digital del driver no válida o corrupta.";
        } else if (lastError == ERROR_FILE_NOT_FOUND) {
            errorMsg += "\n💡 CAUSA: Archivo del driver no encontrado en la ruta especificada.";
        } else if (lastError == ERROR_ACCESS_DENIED) {
            errorMsg += "\n💡 CAUSA: Permisos insuficientes o Test Signing deshabilitado.";
        } else if (lastError == ERROR_ALREADY_EXISTS || lastError == 183) {
            errorMsg = "El driver/servicio ya existe en el sistema.\n💡 INFORMACIÓN: Esto es normal y indica que el sistema funciona correctamente.";
            // En este caso, consideramos que es un éxito
            driverLoaded = true;
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            std::cout << "[LOAD] --- Proceso de Carga de Driver FINALIZADO con ÉXITO (Servicio Existente) ---" << std::endl;
            return {{"success", true}, {"message", "Driver '" + currentDriver->name + "' ya está disponible en el sistema."}};
        }
        
        return {{"success", false}, {"message", errorMsg}};
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
        return {{"success", false}, {"message", "No se pudo conectar al Service Control Manager."}};
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
    
    // Actualizar estado interno
    driverLoaded = false;
    currentDriver = nullptr;
    currentServiceName.clear();
    
    // Al descargar, podemos revocar el privilegio si queremos ser limpios
    SetLoadDriverPrivilege(FALSE);
    this->hasLoadDriverPrivilege = false;
    
    return {{"success", true}, {"message", "Driver descargado correctamente."}};
}

json InjectionEngine::GetDriverInfoJson(int driverIndex) {
    auto it = knownDrivers.find(driverIndex);
    if (it == knownDrivers.end()) {
        return {{"success", false}, {"message", "Índice de driver no válido."}};
    }
    return it->second;
}

size_t InjectionEngine::GetDriverCount() const {
    return knownDrivers.size();
}

bool InjectionEngine::IsDriverLoaded() const {
    return driverLoaded;
}

std::string InjectionEngine::GetCurrentDriverName() const {
    if (currentDriver) {
        return currentDriver->name;
    }
    return "Ninguno";
}

json InjectionEngine::GetSystemStatus() {
    json status;
    status["runningAsAdmin"] = IsUserAdmin() ? 1 : 0;
    
    // --- LÓGICA DE PRIVILEGIOS CENTRALIZADA Y PERSISTENTE ---
    // Intentamos obtener los privilegios UNA SOLA VEZ y guardamos el estado.
    // NO los desactivamos después de comprobar para mantenerlos activos.
    if (IsUserAdmin()) {
        this->hasLoadDriverPrivilege = SetLoadDriverPrivilege(TRUE);
        this->hasDebugPrivilege = SetPrivilege(L"SeDebugPrivilege", TRUE);
    } else {
        this->hasLoadDriverPrivilege = false;
        this->hasDebugPrivilege = false;
    }
    
    status["loadDriverPrivilege"] = this->hasLoadDriverPrivilege ? 1 : 0;
    status["debugPrivilege"] = this->hasDebugPrivilege ? 1 : 0;
    status["driverLoaded"] = IsDriverLoaded();
    status["driverCount"] = GetDriverCount();
    status["currentDriver"] = GetCurrentDriverName();
    status["serviceName"] = IsDriverLoaded() ? currentServiceName : "N/A";
    
    return status;
}

// === RESTO DE FUNCIONALIDADES (Procesos, Cheat Engine, etc.) ===

bool InjectionEngine::ExecuteLuaScript(const std::string& scriptContent, DWORD processId) {
    // Implementación básica para scripts Lua
    std::string tempScriptName = "temp_script_" + std::to_string(processId) + ".lua";
    std::ofstream scriptFile(tempScriptName);
    if (!scriptFile.is_open()) {
        return false;
    }
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Aquí iría la integración con CheatEngine o LuaJIT
    // Por ahora, solo simulamos la ejecución
    std::cout << "[LUA] Ejecutando script para proceso " << processId << std::endl;
    
    // Limpiar archivo temporal
    std::filesystem::remove(tempScriptName);
    return true;
}

json InjectionEngine::FindProcess(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "No se pudo crear snapshot de procesos"}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return {{"success", false}, {"message", "No se pudo enumerar procesos"}};
    }

    do {
        std::string currentProcessName = pe32.szExeFile;
        if (currentProcessName.find(processName) != std::string::npos) {
            CloseHandle(hSnapshot);
            return {{"success", true}, {"pid", pe32.th32ProcessID}, {"name", currentProcessName}};
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return {{"success", false}, {"message", "Proceso no encontrado"}};
}

json InjectionEngine::GetProcessList() {
    json processList = json::array();
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "No se pudo crear snapshot de procesos"}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            ProcessInfo procInfo;
            procInfo.pid = pe32.th32ProcessID;
            procInfo.name = pe32.szExeFile;
            procInfo.cpuUsage = 0.0; // Placeholder para uso de CPU
            
            processList.push_back(procInfo);
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return {{"success", true}, {"processes", processList}};
}

json InjectionEngine::InjectDLL(DWORD processId, const std::string& dllPath) {
    return {{"success", false}, {"message", "Funcionalidad de inyección DLL no implementada"}};
}

json InjectionEngine::SetSpeedhack(DWORD processId, float speed) {
    return {{"success", false}, {"message", "Funcionalidad de speedhack no implementada"}};
}

// Las funciones de parseo ahora son manejadas por ct_loader

json InjectionEngine::LoadCheatTable(const std::string& ctFilePath, DWORD processId) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no se encuentra en la ruta especificada."}};
    }
    
    std::cout << "[CT] Iniciando carga de tabla: " << ctFilePath << std::endl;
    
    // Usar ct_loader para cargar la tabla
    CTLoader::CTParser parser;
    
    if (!parser.loadFromFile(ctFilePath)) {
        return {{"success", false}, {"message", "Error al leer el archivo .CT."}};
    }
    
    if (!parser.parseXML()) {
        return {{"success", false}, {"message", "Error al parsear el archivo .CT: Formato no válido."}};
    }
    
    // Obtener la tabla parseada
    CTLoader::CheatTable table = parser.getTable();
    
    if (table.entries.empty()) {
        return {{"success", false}, {"message", "No se encontraron entradas válidas en el archivo .CT."}};
    }
    
    // Guardar en caché
    cheatTableCache[ctFilePath] = table;
    
    std::cout << "[CT] Tabla de cheats cargada exitosamente: " << table.entries.size() << " entradas." << std::endl;
    std::cout << "[CT] Título: " << table.title << std::endl;
    std::cout << "[CT] Juego: " << table.game_name << std::endl;
    
    return {
        {"success", true}, 
        {"message", "Tabla de cheats cargada correctamente."}, 
        {"entries", table.entries.size()},
        {"title", table.title},
        {"game", table.game_name}
    };
}

json InjectionEngine::GetCheatTableEntries(const std::string& ctFilePath) {
    if (ctFilePath.empty() || !std::filesystem::exists(ctFilePath)) {
        std::cerr << "[CT] Error: La ruta del archivo .CT es inválida o no existe: " << ctFilePath << std::endl;
        return {{"success", false}, {"message", "Ruta de archivo .CT no válida."}};
    }
    
    // Si no está en caché, cargar usando ct_loader
    if (cheatTableCache.find(ctFilePath) == cheatTableCache.end()) {
        std::cout << "[CT] Cargando tabla desde archivo: " << ctFilePath << std::endl;
        
        // 1. Instanciar el parser de ct_loader
        CTLoader::CTParser parser;
        
        // 2. Cargar el archivo
        if (!parser.loadFromFile(ctFilePath)) {
            std::cerr << "[CT] Error: No se pudo cargar el archivo." << std::endl;
            return {{"success", false}, {"message", "Error al cargar el archivo .CT."}};
        }
        
        // 3. Parsear el XML
        if (!parser.parseXML()) {
            std::cerr << "[CT] Error: Formato XML no válido o corrupto." << std::endl;
            return {{"success", false}, {"message", "Error al parsear la tabla: Formato no compatible o archivo corrupto."}};
        }
        
        // 4. Obtener la tabla parseada y guardar en caché
        cheatTableCache[ctFilePath] = parser.getTable();
        std::cout << "[CT] Tabla cargada exitosamente en caché." << std::endl;
    }
    
    // 5. Convertir las entradas a formato JSON para el frontend
    auto& table = cheatTableCache[ctFilePath];
    json j_entries = json::array();
    
    int entryId = 1;
    for (const auto& entry : table.entries) {
        j_entries.push_back({
            {"id", entryId++},
            {"description", entry.description},
            {"type", entry.type},
            {"address", entry.address},
            {"value", entry.value},
            {"enabled", entry.enabled}
        });
    }
    
    std::cout << "[CT] Parseo con ct_loader exitoso. " << table.entries.size() << " entradas encontradas." << std::endl;
    return {{"success", true}, {"entries", j_entries}};
}

json InjectionEngine::ControlCheatEntry(const std::string& ctFilePath, DWORD processId, int entryId, bool activate) {
    if (cheatTableCache.find(ctFilePath) == cheatTableCache.end()) {
        return {{"success", false}, {"message", "La tabla de cheats no está cargada. Cárgala primero."}};
    }

    auto& table = cheatTableCache[ctFilePath];
    
    // Buscar la entrada por ID (índice basado en 1)
    if (entryId < 1 || entryId > static_cast<int>(table.entries.size())) {
        return {{"success", false}, {"message", "ID de entrada " + std::to_string(entryId) + " fuera de rango."}};
    }
    
    // Acceder a la entrada (convertir a índice basado en 0)
    CTLoader::MemoryEntry& targetEntry = table.entries[entryId - 1];
    
    // Actualizar estado
    targetEntry.enabled = activate;
    
    // Aquí iría la lógica de escritura en memoria real usando CTExecutor
    // Por ahora, solo simulamos la activación
    std::cout << "[CT] " << (activate ? "Activando" : "Desactivando") 
              << " cheat: " << targetEntry.description << std::endl;
    std::cout << "[CT] Dirección: " << targetEntry.address << " | Tipo: " << targetEntry.type << " | Valor: " << targetEntry.value << std::endl;
    
    return {{"success", true}, {"message", "Cheat " + std::string(activate ? "activado" : "desactivado") + " correctamente."}};
}

json InjectionEngine::SetCheatEntryValue(const std::string& ctFilePath, DWORD processId, int entryId, const std::string& value) {
    if (cheatTableCache.find(ctFilePath) == cheatTableCache.end()) {
        return {{"success", false}, {"message", "La tabla de cheats no está cargada."}};
    }

    auto& table = cheatTableCache[ctFilePath];
    
    // Verificar rango del ID
    if (entryId < 1 || entryId > static_cast<int>(table.entries.size())) {
        return {{"success", false}, {"message", "ID de entrada " + std::to_string(entryId) + " fuera de rango."}};
    }
    
    // Acceder a la entrada (convertir a índice basado en 0)
    CTLoader::MemoryEntry& targetEntry = table.entries[entryId - 1];
    
    // Actualizar valor
    targetEntry.value = value;
    
    std::cout << "[CT] Valor actualizado para cheat: " << targetEntry.description << " = " << value << std::endl;
    std::cout << "[CT] Dirección: " << targetEntry.address << " | Tipo: " << targetEntry.type << std::endl;
    
    return {{"success", true}, {"message", "Valor actualizado correctamente."}};
}

json InjectionEngine::GetCheatScript(const std::string& ctFilePath, int entryId) {
    return {{"success", false}, {"message", "Funcionalidad de scripts no implementada aún."}};
}

json InjectionEngine::UpdateCheatScript(const std::string& ctFilePath, int entryId, const std::string& newScript) {
    return {{"success", false}, {"message", "Funcionalidad de scripts no implementada aún."}};
}