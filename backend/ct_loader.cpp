#include "ct_loader.h"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace CTLoader {

// ============================================================================
// XMLUtils Implementation
// ============================================================================

namespace XMLUtils {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    std::string unescapeXML(const std::string& str) {
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find("&lt;", pos)) != std::string::npos) {
            result.replace(pos, 4, "<");
            pos += 1;
        }
        pos = 0;
        while ((pos = result.find("&gt;", pos)) != std::string::npos) {
            result.replace(pos, 4, ">");
            pos += 1;
        }
        pos = 0;
        while ((pos = result.find("&amp;", pos)) != std::string::npos) {
            result.replace(pos, 5, "&");
            pos += 1;
        }
        return result;
    }
    
    bool isValidXML(const std::string& content) {
        return content.find("<CheatTable") != std::string::npos;
    }
}

// ============================================================================
// TypeConverter Implementation
// ============================================================================

namespace TypeConverter {
    bool isValidType(const std::string& type) {
        std::string lowerType = type;
        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
        
        return (lowerType == "byte" || lowerType == "2 bytes" || lowerType == "4 bytes" ||
                lowerType == "8 bytes" || lowerType == "float" || lowerType == "double" ||
                lowerType == "string" || lowerType == "array of byte");
    }
    
    int getTypeSize(const std::string& type) {
        std::string lowerType = type;
        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
        
        if (lowerType == "byte") return 1;
        if (lowerType == "2 bytes") return 2;
        if (lowerType == "4 bytes") return 4;
        if (lowerType == "8 bytes") return 8;
        if (lowerType == "float") return 4;
        if (lowerType == "double") return 8;
        return 0; // Variable size for strings and arrays
    }
    
    bool stringToBytes(const std::string& value, const std::string& type, std::vector<BYTE>& bytes) {
        std::string lowerType = type;
        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
        
        try {
            if (lowerType == "byte") {
                int val = std::stoi(value);
                bytes.push_back(static_cast<BYTE>(val));
                return true;
            }
            else if (lowerType == "2 bytes") {
                int val = std::stoi(value);
                bytes.push_back(val & 0xFF);
                bytes.push_back((val >> 8) & 0xFF);
                return true;
            }
            else if (lowerType == "4 bytes") {
                long val = std::stol(value);
                bytes.push_back(val & 0xFF);
                bytes.push_back((val >> 8) & 0xFF);
                bytes.push_back((val >> 16) & 0xFF);
                bytes.push_back((val >> 24) & 0xFF);
                return true;
            }
            else if (lowerType == "8 bytes") {
                long long val = std::stoll(value);
                for (int i = 0; i < 8; i++) {
                    bytes.push_back((val >> (i * 8)) & 0xFF);
                }
                return true;
            }
            else if (lowerType == "float") {
                float val = std::stof(value);
                BYTE* ptr = reinterpret_cast<BYTE*>(&val);
                for (int i = 0; i < 4; i++) {
                    bytes.push_back(ptr[i]);
                }
                return true;
            }
            else if (lowerType == "double") {
                double val = std::stod(value);
                BYTE* ptr = reinterpret_cast<BYTE*>(&val);
                for (int i = 0; i < 8; i++) {
                    bytes.push_back(ptr[i]);
                }
                return true;
            }
        }
        catch (...) {
            return false;
        }
        
        return false;
    }
}

// ============================================================================
// CTParser Implementation
// ============================================================================

std::string CTParser::extractTextBetween(const std::string& content, const std::string& start, const std::string& end) {
    size_t startPos = content.find(start);
    if (startPos == std::string::npos) return "";
    
    startPos += start.length();
    size_t endPos = content.find(end, startPos);
    if (endPos == std::string::npos) return "";
    
    return content.substr(startPos, endPos - startPos);
}

std::vector<std::string> CTParser::findAllBetween(const std::string& content, const std::string& start, const std::string& end) {
    std::vector<std::string> results;
    size_t pos = 0;
    
    while ((pos = content.find(start, pos)) != std::string::npos) {
        size_t startPos = pos + start.length();
        size_t endPos = content.find(end, startPos);
        if (endPos == std::string::npos) break;
        
        results.push_back(content.substr(startPos, endPos - startPos));
        pos = endPos + end.length();
    }
    
    return results;
}

