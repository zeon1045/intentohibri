#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

// Headers necesarios
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <regex>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define INVALID_SOCKET_FD INVALID_SOCKET
#define close_socket closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
using socket_t = int;
#define INVALID_SOCKET_FD -1
#define close_socket close
#endif

namespace httplib {

// Tipos básicos
using Headers = std::map<std::string, std::string>;
using Params = std::map<std::string, std::string>;

// Inicialización de Winsock para Windows
class WSAInitializer {
public:
    WSAInitializer() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    ~WSAInitializer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

static WSAInitializer wsa_init;

// Funciones auxiliares
inline std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline std::string url_decode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value);
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

// Estructuras básicas
struct Request {
    std::string method;
    std::string path;
    Headers headers;
    std::string body;
    Params params;
    std::map<std::string, std::string> files;
    
    bool has_file(const std::string& key) const { 
        return files.find(key) != files.end(); 
    }
    
    bool has_param(const std::string& key) const { 
        return params.find(key) != params.end(); 
    }
    
    std::string get_param_value(const std::string& key) const {
        auto it = params.find(key);
        return it != params.end() ? it->second : "";
    }
    
    struct FileValue {
        std::string content;
        std::string filename;
        std::string content_type;
    };
    
    FileValue get_file_value(const std::string& key) const {
        FileValue fv;
        auto it = files.find(key);
        if (it != files.end()) {
            fv.content = it->second;
            fv.filename = key;
            fv.content_type = "application/octet-stream";
        }
        return fv;
    }
};

struct Response {
    int status = 200;
    Headers headers;
    std::string body;
    
    void set_content(const std::string& content, const std::string& content_type) {
        body = content;
        headers["Content-Type"] = content_type;
    }
};

// Clase del servidor HTTP
class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;
    
    Server() : running_(false) {}
    virtual ~Server() { stop(); }
    
    Server& Get(const std::string& pattern, Handler handler) {
        get_handlers_[pattern] = handler;
        return *this;
    }
    
    Server& Post(const std::string& pattern, Handler handler) {
        post_handlers_[pattern] = handler;
        return *this;
    }
    
    bool set_base_dir(const std::string& dir) {
        base_dir_ = dir;
        return true;
    }
    
    bool set_mount_point(const std::string& mount_point, const std::string& dir) {
        base_dir_ = dir;
        return true;
    }
    
    bool listen(const std::string& host, int port) {
        if (running_) return false;
        
        socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == INVALID_SOCKET_FD) {
            std::cerr << "Error creating socket" << std::endl;
            return false;
        }
        
        // Permitir reutilizar la dirección
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error binding socket to port " << port << std::endl;
            close_socket(server_socket);
            return false;
        }
        
        if (::listen(server_socket, 10) < 0) {
            std::cerr << "Error listening on socket" << std::endl;
            close_socket(server_socket);
            return false;
        }
        
        running_ = true;
        server_socket_ = server_socket;
        
        std::cout << "[HTTP] Servidor HTTP iniciado en puerto " << port << std::endl;
        
        // Ejecutar el server loop directamente (bloqueante)
        this->server_loop();
        
        return true;
    }
    
    void stop() {
        if (running_) {
            running_ = false;
            if (server_socket_ != INVALID_SOCKET_FD) {
                close_socket(server_socket_);
                server_socket_ = INVALID_SOCKET_FD;
            }
            if (server_thread_.joinable()) {
                server_thread_.join();
            }
        }
    }
    
    bool is_running() const {
        return running_;
    }

