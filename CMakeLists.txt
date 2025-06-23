cmake_minimum_required(VERSION 3.10)
project(Belzebub)

# Usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir directorios con headers locales
include_directories(
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/libs
)

# Archivos fuente - SOLO LOS NECESARIOS
set(SOURCES
    backend/main.cpp
    backend/injection_engine.cpp
    backend/privilege_manager.cpp
    backend/utils.cpp
)

# Crear ejecutable
add_executable(Belzebub ${SOURCES})

# Librerías de Windows necesarias para Belzebub
target_link_libraries(Belzebub
    ws2_32      # Para networking (httplib)
    advapi32    # Para servicios y privilegios (SCM)
    shell32     # Para APIs de Shell de Windows
    psapi       # Para información de procesos
    pdh         # Para métricas de rendimiento (CPU)
    comdlg32    # Para diálogos comunes
)

