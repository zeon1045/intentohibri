#include "ct_loader.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

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

std::string extractTextBetween(const std::string& content, const std::string& start, const std::string& end) {
    size_t start_pos = content.find(start);
    if (start_pos == std::string::npos) return "";
    start_pos += start.length();
    size_t end_pos = content.find(end, start_pos);
    if (end_pos == std::string::npos) return "";
    return content.substr(start_pos, end_pos - start_pos);
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
    
    entry.description = ::CTLoader::extractTextBetween(entryXml, "<Description>", "</Description>");
    entry.address = ::CTLoader::extractTextBetween(entryXml, "<Address>", "</Address>");
    entry.type = ::CTLoader::extractTextBetween(entryXml, "<VariableType>", "</VariableType>");
    entry.value = ::CTLoader::extractTextBetween(entryXml, "<Value>", "</Value>");
    
    std::string enabledStr = ::CTLoader::extractTextBetween(entryXml, "<Active>", "</Active>");
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
    
    script.name = ::CTLoader::extractTextBetween(scriptXml, "<n>", "</n>");
    
    std::string enabledStr = ::CTLoader::extractTextBetween(scriptXml, "<Enabled>", "</Enabled>");
    script.enabled = (enabledStr == "1" || enabledStr == "true");
    
    // El código Lua está usualmente en base64
    std::string encodedCode = ::CTLoader::extractTextBetween(scriptXml, "<LuaScript>", "</LuaScript>");
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

bool CTParser::loadFromString(const std::string& content) {
    xmlContent = content;
    
    if (!XMLUtils::isValidXML(xmlContent)) {
        std::cerr << "Error: Contenido XML inválido" << std::endl;
        return false;
    }
    
    return parseXML();
}

bool CTParser::parseXML() {
    if (xmlContent.empty()) return false;
    
    // Parse basic table info
    table.title = ::CTLoader::extractTextBetween(xmlContent, "<Title>", "</Title>");
    table.game_name = ::CTLoader::extractTextBetween(xmlContent, "<GameName>", "</GameName>");
    table.version = ::CTLoader::extractTextBetween(xmlContent, "<Version>", "</Version>");
    
    // Parse memory entries usando la nueva función recursiva
    size_t entries_start = xmlContent.find("<CheatEntries>");
    if (entries_start != std::string::npos) {
        size_t entries_end = xmlContent.find("</CheatEntries>", entries_start);
        if (entries_end != std::string::npos) {
            std::string main_entries_block = xmlContent.substr(entries_start + 14, entries_end - (entries_start + 14));
            parseEntriesRecursive(main_entries_block, table.entries);
        }
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

// NUEVA FUNCIÓN RECURSIVA para parsear entradas anidadas
void CTParser::parseEntriesRecursive(const std::string& block, std::vector<MemoryEntry>& entries) {
    size_t current_pos = 0;
    while (true) {
        size_t entry_start = block.find("<CheatEntry>", current_pos);
        if (entry_start == std::string::npos) break;

        size_t entry_end = block.find("</CheatEntry>", entry_start);
        if (entry_end == std::string::npos) break;

        std::string entry_block = block.substr(entry_start + 12, entry_end - (entry_start + 12));
        
        MemoryEntry new_entry = {}; // Inicializar a cero/vacío
        
        std::string id_str = extractTextBetween(entry_block, "<ID>", "</ID>");
        if (!id_str.empty()) new_entry.id = std::stoi(id_str);

        std::string desc_raw = extractTextBetween(entry_block, "<Description>", "</Description>");
        if (desc_raw.length() > 1 && desc_raw.front() == '"') {
            new_entry.description = desc_raw.substr(1, desc_raw.length() - 2);
        } else {
            new_entry.description = desc_raw;
        }

        new_entry.type = extractTextBetween(entry_block, "<VariableType>", "</VariableType>");
        new_entry.address = extractTextBetween(entry_block, "<Address>", "</Address>");
        new_entry.value = extractTextBetween(entry_block, "<Value>", "</Value>");

        // Lógica recursiva: buscar entradas hijas DENTRO de la entrada actual
        size_t children_start = entry_block.find("<CheatEntries>");
        if (children_start != std::string::npos) {
            size_t children_end = entry_block.find("</CheatEntries>", children_start);
            if (children_end != std::string::npos) {
                std::string children_block = entry_block.substr(children_start + 14, children_end - (children_start + 14));
                parseEntriesRecursive(children_block, new_entry.children);
            }
        }
        
        entries.push_back(new_entry);
        current_pos = entry_end + 13;
    }
}

// NUEVA FUNCIÓN para parsear desde un string de contenido
CTError CTParser::parseFromString(const std::string& content, CheatTable& out_table) {
    if (!XMLUtils::isValidXML(content)) {
        return CT_INVALID_XML;
    }
    
    out_table = {}; // Limpiar la tabla de salida
    
    size_t entries_start = content.find("<CheatEntries>");
    if (entries_start == std::string::npos) {
        return CT_PARSE_ERROR; // No se encontró el bloque principal de entradas
    }
    size_t entries_end = content.find("</CheatEntries>", entries_start);
    if (entries_end == std::string::npos) {
        return CT_PARSE_ERROR;
    }

    std::string main_entries_block = content.substr(entries_start + 14, entries_end - (entries_start + 14));
    
    parseEntriesRecursive(main_entries_block, out_table.entries);

    if (out_table.entries.empty()) {
        return CT_PARSE_ERROR;
    }

    return CT_SUCCESS;
}

// La función original ahora usa la nueva lógica
CTError CTParser::parse(const std::string& filename, CheatTable& out_table) {
    std::ifstream file(filename);
    if (!file) {
        return CT_FILE_NOT_FOUND;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parseFromString(content, out_table);
}

} // namespace CTLoader 