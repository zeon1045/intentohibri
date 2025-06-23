#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include <vector>
#include <string>

// Escanea el directorio ../drivers/ en busca de archivos .sys
std::vector<std::string> list_available_drivers();

#endif // DRIVER_MANAGER_H 