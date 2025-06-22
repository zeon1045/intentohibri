#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <shellapi.h>
#include <vector>
#include <json/json.h> // Necesitaremos para JSON (o podemos usar manual)
#include "injection_engine.h"

// Variables globales para el estado del sistema
std::vector<std::string> g_availableDrivers;
std::string g_currentDriver = "";
std::string g_currentProcess = "";

// Declaración del manejador de comandos que recibirá mensajes de la web
void HandleWebCommand(const std::string& jsonCommand);

// Función para generar el HTML dinámicamente con selección de drivers
std::string GenerateWebAppHTML();

// Nueva función para procesar comandos JSON simples sin librería
std::string ProcessSimpleCommand(const std::string& command, InjectionEngine& engine);

int main() {
    // 1. Asignar una consola para ver los logs del backend.
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "=========================================\n";
    std::cout << "  BYOVD Professional Backend v2.0\n";
    std::cout << "  Sistema con Cheat Engine Real\n";
    std::cout << "=========================================\n" << std::endl;
    
    // **🔥 CREAR INSTANCIA DEL MOTOR DE INYECCIÓN**
    InjectionEngine engine;
    std::cout << "[INIT] Motor de inyección inicializado con " << engine.GetDriverCount() << " drivers en base de datos\n";

    // 2. Detectar drivers disponibles automáticamente
    g_availableDrivers = engine.ListAvailableDrivers();
    
    if (g_availableDrivers.empty()) {
        std::cout << "[WARNING] No hay drivers disponibles. Coloca archivos .sys en la carpeta drivers/\n";
    } else {
        std::cout << "[INFO] Drivers detectados:\n";
        for (size_t i = 0; i < g_availableDrivers.size(); i++) {
            std::cout << "  - " << g_availableDrivers[i] << std::endl;
        }
    }

    // 3. Generar y lanzar la interfaz web actualizada
    std::ofstream htmlFile("..\\frontend\\dashboard.html");
    htmlFile << GenerateWebAppHTML();
    htmlFile.close();
    
    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);
    std::string htmlPath = std::string(currentDir) + "\\..\\frontend\\dashboard.html";

    ShellExecuteA(NULL, "open", htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    std::cout << "[INIT] Interfaz (frontend) lanzada en el navegador." << std::endl;
    std::cout << "[INIT] Backend listo para recibir comandos." << std::endl;

    // 4. Bucle del Servidor Simulado mejorado
    std::cout << "[SERVER] Servidor de comandos interactivo iniciado" << std::endl;
    std::cout << "[INFO] Usando Cheat Engine real para inyección" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "COMANDOS DISPONIBLES:" << std::endl;
    std::cout << "  list_drivers    - Listar drivers disponibles" << std::endl;
    std::cout << "  load <index>    - Cargar driver por índice" << std::endl;
    std::cout << "  unload         - Descargar driver actual" << std::endl;
    std::cout << "  inject <pid> <dll> - Inyectar DLL usando CE" << std::endl;
    std::cout << "  info <index>   - Información del driver" << std::endl;
    std::cout << "  load_ct <pid> <ct_path> - Cargar Cheat Table" << std::endl;
    std::cout << "  exit           - Salir" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::string inputCommand;
    while (true) {
        std::cout << "\nBYOVD> ";
        std::getline(std::cin, inputCommand);
        
        if (inputCommand == "exit") {
            std::cout << "[SERVER] Cerrando backend..." << std::endl;
            break;
        }
        
        std::string response = ProcessSimpleCommand(inputCommand, engine);
        if (!response.empty()) {
            std::cout << "[RESPONSE] " << response << std::endl;
        }
    }

    // 5. Limpieza al salir
    if (engine.IsDriverLoaded()) {
        std::cout << "[CLEANUP] Descargando driver antes de salir..." << std::endl;
        engine.UnloadDriver();
    }
    
    std::cout << "[EXIT] Backend cerrado correctamente." << std::endl;
    return 0;
}

