#include "utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>

std::string get_executable_directory() {
    char path_buffer[MAX_PATH];
    GetModuleFileNameA(NULL, path_buffer, MAX_PATH);
    return std::filesystem::path(path_buffer).parent_path().string();
}

int get_file_architecture(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return 0; // No se pudo abrir el archivo
    }

    IMAGE_DOS_HEADER dos_header;
    file.read(reinterpret_cast<char*>(&dos_header), sizeof(dos_header));
    if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
        return 0; // No es un archivo PE válido
    }

    file.seekg(dos_header.e_lfanew, std::ios::beg);
    IMAGE_NT_HEADERS nt_headers;
    file.read(reinterpret_cast<char*>(&nt_headers), sizeof(nt_headers));
    if (nt_headers.Signature != IMAGE_NT_SIGNATURE) {
        return 0; // No es una firma NT válida
    }

    if (nt_headers.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
        return 64;
    }
    if (nt_headers.FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
        return 32;
    }

    return 0; // Arquitectura desconocida
} 