MemoryEntry CTParser::parseMemoryEntry(const std::string& entryXml) {
    MemoryEntry entry;
    
    entry.description = extractTextBetween(entryXml, "<Description>", "</Description>");
    entry.address = extractTextBetween(entryXml, "<Address>", "</Address>");
    entry.type = extractTextBetween(entryXml, "<VariableType>", "</VariableType>");
    entry.value = extractTextBetween(entryXml, "<Value>", "</Value>");
    
    std::string enabledStr = extractTextBetween(entryXml, "<Active>", "</Active>");
    entry.enabled = (enabledStr == "1" || enabledStr == "true");
    
    // Parse children entries
    std::vector<std::string> childEntries = findAllBetween(entryXml, "<CheatEntry>", "</CheatEntry>");
    for (const auto& childXml : childEntries) {
        entry.children.push_back(parseMemoryEntry(childXml));
    }
    
    return entry;
}

LuaScript CTParser::parseLuaScript(const std::string& scriptXml) {
    LuaScript script;
    
    script.name = extractTextBetween(scriptXml, "<Name>", "</Name>");
    
    std::string enabledStr = extractTextBetween(scriptXml, "<Enabled>", "</Enabled>");
    script.enabled = (enabledStr == "1" || enabledStr == "true");
    
    // El código Lua está usualmente en base64
    std::string encodedCode = extractTextBetween(scriptXml, "<LuaScript>", "</LuaScript>");
    script.code = decodeBase64(encodedCode);
    
    return script;
}

std::string CTParser::decodeBase64(const std::string& encoded) {
    // Implementación simplificada de decodificación base64
    // Para una implementación completa, usarías una librería específica
    return encoded; // Por ahora retorna el string tal como está
}

bool CTParser::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: No se puede abrir el archivo " << filePath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    xmlContent = buffer.str();
    file.close();
    
    if (!XMLUtils::isValidXML(xmlContent)) {
        std::cerr << "Error: Archivo XML inválido" << std::endl;
        return false;
    }
    
    return parseXML();
}

bool CTParser::parseXML() {
    if (xmlContent.empty()) return false;
    
    // Parse basic table info
    table.title = extractTextBetween(xmlContent, "<Title>", "</Title>");
    table.game_name = extractTextBetween(xmlContent, "<GameName>", "</GameName>");
    table.version = extractTextBetween(xmlContent, "<Version>", "</Version>");
    
    // Parse memory entries
    std::vector<std::string> entries = findAllBetween(xmlContent, "<CheatEntry>", "</CheatEntry>");
    for (const auto& entryXml : entries) {
        table.entries.push_back(parseMemoryEntry(entryXml));
    }
    
    // Parse Lua scripts
    std::vector<std::string> scripts = findAllBetween(xmlContent, "<CheatScript>", "</CheatScript>");
    for (const auto& scriptXml : scripts) {
        table.scripts.push_back(parseLuaScript(scriptXml));
    }
    
    return true;
}

void CTParser::printTableInfo() {
    std::cout << "=== Información de la tabla .ct ===" << std::endl;
    std::cout << "Título: " << table.title << std::endl;
    std::cout << "Juego: " << table.game_name << std::endl;
    std::cout << "Versión: " << table.version << std::endl;
    std::cout << "Entradas de memoria: " << table.entries.size() << std::endl;
    std::cout << "Scripts Lua: " << table.scripts.size() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Entradas disponibles:" << std::endl;
    for (size_t i = 0; i < table.entries.size(); i++) {
        std::cout << "  " << (i+1) << ". " << table.entries[i].description << std::endl;
        std::cout << "     Dirección: " << table.entries[i].address << std::endl;
        std::cout << "     Tipo: " << table.entries[i].type << std::endl;
        std::cout << "     Activo: " << (table.entries[i].enabled ? "Sí" : "No") << std::endl;
    }
    
    if (!table.scripts.empty()) {
        std::cout << std::endl << "Scripts Lua disponibles:" << std::endl;
        for (size_t i = 0; i < table.scripts.size(); i++) {
            std::cout << "  " << (i+1) << ". " << table.scripts[i].name << std::endl;
            std::cout << "     Activo: " << (table.scripts[i].enabled ? "Sí" : "No") << std::endl;
        }
    }
}

