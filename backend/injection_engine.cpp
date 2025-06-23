#include "injection_engine.h"
#include "privilege_manager.h"
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

// --- NUEVA FUNCIÓN PRIVADA ---
// Busca y elimina servicios existentes que usen la misma ruta de driver.
void CleanupExistingDriverServices(const std::string& driverPath) {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);
    if (!hSCM) return;

    ENUM_SERVICE_STATUS_PROCESS* services = nullptr;
    DWORD bytesNeeded = 0, servicesReturned = 0, resumeHandle = 0;

    EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle, NULL);

    if (GetLastError() == ERROR_MORE_DATA) {
        std::vector<BYTE> buffer(bytesNeeded);
        services = (ENUM_SERVICE_STATUS_PROCESS*)buffer.data();
        if (EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL, (LPBYTE)services, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle, NULL)) {
            for (DWORD i = 0; i < servicesReturned; ++i) {
                SC_HANDLE hService = OpenServiceA(hSCM, services[i].lpServiceName, SERVICE_QUERY_CONFIG | SERVICE_STOP | DELETE);
                if (hService) {
                    QUERY_SERVICE_CONFIG* qsc = nullptr;
                    DWORD qscBytesNeeded = 0;
                    QueryServiceConfigA(hService, NULL, 0, &qscBytesNeeded);
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                        std::vector<BYTE> qscBuffer(qscBytesNeeded);
                        qsc = (QUERY_SERVICE_CONFIG*)qscBuffer.data();
                        if (QueryServiceConfigA(hService, qsc, qscBytesNeeded, &qscBytesNeeded)) {
                            std::string servicePath = qsc->lpBinaryPathName;
                            // Normalizar la ruta para la comparación
                            if (servicePath.front() == '"') servicePath.erase(0, 1);
                            if (servicePath.back() == '"') servicePath.pop_back();
                            if (_stricmp(servicePath.c_str(), driverPath.c_str()) == 0) {
                                std::cout << "[CLEANUP] Encontrado servicio huerfano '" << services[i].lpServiceName << "' para el driver. Eliminando..." << std::endl;
                                SERVICE_STATUS ssp;
                                ControlService(hService, SERVICE_CONTROL_STOP, &ssp);
                                DeleteService(hService);
                            }
                        }
                    }
                    CloseServiceHandle(hService);
                }
            }
        }
    }
    CloseServiceHandle(hSCM);
}


