#include "injection_engine.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <regex>
#include <map>
#include <vector>

#pragma comment(lib, "pdh.lib")

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

json InjectionEngine::LoadDriver(int driverIndex) {
    if (IsDriverLoaded()) {
        return {{"success", false}, {"message", "Ya hay un driver cargado. Descárgalo primero."}};
    }
    if (driverIndex < 0 || driverIndex >= static_cast<int>(drivers.size())) {
        return {{"success", false}, {"message", "Índice de driver inválido."}};
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

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        return {{"success", false}, {"message", "Error al abrir SCM (¿sin permisos de admin?): " + std::to_string(GetLastError())}};
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
    std::map<DWORD, ProcessInfo> processMap;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "CreateToolhelp32Snapshot falló."}};
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32)) {
        do {
            processMap[pe32.th32ProcessID] = {pe32.th32ProcessID, pe32.szExeFile, 0.0};
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    
    PDH_HQUERY query;
    if (PdhOpenQueryA(nullptr, 0, &query) == ERROR_SUCCESS) {
        PDH_HCOUNTER counter;
        if (PdhAddCounterA(query, "\\Process(*)\\% Processor Time", 0, &counter) == ERROR_SUCCESS) {
            PdhCollectQueryData(query);
            Sleep(100);
            PdhCollectQueryData(query);

            DWORD bufferSize = 0;
            DWORD itemCount = 0;
            if (PdhGetFormattedCounterArrayA(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr) == PDH_MORE_DATA) {
                std::vector<char> buffer(bufferSize);
                PDH_FMT_COUNTERVALUE_ITEM_A* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_A*>(buffer.data());
                if (PdhGetFormattedCounterArrayA(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, items) == ERROR_SUCCESS) {
                    for (auto& pair : processMap) {
                        for (DWORD i = 0; i < itemCount; ++i) {
                            if (pair.second.name == items[i].szName) {
                                pair.second.cpuUsage += items[i].FmtValue.doubleValue;
                            }
                        }
                    }
                }
            }
        }
        PdhCloseQuery(query);
    }
    
    std::vector<ProcessInfo> processList;
    for (const auto& pair : processMap) {
        processList.push_back(pair.second);
    }

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

json InjectionEngine::ControlCheatEntry(DWORD processId, int entryId, bool activate) {
    std::string action = activate ? "true" : "false";
    std::string script = "local al = getAddressList()\n";
    script += "local entry = al.getMemoryRecordByID(" + std::to_string(entryId) + ")\n";
    script += "if entry then entry.Active = " + action + " end";
    
    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de control de entrada enviado."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de control."}};
}

json InjectionEngine::SetCheatEntryValue(DWORD processId, int entryId, const std::string& value) {
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

    std::regex re("<CheatEntry ID=\"" + std::to_string(entryId) + "\".*?>(.*?)</CheatEntry>", std::regex::dotall);
    std::smatch entryMatch;
    
    if (std::regex_search(content, entryMatch, re)) {
        std::string entryContent = entryMatch[1].str();
        std::regex scriptRe("<AssemblerScript>(.*?)</AssemblerScript>", std::regex::dotall);
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
    return status;
}