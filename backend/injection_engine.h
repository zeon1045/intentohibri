//=====================================================================================
//=====================================================================================
//
//   >>> INICIO DEL ARCHIVO: backend/injection_engine.h <<<
//
//   (No hay cambios en este archivo. Déjalo como está).
//
//=====================================================================================
//=====================================================================================

#pragma once
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include "../libs/json.hpp" // Integración de la librería JSON
#include "ct_loader.h"       // <-- INTEGRAMOS CT_LOADER

using json = nlohmann::json;

// Estructura de información del driver
struct DriverInfo {
    std::string name;
    std::string filename;
    std::string tier;
    std::string cve;
    std::string source;
    std::string description;
    int architecture = 0; // 32 o 64. 0 si es desconocido.
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DriverInfo, name, filename, tier, cve, source, description, architecture);
};

// Estructura para la lista de procesos avanzada
struct ProcessInfo {
    DWORD pid;
    std::string name;
    double cpuUsage;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ProcessInfo, pid, name, cpuUsage);
};

// Usamos las estructuras de CTLoader en lugar de las nuestras
using CTLoader::MemoryEntry;
using CTLoader::CheatTable;
using CTLoader::CTParser;
using CTLoader::CTError;

// Declaración de la clase InjectionEngine
class InjectionEngine {
private:
    std::map<int, DriverInfo> knownDrivers;
    DriverInfo* currentDriver;
    bool driverLoaded;
    std::string currentServiceName;
    
    // --- VARIABLES PARA ESTADO DE PRIVILEGIOS ---
    bool hasLoadDriverPrivilege = false;
    bool hasDebugPrivilege = false;

    // --- CACHE DE CHEAT TABLES USANDO CT_LOADER ---
    std::map<std::string, CTLoader::CheatTable> cheatTableCache;

    void InitializeDriverDatabase();
    bool CleanupService(const std::string& serviceName);
    std::string GenerateRandomString(int length);
    bool ExecuteLuaScript(const std::string& scriptContent, DWORD processId);

public:
    InjectionEngine();
    ~InjectionEngine();

    // --- GESTIÓN DE DRIVERS ---
    json ListAvailableDrivers();
    json LoadDriver(int driverIndex);
    json UnloadDriver();
    json GetDriverInfoJson(int driverIndex);
    
    // --- GESTIÓN DE PROCESOS ---
    json FindProcess(const std::string& processName); // Se mantiene por simplicidad
    json GetProcessList(); // NUEVA FUNCIÓN AVANZADA
    
    // --- FUNCIONALIDAD CHEAT ENGINE CON CT_LOADER ---
    json InjectDLL(DWORD processId, const std::string& dllPath);
    json SetSpeedhack(DWORD processId, float speed);
    
    // --- CONTROL DE CHEAT TABLE (.CT) USANDO CT_LOADER ---
    json LoadCheatTable(const std::string& ctFilePath, DWORD processId);
    json GetCheatTableEntries(const std::string& ctFilePath);
    json ControlCheatEntry(const std::string& ctFilePath, DWORD processId, int entryId, bool activate);
    json SetCheatEntryValue(const std::string& ctFilePath, DWORD processId, int entryId, const std::string& value);

    // --- EDITOR DE SCRIPTS ---
    json GetCheatScript(const std::string& ctFilePath, int entryId);
    json UpdateCheatScript(const std::string& ctFilePath, int entryId, const std::string& newScript);

    // --- FUNCIONES DE UTILIDAD ---
    size_t GetDriverCount() const;
    bool IsDriverLoaded() const;
    std::string GetCurrentDriverName() const;
    json GetSystemStatus();
};