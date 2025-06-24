#pragma once
#include <string>
#include <vector>

namespace CTLoader {
    struct MemoryEntry {
        int id = 0;
        std::string description;
        std::string type;
        std::string address;
        std::string value;
        std::vector<MemoryEntry> children; // Para entradas anidadas
    };

    struct CheatTable {
        std::vector<MemoryEntry> entries;
    };

    class CTParser {
    public:
        // La única función pública: parsea desde un string de contenido
        bool parse(const std::string& content, CheatTable& out_table);
    private:
        // Función de ayuda recursiva
        void parseEntriesRecursive(const std::string& block, std::vector<MemoryEntry>& entries);
    };
} 