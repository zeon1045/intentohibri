#include "ct_loader.h"
#include <iostream>

namespace CTLoader {

    // Función de ayuda para extraer texto
    std::string extractTextBetween(const std::string& content, const std::string& start, const std::string& end) {
        size_t start_pos = content.find(start);
        if (start_pos == std::string::npos) return "";
        start_pos += start.length();
        size_t end_pos = content.find(end, start_pos);
        if (end_pos == std::string::npos) return "";
        std::string result = content.substr(start_pos, end_pos - start_pos);
        // Quitar comillas si existen
        if (result.length() > 1 && result.front() == '"' && result.back() == '"') {
            return result.substr(1, result.length() - 2);
        }
        return result;
    }
    
    // Función recursiva que es la clave para parsear correctamente
    void CTParser::parseEntriesRecursive(const std::string& block, std::vector<MemoryEntry>& entries) {
        size_t current_pos = 0;
        while (true) {
            size_t entry_start = block.find("<CheatEntry>", current_pos);
            if (entry_start == std::string::npos) break;

            size_t entry_end = block.find("</CheatEntry>", entry_start);
            if (entry_end == std::string::npos) break;

            std::string entry_block = block.substr(entry_start + 12, entry_end - (entry_start + 12));
            
            MemoryEntry new_entry = {};
            
            std::string id_str = extractTextBetween(entry_block, "<ID>", "</ID>");
            if (!id_str.empty()) new_entry.id = std::stoi(id_str);
            new_entry.description = extractTextBetween(entry_block, "<Description>", "</Description>");
            new_entry.type = extractTextBetween(entry_block, "<VariableType>", "</VariableType>");
            new_entry.address = extractTextBetween(entry_block, "<Address>", "</Address>");
            new_entry.value = extractTextBetween(entry_block, "<Value>", "</Value>");

            // Lógica recursiva: buscar un bloque <CheatEntries> HIJO
            size_t children_start = entry_block.find("<CheatEntries>");
            if (children_start != std::string::npos) {
                size_t children_end = entry_block.find("</CheatEntries>", children_start);
                if (children_end != std::string::npos) {
                    std::string children_block = entry_block.substr(children_start + 14, children_end - (children_start + 14));
                    parseEntriesRecursive(children_block, new_entry.children); // Llamada recursiva
                }
            }
            
            entries.push_back(new_entry);
            current_pos = entry_end + 13;
        }
    }
    
    // Función principal que inicia el proceso
    bool CTParser::parse(const std::string& content, CheatTable& out_table) {
        out_table.entries.clear();
        
        size_t entries_start = content.find("<CheatEntries>");
        if (entries_start == std::string::npos) return false;
        
        size_t entries_end = content.rfind("</CheatEntries>");
        if (entries_end == std::string::npos) return false;

        std::string main_block = content.substr(entries_start + 14, entries_end - (entries_start + 14));
        
        parseEntriesRecursive(main_block, out_table.entries);

        return !out_table.entries.empty();
    }

} // namespace CTLoader 