cmake_minimum_required(VERSION 3.10)
project(Belzebub)

# Usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static") # Enlace estático para portabilidad

# Incluir directorios con headers locales
include_directories(
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/libs
)

# --- Lista de TODOS los archivos fuente necesarios ---
set(SOURCES
    backend/main.cpp
    backend/injection_engine.cpp
    backend/privilege_manager.cpp  # <-- Crítico para privilegios
    backend/utils.cpp              # <-- Utilidades del sistema
    backend/ct_loader.cpp          # <-- Parser de Cheat Tables
)

# Crear ejecutable
add_executable(Belzebub ${SOURCES})

# Librerías de Windows necesarias para Belzebub Professional
target_link_libraries(Belzebub
    ws2_32      # Para networking (httplib)
    advapi32    # Para servicios y privilegios (SCM)
    shell32     # Para APIs de Shell de Windows
    psapi       # Para información de procesos
    pdh         # Para métricas de rendimiento (CPU)
    comdlg32    # Para diálogos comunes
)