// **🔥 PROCESADOR DE COMANDOS CON CHEAT ENGINE REAL**
std::string ProcessSimpleCommand(const std::string& command, InjectionEngine& engine) {
    if (command == "list_drivers") {
        std::string result = "=== DRIVERS DISPONIBLES ===\n";
        result += "Base de datos interna: " + std::to_string(engine.GetDriverCount()) + " drivers\n";
        result += "Driver actual: " + engine.GetCurrentDriverName() + "\n\n";
        
        for (size_t i = 0; i < engine.GetDriverCount(); i++) {
            result += "[" + std::to_string(i) + "] " + engine.GetDriverInfo(i) + "\n";
        }
        return result;
    }
    
    if (command.substr(0, 4) == "load") {
        if (command.length() <= 5) return "Uso: load <índice_driver>";
        
        try {
            int driverIndex = std::stoi(command.substr(5));
            if (engine.LoadDriver(driverIndex)) {
                return "Driver cargado exitosamente: " + engine.GetCurrentDriverName();
            } else {
                return "Error al cargar driver con índice: " + std::to_string(driverIndex);
            }
        } catch (const std::exception& e) {
            return "Índice inválido. Usa 'list_drivers' para ver índices disponibles.";
        }
    }
    
    if (command == "unload") {
        if (!engine.IsDriverLoaded()) return "No hay driver cargado";
        
        std::string currentDriver = engine.GetCurrentDriverName();
        if (engine.UnloadDriver()) {
            return "Driver descargado exitosamente: " + currentDriver;
        } else {
            return "Error al descargar driver";
        }
    }
    
    if (command.substr(0, 4) == "info") {
        if (command.length() <= 5) return "Uso: info <índice>";
        
        try {
            int driverIndex = std::stoi(command.substr(5));
            return engine.GetDriverInfo(driverIndex);
        } catch (const std::exception& e) {
            return "Índice inválido. Usa 'list_drivers' para ver índices disponibles.";
        }
    }
    
    if (command.substr(0, 6) == "inject") {
        // **🔥 INYECCIÓN REAL CON CHEAT ENGINE**
        // Formato: inject <PID> <DLL_PATH>
        std::istringstream iss(command);
        std::string cmd, pidStr, dllPath;
        iss >> cmd >> pidStr >> dllPath;
        
        if (pidStr.empty() || dllPath.empty()) {
            return "Uso: inject <PID> <ruta_dll>\nEjemplo: inject 1234 C:\\test.dll";
        }
        
        try {
            DWORD processId = std::stoul(pidStr);
            if (engine.InjectDLL(processId, dllPath)) {
                return "✅ Inyección completada exitosamente usando Cheat Engine!";
            } else {
                return "❌ Error en la inyección. Verifica PID y DLL.";
            }
        } catch (const std::exception& e) {
            return "PID inválido: " + pidStr;
        }
    }
    
    if (command.substr(0, 4) == "scan") {
        // **🔥 ESCANEO DE MEMORIA CON CHEAT ENGINE**
        // Formato: scan <PID> <valor> <tipo>
        std::istringstream iss(command);
        std::string cmd, pidStr, value, type;
        iss >> cmd >> pidStr >> value >> type;
        
        if (pidStr.empty() || value.empty()) {
            return "Uso: scan <PID> <valor> [tipo]\nEjemplo: scan 1234 100 int";
        }
        
        if (type.empty()) type = "int";
        
        try {
            DWORD processId = std::stoul(pidStr);
            if (engine.ScanMemory(processId, value, type)) {
                return "✅ Escaneo de memoria completado usando Cheat Engine!";
            } else {
                return "❌ Error en el escaneo de memoria.";
            }
        } catch (const std::exception& e) {
            return "PID inválido: " + pidStr;
        }
    }
    
    if (command == "status") {
        std::string result = "=== ESTADO DEL SISTEMA ===\n";
        result += "Driver cargado: " + (engine.IsDriverLoaded() ? "✅ SÍ" : "❌ NO") + "\n";
        result += "Driver actual: " + engine.GetCurrentDriverName() + "\n";
        result += "Cheat Engine: ✅ DISPONIBLE\n";
        result += "Componentes CE: core_dlls/cheatengine-x86_64.exe\n";
        return result;
    }
    
    if (command.substr(0, 8) == "load_ct ") {
        // **🎯 NUEVA FUNCIONALIDAD - Carga de Cheat Tables (.CT)**
        // Formato: load_ct <PID> <CT_FILE_PATH>
        std::istringstream iss(command);
        std::string cmd, pidStr, ctPath;
        iss >> cmd >> pidStr >> ctPath;
        
        if (pidStr.empty() || ctPath.empty()) {
            return "Uso: load_ct <PID> <archivo.ct>\nEjemplo: load_ct 1234 C:\\cheats\\game.ct";
        }
        
        try {
            DWORD processId = std::stoul(pidStr);
            if (engine.LoadCheatTable(ctPath, processId)) {
                return "✅ Cheat Table cargado exitosamente en Cheat Engine!\n"
                       "📋 CE se abrió con el archivo .CT cargado\n"
                       "🎮 Puedes activar/desactivar cheats desde la interfaz de CE\n"
                       "🔧 El driver BYOVD sigue activo para máximo rendimiento";
            } else {
                return "❌ Error al cargar el Cheat Table. Verifica la ruta y el PID.";
            }
        } catch (const std::exception& e) {
            return "PID inválido: " + pidStr;
        }
    }
    
    return "Comando no reconocido. Comandos: list_drivers, load, unload, info, inject, scan, load_ct, status, exit";
}

