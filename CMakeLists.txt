cmake_minimum_required(VERSION 3.10)
project(Belzebub)

# Usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Incluir directorios con headers locales
include_directories(
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/libs
)

# Archivos fuente
set(SOURCES
    backend/server.cpp
)

# Crear ejecutable
add_executable(belzebub ${SOURCES})