InjectionEngine::InjectionEngine() : currentDriverIndex(-1) {
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

void InjectionEngine::InitializeDriverDatabase() {
    drivers = {
        {"Gigabyte GDrv", "gdrv.sys", "Premium", "CVE-2018-19320", "Gigabyte App Center", "Excelente compatibilidad con todos los anti-cheats"},
        {"MSI MSIo", "msio64.sys", "Premium", "CVE-2019-16098", "MSI Afterburner/Dragon Center", "Alto éxito con BattlEye y EAC"},
        {"ASUS AsIO", "AsIO3.sys", "Premium", "CVE-2020-15368", "ASUS Armoury Crate/AI Suite", "Compatible con Vanguard y VAC"},
        {"MSI RTCore", "RTCore64.sys", "Standard", "CVE-2019-16098", "MSI Afterburner", "Funciona bien con la mayoría de juegos"},
        {"CPU-Z", "cpuz159_x64.sys", "Standard", "CVE-2017-15302", "CPU-Z Official", "Driver estable y ampliamente disponible"},
        {"Process Hacker", "kprocesshacker.sys", "High-Risk", "CVE-2020-13833", "Process Hacker", "ALTO RIESGO - Detectado fácilmente"}
    };
}

json InjectionEngine::ListAvailableDrivers() {
    json result = json::array();
    int index = 0;
    for (const auto& db_driver : drivers) {
        json driver_info = db_driver;
        driver_info["id"] = index++;
        if (std::filesystem::exists("drivers/" + db_driver.filename)) {
            driver_info["status"] = "Disponible";
        } else {
            driver_info["status"] = "No Encontrado";
        }
        result.push_back(driver_info);
    }
    return result;
}

// Las funciones de privilegios están ahora en privilege_manager.h/cpp

// Las funciones de verificación de privilegios están ahora en privilege_manager.h/cpp

json InjectionEngine::LoadDriver(int driverIndex) {
    if (IsDriverLoaded()) {
        return {{"success", false}, {"message", "Ya hay un driver cargado. Descárgalo primero."}};
    }
    if (driverIndex < 0 || driverIndex >= static_cast<int>(drivers.size())) {
        return {{"success", false}, {"message", "Índice de driver inválido."}};
    }

    // Verificar permisos de administrador
    if (!IsUserAdmin()) {
        return {{"success", false}, {"message", "Se requieren permisos de administrador para cargar drivers. Ejecuta el programa como administrador."}};
    }

    // --- HABILITAR PRIVILEGIOS PARA CARGAR DRIVERS ---
    std::cout << "[PRIV] Solicitando privilegios para cargar drivers..." << std::endl;
    BOOL privilegeResult = SetLoadDriverPrivilege(TRUE);
    if (!privilegeResult) {
        std::cout << "[PRIV] ⚠️  SeLoadDriverPrivilege no disponible directamente." << std::endl;
        std::cout << "[PRIV] 🔧 Continuando con métodos alternativos..." << std::endl;
        // No fallamos aquí - continuamos con el proceso de carga
    } else {
        std::cout << "[PRIV] ✅ Privilegios obtenidos correctamente." << std::endl;
    }

    const auto& driver = drivers[driverIndex];
    std::string driver_relative_path = "drivers\\" + driver.filename;

    if (!std::filesystem::exists(driver_relative_path)) {
        return {{"success", false}, {"message", "Archivo del driver no encontrado en la carpeta 'drivers': " + driver.filename}};
    }

    char absolutePath[MAX_PATH];
    if (GetFullPathNameA(driver_relative_path.c_str(), MAX_PATH, absolutePath, NULL) == 0) {
         return {{"success", false}, {"message", "No se pudo obtener la ruta absoluta para: " + driver_relative_path}};
    }

    // --- MEJORA: LIMPIAR SERVICIOS HUÉRFANOS ANTES DE CARGAR ---
    CleanupExistingDriverServices(absolutePath);

    std::string quotedPath = "\"" + std::string(absolutePath) + "\"";

    // Intentar diferentes niveles de permisos para el SCM
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) {
        // Si falla con ALL_ACCESS, intentar con permisos más específicos
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE | SC_MANAGER_CONNECT);
        if (!hSCM) {
            DWORD lastError = GetLastError();
            std::string errorMsg = "Error al abrir SCM: " + std::to_string(lastError);
            switch (lastError) {
                case ERROR_ACCESS_DENIED:
                    errorMsg += " (Acceso denegado - Verifica que el programa se ejecute como administrador)";
                    break;
                case ERROR_INVALID_PARAMETER:
                    errorMsg += " (Parámetros inválidos)";
                    break;
                case ERROR_DATABASE_DOES_NOT_EXIST:
                    errorMsg += " (Base de datos del SCM no existe)";
                    break;
                default:
                    errorMsg += " (Error desconocido del sistema)";
                    break;
            }
            return {{"success", false}, {"message", errorMsg}};
        }
    }

    currentServiceName = GenerateRandomString(12);
    SC_HANDLE hService = CreateServiceA(hSCM, currentServiceName.c_str(), currentServiceName.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, quotedPath.c_str(), NULL, NULL, NULL, NULL, NULL);
    
    if (!hService) {
        if (GetLastError() == ERROR_SERVICE_EXISTS) {
            hService = OpenServiceA(hSCM, currentServiceName.c_str(), SERVICE_ALL_ACCESS);
        }
        if (!hService) {
            CloseServiceHandle(hSCM);
            return {{"success", false}, {"message", "No se pudo crear/abrir el servicio del driver: " + std::to_string(GetLastError())}};
        }
    }

    if (!StartService(hService, 0, NULL)) {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_SERVICE_ALREADY_RUNNING) {
            std::string error_msg = "No se pudo iniciar el servicio del driver. Código: " + std::to_string(lastError);
            if(lastError == 2) { 
                error_msg += " (El sistema no pudo encontrar el archivo del driver. Asegúrate de que está en 'build\\drivers\\' y que la ruta es correcta)";
            }
            DeleteService(hService);
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return {{"success", false}, {"message", error_msg}};
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    currentDriverIndex = driverIndex;
    return {{"success", true}, {"message", "Driver '" + driver.name + "' cargado como '" + currentServiceName + "'."}};
}

// ... (El resto de las funciones de injection_engine.cpp permanecen igual que en la versión anterior) ...

json InjectionEngine::UnloadDriver() {
    if (!IsDriverLoaded()) {
        return {{"success", false}, {"message", "No hay ningún driver cargado."}};
    }

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) return {{"success", false}, {"message", "Error al abrir SCM."}};

    SC_HANDLE hService = OpenServiceA(hSCM, currentServiceName.c_str(), SERVICE_STOP | DELETE);
    if (!hService) {
        CloseServiceHandle(hSCM);
        return {{"success", false}, {"message", "No se pudo abrir el servicio para detenerlo."}};
    }

    SERVICE_STATUS status;
    ControlService(hService, SERVICE_CONTROL_STOP, &status);
    
    DeleteService(hService);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    
    std::string unloadedDriverName = GetCurrentDriverName();
    currentDriverIndex = -1;
    currentServiceName = "";

    // --- REVOCAR PRIVILEGIOS DE CARGA DE DRIVERS ---
    std::cout << "[PRIV] Revocando privilegios de carga de drivers." << std::endl;
    SetLoadDriverPrivilege(FALSE);

    return {{"success", true}, {"message", "Driver '" + unloadedDriverName + "' descargado."}};
}

