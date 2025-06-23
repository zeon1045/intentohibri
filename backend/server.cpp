#include "httplib.h"
#include "json.hpp"
#include <vector>
#include <mutex>

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

int main() {
    httplib::Server svr;

    svr.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        json response;
        response["status"] = "Motor activo y operativo";
        res.set_content(response.dump(), "application/json");
    });

    svr.Get("/api/scripts", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(scripts_mutex);
        json response;
        for (const auto& script : scripts) {
            response["scripts"].push_back({
                {"name", script.name},
                {"active", script.active}
            });
        }
        res.set_content(response.dump(), "application/json");
    });

    svr.Post("/api/toggleScript", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(scripts_mutex);
        try {
            auto body = json::parse(req.body);
            int index = body["index"];
            bool active = body["active"];
            if (index >= 0 && index < scripts.size()) {
                scripts[index].active = active;
            }
            res.set_content("{\"result\":\"ok\"}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"Parámetros inválidos\"}", "application/json");
        }
    });

    svr.listen("localhost", 8080);
}
