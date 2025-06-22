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
#include <windows.h>
#include "../libs/json.hpp" // Integración de la librería JSON

using json = nlohmann::json;

// Estructura de información del driver
struct DriverInfo {
    std::string name;
    std::string filename;
    std::string tier;
    std::string cve;
    std::string source;
    std::string description;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DriverInfo, name, filename, tier, cve, source, description);
};

// Estructura para la lista de procesos avanzada
struct ProcessInfo {
    DWORD pid;
    std::string name;
    double cpuUsage;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ProcessInfo, pid, name, cpuUsage);
};

// Declaración de la clase InjectionEngine
class InjectionEngine {
private:
    std::vector<DriverInfo> drivers;
    int currentDriverIndex;
    std::string currentServiceName;

    void InitializeDriverDatabase();
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
    
    // --- FUNCIONALIDAD CHEAT ENGINE ---
    json InjectDLL(DWORD processId, const std::string& dllPath);
    json SetSpeedhack(DWORD processId, float speed);
    
    // --- CONTROL DE CHEAT TABLE (.CT) ---
    json LoadCheatTable(const std::string& ctFilePath, DWORD processId);
    json GetCheatTableEntries(const std::string& ctFilePath);
    json ControlCheatEntry(const std::string& ctFilePath, DWORD processId, int entryId, bool activate);
    json SetCheatEntryValue(const std::string& ctFilePath, DWORD processId, int entryId, const std::string& value);

    // --- NUEVO: EDITOR DE SCRIPTS ---
    json GetCheatScript(const std::string& ctFilePath, int entryId);
    json UpdateCheatScript(const std::string& ctFilePath, int entryId, const std::string& newScript);

    // --- FUNCIONES DE UTILIDAD ---
    size_t GetDriverCount() const;
    bool IsDriverLoaded() const;
    std::string GetCurrentDriverName() const;
    json GetSystemStatus();
};