std::vector<std::string> CTParser::getScriptNames() {
    std::vector<std::string> names;
    for (const auto& script : table.scripts) {
        names.push_back(script.name);
    }
    return names;
}

std::vector<std::string> CTParser::getEntryDescriptions() {
    std::vector<std::string> descriptions;
    for (const auto& entry : table.entries) {
        descriptions.push_back(entry.description);
    }
    return descriptions;
}

// ============================================================================
// CTExecutor Implementation
// ============================================================================

bool CTExecutor::parseAddress(const std::string& addressStr, PVOID& address) {
    try {
        // Manejar diferentes formatos de dirección
        if (addressStr.find("0x") == 0 || addressStr.find("0X") == 0) {
            // Hexadecimal
            address = reinterpret_cast<PVOID>(std::stoull(addressStr, nullptr, 16));
            return true;
        } else {
            // Decimal
            address = reinterpret_cast<PVOID>(std::stoull(addressStr));
            return true;
        }
    } catch (...) {
        return false;
    }
}

bool CTExecutor::writeMemoryValue(PVOID address, const std::string& type, const std::string& value) {
    std::vector<BYTE> bytes;
    if (!TypeConverter::stringToBytes(value, type, bytes)) {
        return false;
    }
    
    SIZE_T bytesWritten;
    BOOL result = WriteProcessMemory(hProcess, address, bytes.data(), bytes.size(), &bytesWritten);
    return result && (bytesWritten == bytes.size());
}

bool CTExecutor::enableEntry(const std::string& description) {
    for (auto& entry : table->entries) {
        if (entry.description == description) {
            PVOID address;
            if (!parseAddress(entry.address, address)) {
                std::cerr << "Error: Dirección inválida para " << description << std::endl;
                return false;
            }
            
            if (writeMemoryValue(address, entry.type, entry.value)) {
                entry.enabled = true;
                std::cout << "[+] Activado: " << description << std::endl;
                return true;
            } else {
                std::cerr << "[-] Error escribiendo memoria para " << description << std::endl;
                return false;
            }
        }
    }
    
    std::cerr << "[-] Entrada no encontrada: " << description << std::endl;
    return false;
}

bool CTExecutor::executeScript(const std::string& scriptName) {
    for (const auto& script : table->scripts) {
        if (script.name == scriptName) {
            std::cout << "[+] Ejecutando script: " << scriptName << std::endl;
            std::cout << "[!] Ejecución de scripts Lua limitada en esta versión" << std::endl;
            
            // Aquí implementarías la ejecución real del script Lua
            // Por ahora solo mostramos el código
            std::cout << "Código del script:" << std::endl;
            std::cout << script.code << std::endl;
            
            return true;
        }
    }
    
    std::cerr << "[-] Script no encontrado: " << scriptName << std::endl;
    return false;
}

bool CTExecutor::activateTable() {
    std::cout << "[+] Activando tabla de cheats..." << std::endl;
    
    int activated = 0;
    for (auto& entry : table->entries) {
        if (entry.enabled) {
            if (enableEntry(entry.description)) {
                activated++;
            }
        }
    }
    
    std::cout << "[+] Activadas " << activated << " entradas de " << table->entries.size() << std::endl;
    return activated > 0;
}

void CTExecutor::listActiveEntries() {
    std::cout << "=== Entradas activas ===" << std::endl;
    for (const auto& entry : table->entries) {
        if (entry.enabled) {
            std::cout << "✓ " << entry.description << " = " << entry.value << std::endl;
        }
    }
}

// ============================================================================
// Error handling
// ============================================================================

const char* getErrorString(CTError error) {
    switch (error) {
        case CT_SUCCESS: return "Éxito";
        case CT_FILE_NOT_FOUND: return "Archivo no encontrado";
        case CT_INVALID_XML: return "XML inválido";
        case CT_PARSE_ERROR: return "Error de parsing";
        case CT_SCRIPT_ERROR: return "Error en script";
        case CT_MEMORY_ERROR: return "Error de memoria";
        case CT_PROCESS_ERROR: return "Error de proceso";
        default: return "Error desconocido";
    }
}

} // namespace CTLoader 