bool InjectionEngine::ExecuteLuaScript(const std::string& scriptContent, DWORD processId) {
    std::string ceExecutable = "core_dlls\\cheatengine-x86_64.exe";
    std::string tempScriptName = "temp_script_" + GenerateRandomString(8) + ".lua";
    
    std::ofstream scriptFile(tempScriptName);
    if (!scriptFile) return false;
    scriptFile << scriptContent;
    scriptFile.close();

    std::stringstream cmd;
    if (processId != 0) {
         cmd << "\"" << ceExecutable << "\" --process=" << processId << " -s \"" << tempScriptName << "\"";
    } else {
         cmd << "\"" << ceExecutable << "\" -s \"" << tempScriptName << "\"";
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    bool success = CreateProcessA(NULL, const_cast<char*>(cmd.str().c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (success) {
        WaitForSingleObject(pi.hProcess, 10000); // 10s timeout
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::filesystem::remove(tempScriptName);
        return true;
    }
    
    std::filesystem::remove(tempScriptName);
    return false;
}

json InjectionEngine::InjectDLL(DWORD processId, const std::string& dllPath) {
    if (!IsDriverLoaded()){
        return {{"success", false}, {"message", "Necesitas cargar un driver primero."}};
    }
    if (!std::filesystem::exists(dllPath)){
        return {{"success", false}, {"message", "La ruta de la DLL no existe: " + dllPath}};
    }
    
    std::string script = "local pid = " + std::to_string(processId) + "\n";
    script += "local result = injectDll(pid, [[" + dllPath + "]])\n";
    script += "print('Resultado de inyección: ' .. tostring(result))";

    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de inyección enviado a Cheat Engine."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de inyección de CE."}};
}

json InjectionEngine::LoadCheatTable(const std::string& ctFilePath, DWORD processId) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no existe: " + ctFilePath}};
    }

    std::string luaPath = ctFilePath;
    std::regex re("\\\\");
    luaPath = std::regex_replace(luaPath, re, "\\\\");

    std::string script = "local pid = " + std::to_string(processId) + "\n";
    script += "openProcess(pid)\n";
    script += "local success = loadTable([[" + luaPath + "]])\n";
    script += "if not success then print('Fallo al cargar la tabla desde el script.') end\n";

    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de carga de tabla enviado de forma silenciosa."}};
    }
    return {{"success", false}, {"message", "No se pudo ejecutar el script de carga de tabla."}};
}

json InjectionEngine::GetProcessList() {
    std::vector<ProcessInfo> processList;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "CreateToolhelp32Snapshot falló."}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Obtener información básica del proceso
            ProcessInfo processInfo;
            processInfo.pid = pe32.th32ProcessID;
            processInfo.name = pe32.szExeFile;
            processInfo.cpuUsage = 0.0; // CPU usage simplificado por ahora
            
            // Intentar obtener información adicional del proceso
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    // Usamos el working set como indicador aproximado de actividad
                    processInfo.cpuUsage = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0); // MB
                }
                CloseHandle(hProcess);
            }
            
            processList.push_back(processInfo);
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    
    // Ordenar por "actividad" (memoria en uso como proxy)
    std::sort(processList.begin(), processList.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.cpuUsage > b.cpuUsage;
    });

    return {{"success", true}, {"processes", processList}};
}

json InjectionEngine::GetCheatTableEntries(const std::string& ctFilePath) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no existe: " + ctFilePath }};
    }
    std::ifstream file(ctFilePath);
    if (!file.is_open()) {
        return {{"success", false}, {"message", "No se pudo abrir el archivo .CT."}};
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    json entries = json::array();
    std::regex re("<CheatEntry ID=\"(\\d+)\"[^>]*>.*?<Description>\"(.*?)\"</Description>.*?</CheatEntry>");
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), re);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        if (match.size() > 2) {
            entries.push_back({
                {"id", std::stoi(match[1].str())},
                {"description", match[2].str()}
            });
        }
    }
    return {{"success", true}, {"entries", entries}};
}

json InjectionEngine::ControlCheatEntry(const std::string& ctFilePath, DWORD processId, int entryId, bool activate) {
    std::string action = activate ? "true" : "false";
    std::string script = "local al = getAddressList()\n";
    script += "local entry = al.getMemoryRecordByID(" + std::to_string(entryId) + ")\n";
    script += "if entry then entry.Active = " + action + " end";
    
    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de control de entrada enviado."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de control."}};
}

