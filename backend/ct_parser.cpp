#include "ct_parser.h"
#include <iostream>

// Función de ayuda para extraer el contenido entre etiquetas XML de forma simple
std::string getTagContent(const std::string& xml, const std::string& tagName, size_t& start_pos) {
    std::string startTag = "<" + tagName + ">";
    std::string endTag = "</" + tagName + ">";
    size_t tagStart = xml.find(startTag, start_pos);
    if (tagStart == std::string::npos) {
        return "";
    }

    size_t contentStart = tagStart + startTag.length();
    size_t contentEnd = xml.find(endTag, contentStart);
    if (contentEnd == std::string::npos) {
        return "";
    }
    
    start_pos = contentEnd + endTag.length();
    // Realiza un trim básico para eliminar espacios y saltos de línea
    size_t first = xml.find_first_not_of(" \n\r\t", contentStart);
    size_t last = xml.find_last_not_of(" \n\r\t", contentEnd - 1);
    if (first == std::string::npos || last == std::string::npos) return "";

    return xml.substr(first, last - first + 1);
}

bool CheatTable::parse_from_xml(const std::string& xml_content) {
    this->entries.clear();
    size_t search_pos = 0;

    while (true) {
        size_t entry_start = xml_content.find("<CheatEntry>", search_pos);
        if (entry_start == std::string::npos) {
            break;
        }

        CheatEntry entry;
        size_t current_pos = entry_start;
        
        entry.description = getTagContent(xml_content, "Description", current_pos);
        entry.address = getTagContent(xml_content, "Address", current_pos);
        entry.type = getTagContent(xml_content, "Type", current_pos);
        entry.value = getTagContent(xml_content, "Value", current_pos);

        // Solo se añade si la dirección y el tipo no están vacíos
        if (!entry.address.empty() && !entry.type.empty()) {
            this->entries.push_back(entry);
        }
        search_pos = current_pos;
    }

    std::cout << "Parseadas " << this->entries.size() << " entradas de la Cheat Table." << std::endl;
    return !this->entries.empty();
} 