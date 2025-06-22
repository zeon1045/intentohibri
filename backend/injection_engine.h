#pragma once
#include <string>
#include <vector>
#include <windows.h>

// **🔥 ESTRUCTURA DE INFORMACIÓN DE DRIVER**
struct DriverInfo {
    std::string name;        // Nombre descriptivo
    std::string filename;    // Nombre del archivo .sys
    std::string tier;        // Premium/Standard/High-Risk
    std::string cve;         // CVE number si disponible
    std::string source;      // Software de origen
    std::string description; // Descripción del riesgo/compatibilidad
};

// **🔥 CLASE PRINCIPAL DEL MOTOR DE INYECCIÓN**
class InjectionEngine {
private:
    std::vector<DriverInfo> drivers;        // Base de datos de drivers
    int currentDriverIndex;                 // Índice del driver actualmente cargado
    std::string currentServiceName;         // Nombre del servicio actual

    // Inicializa la base de datos interna de drivers
    void InitializeDriverDatabase();

public:
    // Constructor y Destructor
    InjectionEngine();
    ~InjectionEngine();

    // **🎯 FUNCIONES PRINCIPALES DE INYECCIÓN**
    
    // Inyecta una DLL usando Cheat Engine real
    bool InjectDLL(DWORD processId, const std::string& dllPath);
    
    // Escanea memoria usando Cheat Engine
    bool ScanMemory(DWORD processId, const std::string& value, const std::string& valueType);
    
    // Encuentra proceso por nombre (mejorado)
    DWORD FindProcess(const std::string& processName);
    
    // **🎯 SOPORTE DE CHEAT TABLES (.CT)**
    
    // Carga un archivo .CT en Cheat Engine para el proceso especificado
    bool LoadCheatTable(const std::string& ctFilePath, DWORD processId);
    
    // **🔧 GESTIÓN DE DRIVERS BYOVD**
    
    // Lista todos los drivers .sys disponibles en carpeta drivers/
    std::vector<std::string> ListAvailableDrivers();
    
    // Carga un driver específico por índice
    bool LoadDriver(int driverIndex);
    
    // Descarga el driver actualmente cargado
    bool UnloadDriver();
    
    // Obtiene información detallada de un driver por índice
    std::string GetDriverInfo(int driverIndex);
    
    // **📊 FUNCIONES DE UTILIDAD**
    
    // Obtiene el número total de drivers en la base de datos
    size_t GetDriverCount() const { return drivers.size(); }
    
    // Verifica si hay un driver cargado actualmente
    bool IsDriverLoaded() const { return currentDriverIndex >= 0; }
    
    // Obtiene el nombre del driver actualmente cargado
    std::string GetCurrentDriverName() const {
        if (currentDriverIndex >= 0 && currentDriverIndex < static_cast<int>(drivers.size())) {
            return drivers[currentDriverIndex].name;
        }
        return "Ninguno";
    }
}; 