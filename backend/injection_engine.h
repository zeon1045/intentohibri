/*
 * =====================================================================================
 *
 * >>> ARCHIVO: backend/injection_engine.h <<<
 *
 * Copia y pega TODO el código de abajo en tu archivo "injection_engine.h".
 * (Este archivo solo contiene DECLARACIONES de funciones, terminadas en ';')
 *
 * =====================================================================================
 */
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

// Declaración de la clase InjectionEngine
class InjectionEngine {
private:
    // --- MIEMBROS PRIVADOS ---
    std::vector<DriverInfo> drivers;
    int currentDriverIndex;
    std::string currentServiceName;

    // --- MÉTODOS PRIVADOS (SOLO DECLARACIONES) ---
    void InitializeDriverDatabase();
    std::string GenerateRandomString(int length);
    bool ExecuteLuaScript(const std::string& scriptContent, DWORD processId);

public:
    // --- CONSTRUCTOR Y DESTRUCTOR ---
    InjectionEngine();
    ~InjectionEngine();

    // --- GESTIÓN DE DRIVERS ---
    json ListAvailableDrivers();
    json LoadDriver(int driverIndex);
    json UnloadDriver();
    json GetDriverInfoJson(int driverIndex);
    
    // --- FUNCIONALIDAD PRINCIPAL CON CHEAT ENGINE ---
    json InjectDLL(DWORD processId, const std::string& dllPath);
    json FindProcess(const std::string& processName);
    
    // --- CONTROL PROGRAMÁTICO DE CHEAT TABLE (.CT) ---
    json LoadCheatTable(const std::string& ctFilePath, DWORD processId);
    json GetCheatTableEntries(const std::string& ctFilePath);
    json ControlCheatEntry(const std::string& ctFilePath, DWORD processId, int entryId, bool activate);
    json SetCheatEntryValue(const std::string& ctFilePath, DWORD processId, int entryId, const std::string& value);

    // --- FUNCIONES AVANZADAS DE CE ---
    json SetSpeedhack(DWORD processId, float speed);

    // --- FUNCIONES DE UTILIDAD ---
    size_t GetDriverCount() const;
    bool IsDriverLoaded() const;
    std::string GetCurrentDriverName() const;
    json GetSystemStatus();
};
