#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

namespace CTLoader {
    
    // Códigos de error
    enum CTError {
        CT_SUCCESS = 0,
        CT_FILE_NOT_FOUND,
        CT_INVALID_XML,
        CT_PARSE_ERROR,
        CT_SCRIPT_ERROR,
        CT_MEMORY_ERROR,
        CT_PROCESS_ERROR
    };
    
    // Estructura para manejar entradas de memoria de .ct
    struct MemoryEntry {
        int id = 0;  // ID de la entrada (añadido)
        std::string description;
        std::string address;
        std::string type;
        std::string value;
        bool enabled;
        bool hotkey_enabled;
        std::string hotkey;
        std::vector<MemoryEntry> children;
    };
    
    // Estructura para scripts Lua
    struct LuaScript {
        std::string name;
        std::string code;
        bool enabled;
    };
    
    // Estructura para tabla .ct completa
    struct CheatTable {
        std::string title;
        std::string game_name;
        std::string version;
        std::vector<MemoryEntry> entries;
        std::vector<LuaScript> scripts;
        std::map<std::string, std::string> options;
    };
    
    class CTParser {
    private:
        std::string xmlContent;
        CheatTable table;
        
        std::vector<std::string> findAllBetween(const std::string& content, const std::string& start, const std::string& end);
        MemoryEntry parseMemoryEntry(const std::string& entryXml);
        LuaScript parseLuaScript(const std::string& scriptXml);
        std::string decodeBase64(const std::string& encoded);
        
        // Función de ayuda recursiva para parsear entradas anidadas
        void parseEntriesRecursive(const std::string& block, std::vector<MemoryEntry>& entries);
        
    public:
        CTParser() {}
        
        // --- FIRMAS DE FUNCIONES MODIFICADAS ---
        CTError parse(const std::string& filename, CheatTable& out_table);
        CTError parseFromString(const std::string& content, CheatTable& out_table);
        
        bool loadFromFile(const std::string& filePath);
        bool loadFromString(const std::string& content);
        bool parseXML();
        CheatTable& getTable() { return table; }
        
        // Funciones de utilidad
        void printTableInfo();
        std::vector<std::string> getScriptNames();
        std::vector<std::string> getEntryDescriptions();
    };
    
    class CTExecutor {
    private:
        CheatTable* table;
        HANDLE hProcess;
        DWORD processId;
        
        // Mini intérprete Lua simplificado
        struct LuaVariable {
            std::string name;
            std::string value;
            std::string type;
        };
        
        std::map<std::string, LuaVariable> luaVariables;
        
        bool executeLuaCommand(const std::string& command);
        bool parseAddress(const std::string& addressStr, PVOID& address);
        bool writeMemoryValue(PVOID address, const std::string& type, const std::string& value);
        bool readMemoryValue(PVOID address, const std::string& type, std::string& value);
        
    public:
        CTExecutor(CheatTable* ct, HANDLE process, DWORD pid) 
            : table(ct), hProcess(process), processId(pid) {}
        
        bool executeScript(const std::string& scriptName);
        bool enableEntry(const std::string& description);
        bool disableEntry(const std::string& description);
        bool setEntryValue(const std::string& description, const std::string& value);
        
        // Funciones principales
        bool activateTable();
        bool deactivateTable();
        void listActiveEntries();
    };
    
    // Utilidades para parsing XML simplificado
    namespace XMLUtils {
        std::string trim(const std::string& str);
        std::string unescapeXML(const std::string& str);
        std::map<std::string, std::string> parseAttributes(const std::string& tag);
        bool isValidXML(const std::string& content);
    }
    
    // Funciones de conversión de tipos
    namespace TypeConverter {
        bool stringToBytes(const std::string& value, const std::string& type, std::vector<BYTE>& bytes);
        bool bytesToString(const std::vector<BYTE>& bytes, const std::string& type, std::string& value);
        bool isValidType(const std::string& type);
        int getTypeSize(const std::string& type);
    }
    
    // Función independiente para extraer texto entre delimitadores
    std::string extractTextBetween(const std::string& content, const std::string& start, const std::string& end);
    
    const char* getErrorString(CTError error);
} 