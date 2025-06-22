cmake_minimum_required(VERSION 3.10)

# Proyecto BYOVD Professional Suite v3.0
project(Belzebub VERSION 3.0.0 LANGUAGES CXX)

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
add_executable(Belzebub ${SOURCES} ${HEADERS})

# Librerías del sistema Windows
if(WIN32)
    target_link_libraries(Belzebub 
        ws2_32
        winmm
        psapi
        advapi32
        shell32
        comdlg32
        pdh
    )
endif()

# Configuración de compilación
if(MSVC)
    # Visual Studio
    target_compile_options(Belzebub PRIVATE /W3 /EHsc)
    set_property(TARGET Belzebub PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    # GCC/MinGW
    target_compile_options(Belzebub PRIVATE 
        -Wall 
        -Wextra 
        -O2
        -static-libgcc 
        -static-libstdc++
    )
endif()

# Configurar directorio de salida
set_target_properties(Belzebub PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
    OUTPUT_NAME "Belzebub"
)

# --- COMANDOS POST-COMPILACIÓN ---
# Estos comandos se ejecutarán DESPUÉS de que Belzebub.exe se haya creado exitosamente.
# Usamos cmake -E para comandos multiplataforma y robustos.
add_custom_command(TARGET Belzebub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/frontend" "${CMAKE_BINARY_DIR}/frontend"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/core_dlls" "${CMAKE_BINARY_DIR}/core_dlls"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/drivers" "${CMAKE_BINARY_DIR}/drivers"
    COMMENT "Copiando recursos a la carpeta 'build'..."
)

# Mensaje final que se mostrará
add_custom_command(TARGET Belzebub POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "============================================="
    COMMAND ${CMAKE_COMMAND} -E echo "  Belzebub - Professional Suite v3.0 - LISTO"
    COMMAND ${CMAKE_COMMAND} -E echo "============================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Ejecutable: build/Belzebub.exe"
    COMMAND ${CMAKE_COMMAND} -E echo "NOTA: Ejecutar como ADMINISTRADOR"
)