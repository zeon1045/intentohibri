#include "../libs/httplib.h"
#include "../libs/json.hpp"
#include "injection_engine.h"
#include "driver_manager.h"
#include "ct_parser.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

using json = nlohmann::json;

struct Script {
    std::string name;
    bool active;
};

std::vector<Script> scripts = {
    {"Velocidad de Ataque", true},
    {"Daño Incrementado", false},
    {"AutoHeal", false},
    {"SpeedHack", false}
};

std::mutex scripts_mutex;

// Almacenamiento en memoria de la tabla cargada
auto current_cheat_table = std::make_shared<CheatTable>();

// Función para obtener la lista de procesos como un string JSON
std::string getProcessListAsJson() {
    // Implementación básica temporal - en main.cpp se usa directamente engine.GetProcessList()
    json j = json::array();
    j.push_back({{"pid", 0}, {"name", "Sistema"}, {"cpuUsage", 0.0}});
    return j.dump();
}

// Funciones auxiliares que pueden ser usadas por main.cpp
void setup_server_endpoints(httplib::Server& svr, const std::vector<std::string>& available_drivers) {
    // Endpoint para obtener el estado del motor
    svr.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        json response;
        response["status"] = "Motor activo y operativo";
        res.set_content(response.dump(), "application/json");
    });

    // Endpoint para obtener la lista de drivers
    svr.Get("/api/drivers", [&available_drivers](const httplib::Request&, httplib::Response& res) {
        json j = available_drivers;
        res.set_content(j.dump(), "application/json");
    });

    // Endpoint para obtener la lista de procesos
    svr.Get("/api/processes", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(getProcessListAsJson(), "application/json");
    });

    // Endpoint para obtener scripts disponibles (manteniendo compatibilidad)
    svr.Get("/api/scripts", [](const httplib::Request&, httplib::Response& res) {
        json response;
        response["scripts"] = json::array({
            {{"name", "Velocidad de Ataque"}, {"active", true}},
            {{"name", "Daño Incrementado"}, {"active", false}},
            {{"name", "AutoHeal"}, {"active", false}},
            {{"name", "SpeedHack"}, {"active", false}}
        });
        res.set_content(response.dump(), "application/json");
    });

    // Endpoint para toggle de scripts (manteniendo compatibilidad)
    svr.Post("/api/toggleScript", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            int index = body["index"];
            bool active = body["active"];
            // TODO: Implementar lógica real de toggle
            res.set_content("{\"result\":\"ok\"}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"Parámetros inválidos\"}", "application/json");
        }
    });

    // Endpoint para inyectar DLL
    svr.Post("/api/inject_dll", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            DWORD pid = body["pid"];
            std::string dllPath = body["dllPath"];
            
            // TODO: Usar injection_engine para inyectar la DLL
            json response;
            response["success"] = true;
            response["message"] = "DLL inyectada correctamente (simulado)";
            res.set_content(response.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"Error al inyectar DLL\"}", "application/json");
        }
    });

    // Endpoint para subir y parsear un archivo .ct
    svr.Post("/api/upload_ct", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_file("ct_file")) {
            const auto& file = req.get_file_value("ct_file");
            
            if (current_cheat_table->parse_from_xml(file.content)) {
                json j_entries = json::array();
                for (const auto& entry : current_cheat_table->entries) {
                    j_entries.push_back({
                        {"description", entry.description},
                        {"address", entry.address},
                        {"type", entry.type},
                        {"value", entry.value},
                        {"active", entry.active}
                    });
                }
                res.set_content(j_entries.dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content("{\"error\": \"Fallo al parsear el archivo .ct. Formato XML inválido o vacío.\"}", "application/json");
            }
        } else {
            res.status = 400;
            res.set_content("{\"error\": \"No se proporcionó ningún archivo.\"}", "application/json");
        }
    });

    // Endpoint para aplicar trucos (placeholder)
    svr.Post("/api/apply_cheats", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            DWORD processId = body["processId"];
            auto activeCheats = body["cheats"];
            
            // TODO: Implementar aplicación real de cheats usando injection_engine
            json response;
            response["success"] = true;
            response["message"] = "Trucos aplicados correctamente (simulado)";
            response["applied_count"] = activeCheats.size();
            res.set_content(response.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"Error al aplicar trucos\"}", "application/json");
        }
    });
    
    // Endpoint para obtener entradas de la tabla actual
    svr.Get("/api/cheat_entries", [&](const httplib::Request&, httplib::Response& res) {
        json j_entries = json::array();
        for (const auto& entry : current_cheat_table->entries) {
            j_entries.push_back({
                {"description", entry.description},
                {"address", entry.address},
                {"type", entry.type},
                {"value", entry.value},
                {"active", entry.active}
            });
        }
        res.set_content(j_entries.dump(), "application/json");
    });
}
