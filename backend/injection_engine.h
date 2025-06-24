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

// Forward declarations
struct DriverInfo;

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

// Declaración de la clase InjectionEngine
class InjectionEngine {
public:
    InjectionEngine();
    ~InjectionEngine();

    json GetSystemStatus();
    json ListAvailableDrivers();
    json LoadDriver(int driverIndex);
    json UnloadDriver();
    json GetProcessList();
    json GetCheatTableEntries(const std::string& ctFilePath);
    json GetCheatTableEntriesFromContent(const std::string& ctContent);
    
    // Nueva función para activar/desactivar cheats
    json ActivateCheatEntry(const std::string& ctFilePath, int entryId, bool activate);

    // Nueva función para establecer el proceso objetivo
    void SelectProcess(DWORD pid);

private:
    std::map<int, DriverInfo> knownDrivers;
    const DriverInfo* currentDriver = nullptr;
    std::string currentServiceName = "";
    bool driverLoaded = false;
    bool hasLoadDriverPrivilege = false;
    bool hasDebugPrivilege = false;
    DWORD targetProcessId = 0; // Guardamos el PID objetivo

    std::map<std::string, CTLoader::CheatTable> cheatTableCache;
    
    void InitializeDriverDatabase();
    bool CleanupService(const std::string& serviceName);
    bool IsDriverLoaded() const { return driverLoaded; }
    std::string GetCurrentDriverName() const { return currentDriver ? currentDriver->name : "Ninguno"; }
};