// Función para manejar comandos JSON (para futura implementación WebSocket)
void HandleWebCommand(const std::string& jsonCommand) {
    std::cout << "[WEB-CMD] Recibido: " << jsonCommand << std::endl;
    
    // Aquí implementaríamos el parsing JSON y las respuestas
    // Por ahora, solo logueamos
}

// Genera el HTML completo con sistema de selección de drivers
std::string GenerateWebAppHTML() {
    std::string html = R"(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>BYOVD Professional Suite v2.0</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0c0c0c, #1a1a1a);
            color: #ffffff;
            min-height: 100vh;
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .header {
            text-align: center;
            margin-bottom: 40px;
            padding: 30px;
            background: linear-gradient(135deg, #ff6b6b, #4ecdc4);
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        
        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }
        
        .header p {
            font-size: 1.2rem;
            opacity: 0.9;
        }
        
        .main-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 30px;
            margin-bottom: 30px;
        }
        
        .card {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.2);
            box-shadow: 0 8px 25px rgba(0,0,0,0.3);
        }
        
        .card h2 {
            color: #4ecdc4;
            margin-bottom: 20px;
            font-size: 1.5rem;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .driver-selection {
            grid-column: 1 / -1;
        }
        
        .driver-grid {
            display: grid;
            grid-template-columns: 2fr 1fr;
            gap: 20px;
            align-items: start;
        }
        
        .driver-list {
            background: rgba(0,0,0,0.3);
            border-radius: 10px;
            padding: 15px;
            max-height: 300px;
            overflow-y: auto;
        }
        
        .driver-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            margin: 5px 0;
            background: rgba(255,255,255,0.1);
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .driver-item:hover {
            background: rgba(76,205,196,0.2);
            transform: translateY(-2px);
        }
        
        .driver-item.selected {
            background: linear-gradient(135deg, #4ecdc4, #44a08d);
            box-shadow: 0 4px 15px rgba(76,205,196,0.3);
        }
        
        .driver-info {
            background: rgba(0,0,0,0.3);
            border-radius: 10px;
            padding: 15px;
            height: 300px;
            overflow-y: auto;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: #4ecdc4;
            font-weight: 600;
        }
        
        .form-group input, .form-group select {
            width: 100%;
            padding: 12px;
            border: none;
            border-radius: 8px;
            background: rgba(255,255,255,0.1);
            color: #ffffff;
            font-size: 1rem;
            border: 1px solid rgba(255,255,255,0.2);
        }
        
        .form-group input:focus, .form-group select:focus {
            outline: none;
            border-color: #4ecdc4;
            box-shadow: 0 0 10px rgba(76,205,196,0.3);
        }
        
        .btn {
            padding: 12px 25px;
            border: none;
            border-radius: 8px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #4ecdc4, #44a08d);
            color: white;
        }
        
        .btn-danger {
            background: linear-gradient(135deg, #ff6b6b, #ee5a52);
            color: white;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 20px rgba(0,0,0,0.3);
        }
        
        .btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }
        
        .status-bar {
            background: rgba(0,0,0,0.5);
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
            border-left: 4px solid #4ecdc4;
        }
        
        .log-area {
            background: rgba(0,0,0,0.7);
            border-radius: 10px;
            padding: 15px;
            height: 300px;
            overflow-y: auto;
            font-family: 'Courier New', monospace;
            font-size: 0.9rem;
            line-height: 1.4;
        }
        
        .log-entry {
            margin-bottom: 5px;
            padding: 2px 0;
        }
        
        .log-info { color: #4ecdc4; }
        .log-success { color: #2ecc71; }
        .log-warning { color: #f39c12; }
        .log-error { color: #e74c3c; }
        
        .footer {
            text-align: center;
            margin-top: 40px;
            padding: 20px;
            opacity: 0.7;
        }
        
        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(76,205,196,0.7); }
            70% { box-shadow: 0 0 0 10px rgba(76,205,196,0); }
            100% { box-shadow: 0 0 0 0 rgba(76,205,196,0); }
        }
        
        .status-active {
            animation: pulse 2s infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 BYOVD Professional Suite v2.0</h1>
            <p>Sistema Avanzado de Selección Múltiple de Drivers Vulnerables</p>
        </div>
        
        <div class="card driver-selection">
            <h2>🔧 Selección de Driver Vulnerable</h2>
            <div class="driver-grid">
                <div>
                    <h3 style="color: #4ecdc4; margin-bottom: 15px;">Drivers Disponibles:</h3>
                    <div class="driver-list" id="driverList">
                        <div class="log-entry log-info">Detectando drivers disponibles...</div>
                    </div>
                </div>
                <div>
                    <h3 style="color: #4ecdc4; margin-bottom: 15px;">Información del Driver:</h3>
                    <div class="driver-info" id="driverInfo">
                        <div class="log-entry">Selecciona un driver para ver información detallada.</div>
                    </div>
                </div>
            </div>
        </div>
        
        <div class="main-grid">
            <div class="card">
                <h2>⚙️ Control del Sistema</h2>
                <div class="form-group">
                    <label>Driver Seleccionado:</label>
                    <input type="text" id="selectedDriver" readonly placeholder="Ningún driver seleccionado">
                </div>
                <div class="form-group" style="display: flex; gap: 10px;">
                    <button class="btn btn-primary" id="loadDriverBtn" onclick="loadSelectedDriver()" disabled>
                        Cargar Driver
                    </button>
                    <button class="btn btn-danger" id="unloadDriverBtn" onclick="unloadDriver()" disabled>
                        Descargar Driver
                    </button>
                </div>
            </div>
            
            <div class="card">
                <h2>🎯 Inyección de Proceso</h2>
                <div class="form-group">
                    <label>Proceso Objetivo:</label>
                    <input type="text" id="targetProcess" placeholder="ejemplo: notepad.exe">
                </div>
                <div class="form-group">
                    <label>DLL a Inyectar:</label>
                    <input type="text" id="dllPath" placeholder="Ruta completa a la DLL">
                </div>
                <button class="btn btn-primary" onclick="injectDLL()" disabled id="injectBtn">
                    Ejecutar Inyección
                </button>
            </div>
        </div>
        
        <div class="card">
            <h2>📊 Estado del Sistema</h2>
            <div class="status-bar">
                <div><strong>Estado:</strong> <span id="systemStatus">Iniciando...</span></div>
                <div><strong>Driver Actual:</strong> <span id="currentDriverStatus">Ninguno</span></div>
                <div><strong>Última Operación:</strong> <span id="lastOperation">Sistema iniciado</span></div>
            </div>
        </div>
        
        <div class="card">
            <h2>📝 Registro de Actividad</h2>
            <div class="log-area" id="logArea">
                <div class="log-entry log-info">[INIT] BYOVD Professional Suite v2.0 iniciado</div>
                <div class="log-entry log-info">[INIT] Detectando drivers disponibles...</div>
            </div>
        </div>
        
        <div class="footer">
            <p>BYOVD Professional Suite v2.0 - Sistema de Inyección Avanzado</p>
            <p style="font-size: 0.9rem; margin-top: 5px; opacity: 0.7;">
                ⚠️ Solo para uso educativo y en sistemas propios
            </p>
        </div>
    </div>
    
    <script>
        let selectedDriver = null;
        let currentDriverLoaded = null;
        let systemStatus = 'ready';
        
        // Simular detección de drivers (en aplicación real vendría del backend)
        const availableDrivers = [)";

    // Insertar la lista de drivers disponibles dinámicamente
    html += "\n            ";
    for (size_t i = 0; i < g_availableDrivers.size(); i++) {
        if (i > 0) html += ",\n            ";
        html += "{ name: '" + g_availableDrivers[i] + "', info: '" + 
                InjectionEngine::GetDriverInfo(g_availableDrivers[i]) + "' }";
    }
    
    html += R"(
        ];
        
        function initializeInterface() {
            populateDriverList();
            updateSystemStatus('Listo para operación', 'ready');
            logMessage('info', '[INIT] Interfaz inicializada correctamente');
        }
        
        function populateDriverList() {
            const driverList = document.getElementById('driverList');
            driverList.innerHTML = '';
            
            if (availableDrivers.length === 0) {
                driverList.innerHTML = '<div class="log-entry log-warning">No se encontraron drivers .sys en la carpeta drivers/</div>';
                return;
            }
            
            availableDrivers.forEach((driver, index) => {
                const driverItem = document.createElement('div');
                driverItem.className = 'driver-item';
                driverItem.innerHTML = `
                    <div>
                        <strong>${driver.name}</strong>
                        <div style="font-size: 0.8rem; opacity: 0.7; margin-top: 2px;">
                            ${driver.info.split(' - ')[0]}
                        </div>
                    </div>
                    <div style="font-size: 0.8rem; opacity: 0.6;">
                        .sys
                    </div>
                `;
                
                driverItem.onclick = () => selectDriver(driver, driverItem);
                driverList.appendChild(driverItem);
            });
            
            logMessage('info', `[DRIVERS] Encontrados ${availableDrivers.length} drivers disponibles`);
        }
        
        function selectDriver(driver, element) {
            // Limpiar selección anterior
            document.querySelectorAll('.driver-item').forEach(item => {
                item.classList.remove('selected');
            });
            
            // Seleccionar nuevo driver
            element.classList.add('selected');
            selectedDriver = driver;
            
            // Actualizar UI
            document.getElementById('selectedDriver').value = driver.name;
            document.getElementById('driverInfo').innerHTML = `
                <div class="log-entry log-info"><strong>Driver:</strong> ${driver.name}.sys</div>
                <div class="log-entry"><strong>Descripción:</strong> ${driver.info}</div>
                <div class="log-entry"><strong>Estado:</strong> ${currentDriverLoaded === driver.name ? 'Cargado' : 'No cargado'}</div>
                <div class="log-entry"><strong>Ruta:</strong> drivers/${driver.name}.sys</div>
            `;
            
            // Habilitar botón de carga
            document.getElementById('loadDriverBtn').disabled = false;
            
            logMessage('info', `[SELECT] Driver seleccionado: ${driver.name}`);
        }
        
        function loadSelectedDriver() {
            if (!selectedDriver) {
                logMessage('error', '[ERROR] No hay driver seleccionado');
                return;
            }
            
            logMessage('info', `[LOAD] Cargando driver: ${selectedDriver.name}`);
            updateSystemStatus('Cargando driver...', 'loading');
            
            // Solicitar al backend que cargue el driver real
            fetch('/api/load_driver', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({driver: selectedDriver.name})
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    currentDriverLoaded = selectedDriver.name;
                    updateSystemStatus('Driver cargado correctamente', 'active');
                    document.getElementById('currentDriverStatus').textContent = selectedDriver.name;
                    document.getElementById('lastOperation').textContent = `Driver ${selectedDriver.name} cargado`;
                    
                    // Actualizar botones
                    document.getElementById('loadDriverBtn').disabled = true;
                    document.getElementById('unloadDriverBtn').disabled = false;
                    document.getElementById('injectBtn').disabled = false;
                    
                    logMessage('success', `[SUCCESS] ${data.message}`);
                } else {
                    logMessage('error', `[ERROR] ${data.message}`);
                    updateSystemStatus('Error al cargar driver', 'error');
                }
                
                // Actualizar info del driver
                selectDriver(selectedDriver, document.querySelector('.driver-item.selected'));
            })
            .catch(error => {
                logMessage('error', `[ERROR] Error de conexión: ${error.message}`);
                updateSystemStatus('Error de conexión', 'error');
            });
        }
        
        function unloadDriver() {
            if (!currentDriverLoaded) {
                logMessage('error', '[ERROR] No hay driver cargado');
                return;
            }
            
            logMessage('info', `[UNLOAD] Descargando driver: ${currentDriverLoaded}`);
            updateSystemStatus('Descargando driver...', 'loading');
            
            fetch('/api/unload_driver', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'}
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    const oldDriver = currentDriverLoaded;
                    currentDriverLoaded = null;
                    updateSystemStatus('Driver descargado', 'ready');
                    document.getElementById('currentDriverStatus').textContent = 'Ninguno';
                    document.getElementById('lastOperation').textContent = `Driver ${oldDriver} descargado`;
                    
                    // Actualizar botones
                    document.getElementById('loadDriverBtn').disabled = false;
                    document.getElementById('unloadDriverBtn').disabled = true;
                    document.getElementById('injectBtn').disabled = true;
                    
                    logMessage('success', `[SUCCESS] ${data.message}`);
                } else {
                    logMessage('error', `[ERROR] ${data.message}`);
                }
                
                // Actualizar info si el driver seleccionado era el descargado
                if (selectedDriver && selectedDriver.name === oldDriver) {
                    selectDriver(selectedDriver, document.querySelector('.driver-item.selected'));
                }
            })
            .catch(error => {
                logMessage('error', `[ERROR] Error de conexión: ${error.message}`);
            });
        }
        
        function injectDLL() {
            const targetProcess = document.getElementById('targetProcess').value;
            const dllPath = document.getElementById('dllPath').value;
            
            if (!targetProcess || !dllPath) {
                logMessage('error', '[ERROR] Debe especificar proceso objetivo y DLL');
                return;
            }
            
            if (!currentDriverLoaded) {
                logMessage('error', '[ERROR] Debe cargar un driver primero');
                return;
            }
            
            logMessage('info', `[INJECT] Iniciando inyección en ${targetProcess}`);
            logMessage('info', `[INJECT] DLL: ${dllPath}`);
            logMessage('info', `[INJECT] Driver: ${currentDriverLoaded}`);
            
            updateSystemStatus('Ejecutando inyección...', 'loading');
            
            fetch('/api/inject_dll', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({
                    process: targetProcess,
                    dll: dllPath,
                    driver: currentDriverLoaded
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    logMessage('success', `[SUCCESS] ${data.message}`);
                    updateSystemStatus('Inyección completada', 'active');
                    document.getElementById('lastOperation').textContent = `Inyección en ${targetProcess}`;
                } else {
                    logMessage('error', `[ERROR] ${data.message}`);
                    updateSystemStatus('Error en inyección', 'error');
                    document.getElementById('lastOperation').textContent = `Error en ${targetProcess}`;
                }
            })
            .catch(error => {
                logMessage('error', `[ERROR] Error de conexión: ${error.message}`);
                updateSystemStatus('Error de conexión', 'error');
            });
        }
        
        function updateSystemStatus(message, status) {
            const statusElement = document.getElementById('systemStatus');
            statusElement.textContent = message;
            statusElement.className = `status-${status}`;
            
            if (status === 'active') {
                statusElement.classList.add('status-active');
            }
        }
        
        function logMessage(type, message) {
            const logArea = document.getElementById('logArea');
            const timestamp = new Date().toLocaleTimeString();
            const logEntry = document.createElement('div');
            logEntry.className = `log-entry log-${type}`;
            logEntry.textContent = `[${timestamp}] ${message}`;
            
            logArea.appendChild(logEntry);
            logArea.scrollTop = logArea.scrollHeight;
        }
        
        // Inicializar cuando se carga la página
        window.onload = initializeInterface;
        
        // Simular actualización periódica de estado
        setInterval(() => {
            if (systemStatus === 'active') {
                // Simular monitoreo del sistema
            }
        }, 5000);
    </script>
</body>
</html>)";

    return html;
} 