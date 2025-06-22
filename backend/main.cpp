/*
 * =====================================================================================
 *
 * >>> ARCHIVO: backend/main.cpp <<<
 *
 * Copia y pega TODO el código de abajo en tu archivo "main.cpp".
 *
 * =====================================================================================
 */
#include "../libs/httplib.h"
#include "injection_engine.h"
#include <iostream>
#include <shellapi.h>
#include <windows.h>
#include <filesystem>
#include <string>

#define SERVER_PORT 12345

// Función para obtener la ruta del directorio donde se encuentra el .exe
std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

int main() {
    // Asignar consola para logging
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    
    std::cout << "=================================================" << std::endl;
    std::cout << "      BYOVD Professional Suite v3.1 - Backend" << std::endl;
    std::cout << "=================================================" << std::endl;

    // Establecer el directorio de trabajo al del ejecutable
    std::string exe_dir = get_executable_dir();
    SetCurrentDirectoryA(exe_dir.c_str());
    std::cout << "[INFO] Directorio de trabajo establecido en: " << exe_dir << std::endl;

    InjectionEngine engine;
    httplib::Server svr;

    // Servir archivos estáticos del frontend
    std::string frontend_path = exe_dir + "\\frontend";
    auto ret = svr.set_mount_point("/", frontend_path.c_str());
    if (!ret) {
        std::cerr << "ERROR: La carpeta 'frontend' no existe en la misma ubicación que el .exe" << std::endl;
        std::cerr << "Se esperaba encontrar en: " << frontend_path << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[SERVER] Sirviendo archivos del frontend desde: " << frontend_path << std::endl;

    // --- API Endpoints ---
    svr.Get("/api/status", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.GetSystemStatus().dump(), "application/json");
    });
    svr.Get("/api/drivers", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.ListAvailableDrivers().dump(), "application/json");
    });
    svr.Post("/api/load_driver", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        int driverId = body["id"];
        res.set_content(engine.LoadDriver(driverId).dump(), "application/json");
    });
    svr.Post("/api/unload_driver", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.UnloadDriver().dump(), "application/json");
    });
    svr.Post("/api/inject_dll", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string dllPath = body["dllPath"];
        res.set_content(engine.InjectDLL(pid, dllPath).dump(), "application/json");
    });
    svr.Post("/api/find_process", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string processName = body["processName"];
        res.set_content(engine.FindProcess(processName).dump(), "application/json");
    });
    svr.Post("/api/load_ct", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = body["ctPath"];
        res.set_content(engine.LoadCheatTable(ctPath, pid).dump(), "application/json");
    });
    svr.Post("/api/get_ct_entries", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string ctPath = body["ctPath"];
        res.set_content(engine.GetCheatTableEntries(ctPath).dump(), "application/json");
    });
    svr.Post("/api/control_cheat", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = body["ctPath"];
        int entryId = body["entryId"];
        bool activate = body["activate"];
        res.set_content(engine.ControlCheatEntry(ctPath, pid, entryId, activate).dump(), "application/json");
    });
    svr.Post("/api/set_cheat_value", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = body["ctPath"];
        int entryId = body["entryId"];
        std::string value = body["value"];
        res.set_content(engine.SetCheatEntryValue(ctPath, pid, entryId, value).dump(), "application/json");
    });
    svr.Post("/api/set_speedhack", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        float speed = body["speed"];
        res.set_content(engine.SetSpeedhack(pid, speed).dump(), "application/json");
    });

    std::string url = "http://localhost:" + std::to_string(SERVER_PORT);
    std::cout << "[SERVER] Iniciando servidor en " << url << std::endl;
    
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    svr.listen("0.0.0.0", SERVER_PORT);

    return 0;
}