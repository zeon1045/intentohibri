//=====================================================================================
//=====================================================================================
//
//   >>> FIN DEL ARCHIVO: backend/injection_engine.h <<<
//   >>> INICIO DEL ARCHIVO: backend/injection_engine.cpp <<<
//
//=====================================================================================
//=====================================================================================

#include "injection_engine.h"
#include <windows.h>
#include <tlhelp32.h>
#include <Pdh.h>
#include <PdhMsg.h>
#pragma comment(lib, "pdh.lib")
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <random>
#include <regex>
#include <map>

// --- IMPLEMENTACIONES DE LAS FUNCIONES DE INJECTIONENGINE ---

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
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);
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
    std::string driverPath = "drivers\\" + driver.filename;
    char absolutePath[MAX_PATH];
    if (GetFullPathNameA(driverPath.c_str(), MAX_PATH, absolutePath, NULL) == 0) {
         return {{"success", false}, {"message", "No se pudo encontrar el archivo del driver: " + driverPath}};
    }

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        return {{"success", false}, {"message", "Error al abrir SCM (¿sin permisos de admin?): " + std::to_string(GetLastError())}};
    }

    currentServiceName = GenerateRandomString(12);
    SC_HANDLE hService = CreateServiceA(hSCM, currentServiceName.c_str(), currentServiceName.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, absolutePath, NULL, NULL, NULL, NULL, NULL);
    
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
        if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) {
            DeleteService(hService);
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return {{"success", false}, {"message", "No se pudo iniciar el servicio del driver: " + std::to_string(GetLastError())}};
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    currentDriverIndex = driverIndex;
    return {{"success", true}, {"message", "Driver '" + driver.name + "' cargado como '" + currentServiceName + "'."}};
}

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

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    bool success = CreateProcessA(NULL, const_cast<char*>(cmd.str().c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (success) {
        WaitForSingleObject(pi.hProcess, 10000);
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
        return {{"success", false}, {"message", "La ruta de la DLL no existe."}};
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
        return {{"success", false}, {"message", "El archivo .CT no existe."}};
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
    std::map<std::string, DWORD> nameToPid;
    std::map<DWORD, double> pidToCpu;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                std::string processName = pe32.szExeFile;
                nameToPid[processName] = pe32.th32ProcessID;
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    } else {
        return {{"success", false}, {"message", "CreateToolhelp32Snapshot falló."}};
    }

    PDH_HQUERY query;
    if (PdhOpenQueryA(NULL, NULL, &query) != ERROR_SUCCESS) {
        return {{"success", false}, {"message", "PdhOpenQueryA falló."}};
    }

    PDH_HCOUNTER counter;
    if (PdhAddCounterA(query, "\\Process(*)\\% Processor Time", NULL, &counter) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return {{"success", false}, {"message", "PdhAddCounterA falló."}};
    }
    
    PdhCollectQueryData(query);
    Sleep(100);
    PdhCollectQueryData(query);

    DWORD bufferSize = 0;
    DWORD itemCount = 0;
    PDH_FMT_COUNTERVALUE_ITEM_A* items = nullptr; 

    if (PdhGetFormattedCounterArrayA(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, items) == PDH_MORE_DATA) {
        items = (PDH_FMT_COUNTERVALUE_ITEM_A*)malloc(bufferSize);
        if (items && PdhGetFormattedCounterArrayA(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, items) == ERROR_SUCCESS) {
            for (DWORD i = 0; i < itemCount; ++i) {
                std::string instanceName = items[i].szName;
                if (nameToPid.count(instanceName)) {
                     pidToCpu[nameToPid[instanceName]] = items[i].FmtValue.doubleValue;
                }
            }
        }
        if (items) free(items);
    }
    PdhCloseQuery(query);

    for (const auto& pair : nameToPid) {
        double cpu = pidToCpu.count(pair.second) ? pidToCpu[pair.second] : 0.0;
        processList.push_back({pair.second, pair.first, cpu});
    }

    std::sort(processList.begin(), processList.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.cpuUsage > b.cpuUsage;
    });

    return {{"success", true}, {"processes", processList}};
}