json InjectionEngine::SetCheatEntryValue(const std::string& ctFilePath, DWORD processId, int entryId, const std::string& value) {
    std::string script = "local al = getAddressList()\n";
    script += "local entry = al.getMemoryRecordByID(" + std::to_string(entryId) + ")\n";
    script += "if entry then entry.Value = [[" + value + "]] end";

    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de cambio de valor enviado."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de cambio de valor."}};
}

json InjectionEngine::GetCheatScript(const std::string& ctFilePath, int entryId) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no existe."}};
    }
    std::ifstream file(ctFilePath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::regex re("<CheatEntry ID=\"" + std::to_string(entryId) + "\"[\\s\\S]*?>[\\s\\S]*?</CheatEntry>");
    std::smatch entryMatch;
    
    if (std::regex_search(content, entryMatch, re)) {
        std::string entryContent = entryMatch[1].str();
        std::regex scriptRe("<AssemblerScript>([\\s\\S]*?)</AssemblerScript>");
        std::smatch scriptMatch;
        if (std::regex_search(entryContent, scriptMatch, scriptRe)) {
            return {{"success", true}, {"script", scriptMatch[1].str()}};
        }
        return {{"success", false}, {"message", "No se encontró un script LUA/ASM para esta entrada."}};
    }
    
    return {{"success", false}, {"message", "No se encontró la entrada con el ID especificado."}};
}

json InjectionEngine::UpdateCheatScript(const std::string& ctFilePath, int entryId, const std::string& newScript) {
    return {{"success", false}, {"message", "La edición de scripts aún no está implementada."}};
}

json InjectionEngine::SetSpeedhack(DWORD processId, float speed) {
    std::string script = "speedhack_setSpeed(" + std::to_string(speed) + ")";
    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de speedhack enviado."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de speedhack."}};
}

size_t InjectionEngine::GetDriverCount() const {
    return drivers.size();
}

bool InjectionEngine::IsDriverLoaded() const {
    return currentDriverIndex >= 0;
}

std::string InjectionEngine::GetCurrentDriverName() const {
    if (IsDriverLoaded()) {
        return drivers[currentDriverIndex].name;
    }
    return "Ninguno";
}

json InjectionEngine::GetDriverInfoJson(int driverIndex) {
    if (driverIndex < 0 || driverIndex >= static_cast<int>(drivers.size())) {
        return {{"success", false}, {"message", "Índice inválido"}};
    }
    return drivers[driverIndex];
}

json InjectionEngine::GetSystemStatus() {
    json status;
    status["driverLoaded"] = IsDriverLoaded();
    status["currentDriverName"] = GetCurrentDriverName();
    status["serviceName"] = IsDriverLoaded() ? currentServiceName : "N/A";
    status["cheatEnginePath"] = std::filesystem::absolute("core_dlls/cheatengine-x86_64.exe").string();
    
    // --- INFORMACIÓN DE PRIVILEGIOS Y PERMISOS ---
    status["runningAsAdmin"] = IsUserAdmin();
    // Comprueba si podemos obtener el privilegio, y luego lo revoca para no dejarlo activo
    BOOL can_get_load_priv = SetLoadDriverPrivilege(TRUE);
    if (can_get_load_priv) {
        SetLoadDriverPrivilege(FALSE);
    }
    status["loadDriverPrivilege"] = can_get_load_priv;
    status["debugPrivilege"] = IsPrivilegeEnabled(L"SeDebugPrivilege");
    
    return status;
}

json InjectionEngine::FindProcess(const std::string& processName) {
    std::vector<ProcessInfo> foundProcesses;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "CreateToolhelp32Snapshot falló."}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string currentProcessName = pe32.szExeFile;
            
            // Buscar coincidencias exactas o parciales (case-insensitive)
            std::transform(currentProcessName.begin(), currentProcessName.end(), currentProcessName.begin(), ::tolower);
            std::string searchName = processName;
            std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
            
            if (currentProcessName.find(searchName) != std::string::npos) {
                ProcessInfo processInfo;
                processInfo.pid = pe32.th32ProcessID;
                processInfo.name = pe32.szExeFile;
                processInfo.cpuUsage = 0.0;
                
                // Intentar obtener información adicional del proceso
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        processInfo.cpuUsage = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0); // MB
                    }
                    CloseHandle(hProcess);
                }
                
                foundProcesses.push_back(processInfo);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    
    if (foundProcesses.empty()) {
        return {{"success", false}, {"message", "No se encontraron procesos con el nombre: " + processName}};
    }
    
    return {{"success", true}, {"processes", foundProcesses}, {"count", foundProcesses.size()}};
}