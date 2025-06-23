#ifndef CT_PARSER_H
#define CT_PARSER_H

#include <string>
#include <vector>

// Estructura para representar una entrada en la Cheat Table
struct CheatEntry {
    std::string description;
    std::string address; // Se mantiene como string para manejar punteros complejos
    std::string type;
    std::string value;
    bool active = false; // Para que el usuario la active/desactive desde el frontend
};

// Clase para manejar la tabla de trucos
class CheatTable {
public:
    std::vector<CheatEntry> entries;

    // Parsea el contenido XML de un archivo .ct
    // Devuelve 'true' si el parseo fue exitoso
    bool parse_from_xml(const std::string& xml_content);
};

#endif // CT_PARSER_H 