json InjectionEngine::GetCheatTableEntries(const std::string& ctFilePath) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no existe."}};
    }
    std::ifstream file(ctFilePath);
    if (!file.is_open()) {
        return {{"success", false}, {"message", "No se pudo abrir el archivo .CT."}};
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    json entries = json::array();
    std::regex re("<Description>\"(.*?)\"</Description>");
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), re);
    auto words_end = std::sregex_iterator();

    int id = 0;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        if (match.size() > 1) {
            entries.push_back({
                {"id", id++},
                {"description", match[1].str()}
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

    std::regex re("<CheatEntry>(.*?)</CheatEntry>");
    auto begin = std::sregex_iterator(content.begin(), content.end(), re);
    auto end = std::sregex_iterator();
    
    int currentIndex = 0;
    for (std::sregex_iterator i = begin; i != end; ++i) {
        if (currentIndex == entryId) {
            std::smatch match = *i;
            std::string entryContent = match[1].str();
            std::regex scriptRe("<AssemblerScript>(.*?)</AssemblerScript>");
            std::smatch scriptMatch;
            if (std::regex_search(entryContent, scriptMatch, scriptRe)) {
                return {{"success", true}, {"script", scriptMatch[1].str()}};
            }
            return {{"success", false}, {"message", "No se encontró un script para esta entrada."}};
        }
        currentIndex++;
    }
    return {{"success", false}, {"message", "No se encontró la entrada con el ID especificado."}};
}

json InjectionEngine::UpdateCheatScript(const std::string& ctFilePath, int entryId, const std::string& newScript) {
    if (!std::filesystem::exists(ctFilePath)) {
        return {{"success", false}, {"message", "El archivo .CT no existe."}};
    }
    std::ifstream file(ctFilePath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::regex re("<CheatEntry>(.*?)</CheatEntry>");
    auto begin = std::sregex_iterator(content.begin(), content.end(), re);
    auto end = std::sregex_iterator();
    
    int currentIndex = 0;
    bool updated = false;
    for (std::sregex_iterator i = begin; i != end; ++i) {
        if (currentIndex == entryId) {
            std::smatch match = *i;
            std::string entryContent = match[1].str();
            std::regex scriptRe("<AssemblerScript>(.*?)</AssemblerScript>");
            std::string updatedEntry = std::regex_replace(entryContent, scriptRe, "<AssemblerScript>" + newScript + "</AssemblerScript>");
            content.replace(match.position(), match.length(), "<CheatEntry>" + updatedEntry + "</CheatEntry>");
            updated = true;
            break;
        }
        currentIndex++;
    }

    if(updated) {
        std::ofstream outFile(ctFilePath);
        outFile << content;
        outFile.close();
        return {{"success", true}, {"message", "Script actualizado correctamente. La tabla debe ser recargada."}};
    }

    return {{"success", false}, {"message", "No se pudo actualizar el script."}};
}

json InjectionEngine::SetSpeedhack(DWORD processId, float speed) {
    std::string script = "speedhack_setSpeed(" + std::to_string(speed) + ")";
    if (ExecuteLuaScript(script, processId)) {
        return {{"success", true}, {"message", "Comando de speedhack enviado."}};
    }
    return {{"success", false}, {"message", "Fallo al ejecutar el script de speedhack."}};
}

json InjectionEngine::FindProcess(const std::string& processName) {
    json processes = json::array();
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return {{"success", false}, {"message", "No se pudo crear snapshot de procesos."}};
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string currentProcess = pe32.szExeFile;
            if (currentProcess.find(processName) != std::string::npos) {
                processes.push_back({
                    {"pid", pe32.th32ProcessID},
                    {"name", currentProcess}
                });
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return {{"success", true}, {"processes", processes}};
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