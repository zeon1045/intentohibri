#include "driver_manager.h"
#include <windows.h>
#include <iostream>
#include <string>

std::vector<std::string> list_available_drivers() {
    std::vector<std::string> drivers;
    WIN32_FIND_DATAA findFileData;
    // La ruta es relativa al directorio de compilación (build)
    HANDLE hFind = FindFirstFileA("../drivers/*.sys", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "ADVERTENCIA: No se encontraron drivers (.sys) en el directorio 'drivers/' o el directorio no existe." << std::endl;
        return drivers;
    }

    do {
        std::cout << "Driver encontrado: " << findFileData.cFileName << std::endl;
        drivers.push_back(findFileData.cFileName);
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    return drivers;
} 