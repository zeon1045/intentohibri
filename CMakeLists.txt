cmake_minimum_required(VERSION 3.10)
project(Belzebub_Suite)

# Usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static") # Enlace estático para portabilidad

# Incluir directorios con headers locales
include_directories(
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/libs
)

# --- DETECCIÓN AUTOMÁTICA DE ARCHIVOS FUENTE ---
# Esta línea busca todos los archivos .cpp dentro de la carpeta backend
# y los añade a la variable SOURCES. A prueba de futuros olvidos.
file(GLOB_RECURSE SOURCES "backend/*.cpp")

# Muestra los archivos encontrados para depuración
message(STATUS "Archivos fuente encontrados: ${SOURCES}")

# Crear el ejecutable principal con todos los fuentes encontrados
add_executable(Belzebub_Suite ${SOURCES})

# Enlazar las bibliotecas de Windows necesarias
target_link_libraries(Belzebub_Suite
    ws2_32      # Para networking (httplib)
    advapi32    # Para servicios y privilegios (SCM)
    shell32     # Para APIs de Shell de Windows
    psapi       # Para información de procesos
    pdh         # Para métricas de rendimiento (CPU)
    comdlg32    # Para diálogos comunes
)

