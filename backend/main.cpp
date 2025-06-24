//=====================================================================================
//=====================================================================================
//
//   >>> FIN DEL ARCHIVO: backend/injection_engine.cpp <<<
//   >>> INICIO DEL ARCHIVO: backend/main.cpp <<<
//
//=====================================================================================
//=====================================================================================

#define _WIN32_WINNT 0x0A00
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
#include <commdlg.h> // Para el diálogo de archivos

// Incluir privilege_manager para funciones de administrador
#include "privilege_manager.h"

#define SERVER_PORT 12345

std::string get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path().string();
}

std::string trim_quotes(std::string s) {
    s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
    return s;
}

// NUEVA FUNCIÓN: Abrir diálogo nativo para seleccionar archivos
std::string open_file_dialog(const char* filter) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return ofn.lpstrFile;
    }
    return "";
}


// Función para reiniciar como administrador
bool RestartAsAdmin() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    std::cout << "[ADMIN] Reiniciando como administrador..." << std::endl;
    
    SHELLEXECUTEINFOA sei = {0};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = "runas";  // Solicitar elevación
    sei.lpFile = exePath;
    sei.lpDirectory = NULL;
    sei.nShow = SW_NORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    
    if (ShellExecuteExA(&sei)) {
        std::cout << "[ADMIN] Proceso elevado iniciado. Cerrando instancia actual..." << std::endl;
        return true;
    } else {
        DWORD error = GetLastError();
        std::cout << "[ADMIN] Error al elevar privilegios: " << error << std::endl;
        if (error == ERROR_CANCELLED) {
            std::cout << "[ADMIN] El usuario canceló la elevación de privilegios" << std::endl;
        }
        return false;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    
    std::cout << "=================================================" << std::endl;
    std::cout << "          🔥 Belzebub - Professional Suite 🔥" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Verificar privilegios de administrador al inicio
    if (!IsUserAdmin()) {
        std::cout << "[ADMIN] ⚠️  No se detectaron privilegios de administrador." << std::endl;
        std::cout << "[ADMIN] 🔧 Continuando sin privilegios de administrador (funcionalidad limitada)..." << std::endl;
        std::cout << "[ADMIN] 💡 Para funcionalidad completa, ejecutar como administrador manualmente." << std::endl;
    } else {
        std::cout << "[ADMIN] ✅ Ejecutándose con privilegios de administrador" << std::endl;
    }

    std::string exe_dir = get_executable_dir();
    SetCurrentDirectoryA(exe_dir.c_str());
    std::cout << "[INFO] Directorio de trabajo establecido en: " << exe_dir << std::endl;

    InjectionEngine engine;
    httplib::Server svr;

    // === CONFIGURACIÓN DE CORS ===
    // Función auxiliar para añadir cabeceras CORS a todas las respuestas
    auto add_cors_headers = [](httplib::Response& res) {
        res.headers["Access-Control-Allow-Origin"] = "*";
        res.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        res.headers["Access-Control-Allow-Headers"] = "Origin, X-Requested-With, Content-Type, Accept";
    };

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

    // --- API Endpoints (Existentes) ---
    svr.Get("/api/status", [&add_cors_headers, &engine](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        res.set_content(engine.GetSystemStatus().dump(), "application/json; charset=utf-8");
    });
    svr.Get("/api/drivers", [&add_cors_headers, &engine](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        res.set_content(engine.ListAvailableDrivers().dump(), "application/json; charset=utf-8");
    });
    svr.Get("/api/processes", [&add_cors_headers, &engine](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        res.set_content(engine.GetProcessList().dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/load_driver", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        
        // --- VALIDACIÓN ROBUSTA DEL JSON ---
        if (req.body.empty()) {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"El cuerpo de la solicitud esta vacio.\"}", "application/json; charset=utf-8");
            return;
        }
        
        try {
            auto body = json::parse(req.body);
            if (!body.contains("id")) {
                res.status = 400;
                res.set_content("{\"success\": false, \"message\": \"Falta el campo 'id' en el JSON.\"}", "application/json; charset=utf-8");
                return;
            }
            int driverId = body["id"];
            res.set_content(engine.LoadDriver(driverId).dump(), "application/json; charset=utf-8");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"Error de JSON: " + std::string(e.what()) + "\"}", "application/json; charset=utf-8");
        }
    });
    svr.Post("/api/unload_driver", [&add_cors_headers, &engine](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        res.set_content(engine.UnloadDriver().dump(), "application/json; charset=utf-8");
    });
    /*
    svr.Post("/api/inject_dll", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string dllPath = trim_quotes(body["dllPath"]);
        res.set_content(engine.InjectDLL(pid, dllPath).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/find_process", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        std::string processName = body["processName"];
        res.set_content(engine.FindProcess(processName).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/load_ct", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        res.set_content(engine.LoadCheatTable(ctPath, pid).dump(), "application/json; charset=utf-8");
    });
    */
    svr.Post("/api/get_ct_entries", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        // La petición ahora envía el contenido del archivo en el cuerpo (body)
        std::cout << "[HTTP] Recibido contenido de tabla CT para parsear." << std::endl;
        res.set_content(engine.GetCheatTableEntriesFromContent(req.body).dump(), "application/json; charset=utf-8");
    });
    /*
    svr.Post("/api/control_cheat", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        bool activate = body["activate"];
        res.set_content(engine.ControlCheatEntry(ctPath, pid, entryId, activate).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/set_cheat_value", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        std::string value = body["value"];
        res.set_content(engine.SetCheatEntryValue(ctPath, pid, entryId, value).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/set_speedhack", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        DWORD pid = body["pid"];
        float speed = body["speed"];
        res.set_content(engine.SetSpeedhack(pid, speed).dump(), "application/json; charset=utf-8");
    });
    svr.Post("/api/get_script", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        res.set_content(engine.GetCheatScript(ctPath, entryId).dump(), "application/json; charset=utf-8");
    });
     svr.Post("/api/update_script", [&add_cors_headers, &engine](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        auto body = json::parse(req.body);
        std::string ctPath = trim_quotes(body["ctPath"]);
        int entryId = body["entryId"];
        std::string script = body["script"];
        res.set_content(engine.UpdateCheatScript(ctPath, entryId, script).dump(), "application/json; charset=utf-8");
    });
    */

    // --- NUEVO ENDPOINT PARA EL EXPLORADOR DE ARCHIVOS NATIVO ---
    svr.Get("/api/browse-file", [&add_cors_headers](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        std::string filter = "Todos los Archivos\0*.*\0";
        if (req.has_param("ext")) {
            auto ext = req.get_param_value("ext");
            if (ext == "dll") {
                filter = "DLLs (*.dll)\0*.dll\0Todos los Archivos\0*.*\0";
            } else if (ext == "ct") {
                filter = "Cheat Tables (*.ct)\0*.ct\0Todos los Archivos\0*.*\0";
            }
        }
        std::string path = open_file_dialog(filter.c_str());
        json result;
        if (!path.empty()) {
            result["success"] = true;
            result["path"] = path;
        } else {
            result["success"] = false;
            result["message"] = "Dialogo cancelado por el usuario.";
        }
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    // Endpoint para seleccionar el proceso objetivo
    svr.Post("/api/select_process", [&](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        try {
            auto j = json::parse(req.body);
            DWORD pid = j.at("pid");
            engine.SelectProcess(pid);
            res.set_content("{\"success\": true, \"message\": \"Proceso seleccionado\"}", "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"JSON invalido\"}", "application/json");
        }
    });

    // Endpoint para activar/desactivar cheats
    svr.Post("/api/activate_cheat", [&](const httplib::Request& req, httplib::Response& res) {
        add_cors_headers(res);
        try {
            auto j = json::parse(req.body);
            std::string ctPath = j.at("ctPath");
            int entryId = j.at("entryId");
            bool activate = j.at("activate");
            res.set_content(engine.ActivateCheatEntry(ctPath, entryId, activate).dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"JSON invalido\"}", "application/json");
        }
    });

    svr.Get("/api/open_file_dialog", [&](const httplib::Request&, httplib::Response& res) {
        add_cors_headers(res);
        std::string filePath = open_file_dialog("Cheat Tables (*.ct)\0*.ct\0Todos los Archivos (*.*)\0*.*\0");
        json result;
        if (!filePath.empty()) {
            result["success"] = true;
            result["path"] = filePath;
        } else {
            result["success"] = false;
            result["message"] = "Dialogo cancelado por el usuario.";
        }
        res.set_content(result.dump(), "application/json; charset=utf-8");
    });

    std::string url = "http://localhost:" + std::to_string(SERVER_PORT);
    std::cout << "[SERVER] Iniciando servidor en " << url << std::endl;
    
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    svr.listen("0.0.0.0", SERVER_PORT);

    return 0;
}