private:
    void server_loop() {
        std::cout << "[HTTP] Iniciando bucle del servidor..." << std::endl;
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            
            socket_t client_socket = accept(server_socket_, (sockaddr*)&client_addr, &client_len);
            if (client_socket == INVALID_SOCKET_FD) {
                DWORD error = GetLastError();
                if (running_) {
                    std::cerr << "[HTTP] Error accepting connection: " << error << std::endl;
                    // En lugar de continuar indefinidamente, añadir un pequeño delay
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                } else {
                    // Si no está corriendo, salir del bucle
                    break;
                }
            }
            
            std::cout << "[HTTP] Conexión aceptada desde " << inet_ntoa(client_addr.sin_addr) << std::endl;
            
            // Procesar la solicitud en un thread separado
            std::thread([this, client_socket]() {
                try {
                    this->handle_client(client_socket);
                } catch (const std::exception& e) {
                    std::cerr << "[HTTP] Error handling client: " << e.what() << std::endl;
                }
                close_socket(client_socket);
            }).detach();
        }
        std::cout << "[HTTP] Servidor detenido." << std::endl;
    }
    
    void handle_client(socket_t client_socket) {
        char buffer[8192];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "[HTTP] Cliente desconectado o error en recv: " << GetLastError() << std::endl;
            return;
        }
        
        buffer[bytes_received] = '\0';
        std::string request_str(buffer);
        
        std::cout << "[HTTP] Solicitud recibida: " << request_str.substr(0, request_str.find('\n')) << std::endl;
        
        Request req;
        Response res;
        
        try {
            if (parse_request(request_str, req)) {
                handle_request(req, res);
            } else {
                res.status = 400;
                res.set_content("Bad Request", "text/plain");
                std::cout << "[HTTP] Error parsing request" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[HTTP] Error handling request: " << e.what() << std::endl;
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }
        
        send_response(client_socket, res);
        std::cout << "[HTTP] Respuesta enviada con código: " << res.status << std::endl;
    }
    
    bool parse_request(const std::string& request_str, Request& req) {
        auto lines = split(request_str, '\n');
        if (lines.empty()) return false;
        
        // Parse request line
        auto request_line = split(trim(lines[0]), ' ');
        if (request_line.size() < 3) return false;
        
        req.method = request_line[0];
        std::string full_path = request_line[1];
        
        // Parse path and query parameters
        size_t query_pos = full_path.find('?');
        if (query_pos != std::string::npos) {
            req.path = full_path.substr(0, query_pos);
            std::string query = full_path.substr(query_pos + 1);
            parse_query_params(query, req.params);
        } else {
            req.path = full_path;
        }
        
        // Parse headers
        size_t i = 1;
        for (; i < lines.size(); ++i) {
            std::string line = trim(lines[i]);
            if (line.empty()) break;
            
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = trim(line.substr(0, colon_pos));
                std::string value = trim(line.substr(colon_pos + 1));
                req.headers[key] = value;
            }
        }
        
        // Parse body
        if (i + 1 < lines.size()) {
            std::ostringstream body_stream;
            for (size_t j = i + 1; j < lines.size(); ++j) {
                if (j > i + 1) body_stream << '\n';
                body_stream << lines[j];
            }
            req.body = body_stream.str();
            
            // Parse form data if it's a POST request
            if (req.method == "POST") {
                auto content_type = req.headers.find("Content-Type");
                if (content_type != req.headers.end()) {
                    if (content_type->second.find("application/x-www-form-urlencoded") != std::string::npos) {
                        parse_query_params(req.body, req.params);
                    } else if (content_type->second.find("multipart/form-data") != std::string::npos) {
                        // Simplificado: tratar como archivo
                        req.files["ct_file"] = req.body;
                    }
                }
            }
        }
        
        return true;
    }
    
    void parse_query_params(const std::string& query, Params& params) {
        auto pairs = split(query, '&');
        for (const auto& pair : pairs) {
            size_t eq_pos = pair.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = url_decode(pair.substr(0, eq_pos));
                std::string value = url_decode(pair.substr(eq_pos + 1));
                params[key] = value;
            }
        }
    }
    
    void handle_request(const Request& req, Response& res) {
        // Buscar handler para la ruta
        auto& handlers = (req.method == "GET") ? get_handlers_ : post_handlers_;
        
        for (const auto& handler_pair : handlers) {
            if (req.path == handler_pair.first) {
                handler_pair.second(req, res);
                return;
            }
        }
        
        // Si no hay handler específico, servir archivos estáticos
        if (req.method == "GET" && !base_dir_.empty()) {
            serve_static_file(req.path, res);
            return;
        }
        
        // 404 Not Found
        res.status = 404;
        res.set_content("Not Found", "text/plain");
    }
    
    void serve_static_file(const std::string& path, Response& res) {
        std::string file_path = base_dir_ + path;
        if (path == "/") {
            file_path = base_dir_ + "/dashboard.html";
        }
        
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            res.status = 404;
            res.set_content("File not found", "text/plain");
            return;
        }
        
        std::ostringstream content_stream;
        content_stream << file.rdbuf();
        std::string content = content_stream.str();
        
        // Determinar content type
        std::string content_type = "text/plain";
        if (file_path.substr(file_path.find_last_of(".") + 1) == "html") content_type = "text/html";
        else if (file_path.substr(file_path.find_last_of(".") + 1) == "css") content_type = "text/css";
        else if (file_path.substr(file_path.find_last_of(".") + 1) == "js") content_type = "application/javascript";
        else if (file_path.substr(file_path.find_last_of(".") + 1) == "json") content_type = "application/json";
        
        res.set_content(content, content_type);
    }
    
    void send_response(socket_t client_socket, const Response& res) {
        std::ostringstream response_stream;
        response_stream << "HTTP/1.1 " << res.status << " ";
        
        switch (res.status) {
            case 200: response_stream << "OK"; break;
            case 400: response_stream << "Bad Request"; break;
            case 404: response_stream << "Not Found"; break;
            case 500: response_stream << "Internal Server Error"; break;
            default: response_stream << "Unknown"; break;
        }
        response_stream << "\r\n";
        
        // Headers CORS para permitir llamadas AJAX
        response_stream << "Access-Control-Allow-Origin: *\r\n";
        response_stream << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response_stream << "Access-Control-Allow-Headers: Content-Type\r\n";
        
        for (const auto& header : res.headers) {
            response_stream << header.first << ": " << header.second << "\r\n";
        }
        response_stream << "Content-Length: " << res.body.length() << "\r\n";
        response_stream << "Connection: close\r\n";
        response_stream << "\r\n";
        response_stream << res.body;
        
        std::string response_str = response_stream.str();
        int sent = send(client_socket, response_str.c_str(), (int)response_str.length(), 0);
        if (sent < 0) {
            std::cerr << "[HTTP] Error sending response: " << GetLastError() << std::endl;
        }
    }
    
    std::map<std::string, Handler> get_handlers_;
    std::map<std::string, Handler> post_handlers_;
    std::string base_dir_;
    std::atomic<bool> running_;
    socket_t server_socket_ = INVALID_SOCKET_FD;
    std::thread server_thread_;
};

// Clase básica del cliente
class Client {
public:
    struct Result {
        Response res;
        operator bool() const { return true; }
        const Response& value() const { return res; }
    };
    
    Client(const std::string& host, int port = 80) 
        : host_(host), port_(port) {}
    
    Result Get(const std::string& path) {
        Result result;
        result.res.status = 200;
        result.res.body = "GET " + path;
        return result;
    }
    
    Result Post(const std::string& path, const std::string& body, const std::string& content_type) {
        Result result;
        result.res.status = 200;
        result.res.body = "POST " + path;
        return result;
    }
    
private:
    std::string host_;
    int port_;
};

} // namespace httplib

#endif // CPPHTTPLIB_HTTPLIB_H 