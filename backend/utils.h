#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <windows.h>

// Devuelve el directorio donde se encuentra el ejecutable actual.
std::string get_executable_directory();

// Analiza la cabecera PE de un archivo para determinar su arquitectura.
// Devuelve 64, 32, o 0 si hay un error o es desconocido.
int get_file_architecture(const std::string& file_path);

#endif // UTILS_H 