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

# Archivos fuente
set(SOURCES
    backend/main.cpp
    backend/server.cpp
    backend/injection_engine.cpp
    backend/driver_manager.cpp
    backend/ct_parser.cpp
    backend/privilege_manager.cpp
)

# Crear ejecutable
add_executable(Belzebub ${SOURCES})

# Dependiendo de tu sistema, puede que necesites estas librerías para networking
find_library(WS2_32_LIBRARY ws2_32)
if(WS2_32_LIBRARY)
    target_link_libraries(Belzebub ${WS2_32_LIBRARY})
endif()

find_library(NTDLL_LIBRARY ntdll)
if(NTDLL_LIBRARY)
    target_link_libraries(Belzebub ${NTDLL_LIBRARY})
endif()

# Librerías adicionales para información de procesos y privilegios
target_link_libraries(Belzebub psapi advapi32)

