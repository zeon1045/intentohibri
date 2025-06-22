//=====================================================================================
//=====================================================================================
//
//   >>> FIN DEL ARCHIVO: backend/injection_engine.cpp <<<
//   >>> INICIO DEL ARCHIVO: backend/main.cpp <<<
//
//=====================================================================================
//=====================================================================================

#define _WIN32_WINNT 0x0A00 // Definir la versión de Windows para compatibilidad
#include "../libs/httplib.h"
#include "injection_engine.h"
#include <iostream>
#include <shellapi.h>
#include <windows.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#define SERVER_PORT 12345

// Función para obtener la ruta del directorio donde se encuentra el .exe
std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

// Función para recortar comillas
std::string trim_quotes(std::string s) {
    s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
    return s;
}

int main() {
    // Establecer la página de códigos de la consola a UTF-8
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    // Asignar consola para logging
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    
    std::cout << "=================================================" << std::endl;
    std::cout << "          🔥 Belzebub - Professional Suite 🔥" << std::endl;
    std::cout << "=================================================" << std::endl;

    // Establecer el directorio de trabajo al del ejecutable
    std::string exe_dir = get_executable_dir();
    SetCurrentDirectoryA(exe_dir.c_str());
    std::cout << "[INFO] Directorio de trabajo establecido en: " << exe_dir << std::endl;

    InjectionEngine engine;
    httplib::Server svr;

    const char* frontend_path = "./frontend";
    auto ret = svr.set_mount_point("/", frontend_path);
    if (!ret) {
        std::cerr << "ERROR: La carpeta 'frontend' no existe en la misma ubicacion que el .exe" << std::endl;
        std::cerr << "Se esperaba encontrar en: " << std::filesystem::absolute(frontend_path) << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[SERVER] Sirviendo archivos del frontend desde: " << std::filesystem::absolute(frontend_path) << std::endl;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        std::ifstream file("./frontend/dashboard.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html; charset=utf-8");
        } else {
            res.status = 404;
            res.set_content("No se pudo encontrar dashboard.html", "text/plain; charset=utf-8");
        }
    });

    // --- API Endpoints ---
    svr.Get("/api/status", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.GetSystemStatus().dump(), "application/json; charset=utf-8");
    });
    svr.Get("/api/drivers", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.ListAvailableDrivers().dump(), "application/json; charset=utf-8");
    });
    svr.Get("/api/processes", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.GetProcessList().dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/load_driver", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        int driverId = body["id"];
        res.set_content(engine.LoadDriver(driverId).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/unload_driver", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(engine.UnloadDriver().dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/inject_dll", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string dllPath = trim_quotes(body["dllPath"]);
        res.set_content(engine.InjectDLL(pid, dllPath).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/find_process", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string processName = body["processName"];
        res.set_content(engine.FindProcess(processName).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/load_ct", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        res.set_content(engine.LoadCheatTable(ctPath, pid).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/get_ct_entries", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string ctPath = trim_quotes(body["ctPath"]);
        res.set_content(engine.GetCheatTableEntries(ctPath).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/control_cheat", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        bool activate = body["activate"];
        res.set_content(engine.ControlCheatEntry(ctPath, pid, entryId, activate).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/set_cheat_value", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        std::string value = body["value"];
        res.set_content(engine.SetCheatEntryValue(ctPath, pid, entryId, value).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/set_speedhack", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        float speed = body["speed"];
        res.set_content(engine.SetSpeedhack(pid, speed).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/get_script", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        res.set_content(engine.GetCheatScript(ctPath, entryId).dump(), "application/json; charset=utf-8");
    });
     svr.Post("/api/update_script", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        std::string script = body["script"];
        res.set_content(engine.UpdateCheatScript(ctPath, entryId, script).dump(), "application/json; charset=utf-8");
    });

    std::string url = "http://localhost:" + std::to_string(SERVER_PORT);
    std::cout << "[SERVER] Iniciando servidor en " << url << std::endl;
    
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    svr.listen("0.0.0.0", SERVER_PORT);

    return 0;
}