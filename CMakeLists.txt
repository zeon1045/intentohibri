cmake_minimum_required(VERSION 3.10)

# Proyecto BYOVD Professional Suite v3.0
project(BYOVDProfessional VERSION 3.0.0 LANGUAGES CXX)

# Configuración C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuración para Windows
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Directorios de include
include_directories(libs)

# Archivos fuente
set(SOURCES
    backend/main.cpp
    backend/injection_engine.cpp
)

# Archivos de cabecera
set(HEADERS
    backend/injection_engine.h
)

# Crear el ejecutable
add_executable(BYOVD_Professional_v3 ${SOURCES} ${HEADERS})

# Librerías del sistema Windows
if(WIN32)
    target_link_libraries(BYOVD_Professional_v3 
        ws2_32
        winmm
        psapi
        advapi32
        shell32
    )
endif()

# Configuración de compilación
if(MSVC)
    # Visual Studio
    target_compile_options(BYOVD_Professional_v3 PRIVATE /W3)
    set_property(TARGET BYOVD_Professional_v3 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    # GCC/MinGW
    target_compile_options(BYOVD_Professional_v3 PRIVATE 
        -Wall 
        -Wextra 
        -O2
        -static-libgcc 
        -static-libstdc++
    )
endif()

# Configurar directorio de salida
set_target_properties(BYOVD_Professional_v3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
    OUTPUT_NAME "BYOVD_Professional_v3"
)

# Mensaje de información
message(STATUS "=== BYOVD Professional Suite v3.0 ===")
message(STATUS "Compilador: ${CMAKE_CXX_COMPILER}")
message(STATUS "Sistema: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Arquitectura: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Directorio de salida: ${CMAKE_SOURCE_DIR}/build")

# Target personalizado para copiar archivos necesarios
add_custom_target(copy_resources ALL
    COMMENT "Copiando recursos necesarios..."
)

# Crear directorios necesarios
add_custom_command(TARGET copy_resources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/core_dlls"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/drivers"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/frontend"
    COMMENT "Creando estructura de directorios..."
)

# Copiar archivos core_dlls si existen
if(EXISTS "${CMAKE_SOURCE_DIR}/core_dlls")
    add_custom_command(TARGET copy_resources POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                "${CMAKE_SOURCE_DIR}/core_dlls" 
                "${CMAKE_SOURCE_DIR}/build/core_dlls"
        COMMENT "Copiando componentes de Cheat Engine..."
    )
endif()

# Copiar frontend si existe
if(EXISTS "${CMAKE_SOURCE_DIR}/frontend")
    add_custom_command(TARGET copy_resources POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                "${CMAKE_SOURCE_DIR}/frontend" 
                "${CMAKE_SOURCE_DIR}/build/frontend"
        COMMENT "Copiando interfaz web..."
    )
endif()

# Copiar drivers si existen
if(EXISTS "${CMAKE_SOURCE_DIR}/drivers")
    add_custom_command(TARGET copy_resources POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                "${CMAKE_SOURCE_DIR}/drivers" 
                "${CMAKE_SOURCE_DIR}/build/drivers"
        COMMENT "Copiando drivers disponibles..."
    )
endif()

# Mensaje final
add_custom_command(TARGET BYOVD_Professional_v3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "============================================="
    COMMAND ${CMAKE_COMMAND} -E echo "  BYOVD Professional Suite v3.0 - LISTO"
    COMMAND ${CMAKE_COMMAND} -E echo "============================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Ejecutable: build/BYOVD_Professional_v3.exe"
    COMMAND ${CMAKE_COMMAND} -E echo "URL: http://localhost:12345"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "NOTA: Ejecutar como ADMINISTRADOR"
    COMMAND ${CMAKE_COMMAND} -E echo ""
) 