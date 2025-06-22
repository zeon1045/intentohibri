# 🚀 BYOVD Professional Suite v2.0

## Descripción

**BYOVD Professional Suite v2.0** es una aplicación completa que implementa el método **BYOVD (Bring Your Own Vulnerable Driver)** con un innovador **sistema de selección múltiple de drivers vulnerables** y una interfaz web moderna y profesional.

### 🆕 Nuevas Características v2.0:

- ✅ **Sistema de Selección Múltiple** - Detecta automáticamente todos los drivers disponibles
- ✅ **Interfaz web completamente renovada** con diseño moderno y intuitivo
- ✅ **Base de datos integrada** con información detallada de cada driver
- ✅ **Detección automática** de compatibilidad y estado de drivers
- ✅ **Comandos de consola interactivos** para control avanzado
- ✅ **Sistema de logs mejorado** con categorización de mensajes
- ✅ **Respaldo automático** - Si un driver falla, puedes cambiar a otro al instante
- ✅ **Información en tiempo real** sobre el estado del sistema

### Características principales:

- ✅ **Interfaz web moderna** con diseño oscuro profesional
- ✅ **Backend robusto en C++** con motor de inyección BYOVD
- ✅ **Soporte para múltiples drivers vulnerables**
- ✅ **Integración completa con Cheat Engine**
- ✅ **Logs detallados** y monitoreo en tiempo real

## 🏗️ Estructura del Proyecto

```
BYOVD_Professional_Suite/
├── 📁 backend/                    # Código fuente del motor en C++
│   ├── main.cpp                   # Punto de entrada principal (v2.0)
│   ├── injection_engine.h         # Definiciones del motor
│   └── injection_engine.cpp       # Motor de inyección BYOVD (v2.0)
├── 📁 frontend/                   # Archivos de la interfaz web
│   └── dashboard.html             # Se genera automáticamente
├── 📁 drivers/                    # Drivers vulnerables (.sys)
│   ├── README.md                  # Guía completa de drivers
│   └── sample_drivers_info.md     # Base de datos técnica detallada
├── 📁 core_dlls/                  # DLLs del motor de Cheat Engine
│   └── README.md                  # Guía de DLLs
├── 📜 build.bat                   # Script de compilación (v2.0)
├── 📜 verify_drivers.bat          # Verificador de integridad (NUEVO)
└── 📜 README.md                   # Este archivo
```

## 🔧 Drivers Vulnerables Soportados

El sistema detecta y soporta automáticamente los siguientes drivers:

### 🥇 **Recomendados para Principiantes**
- **gdrv.sys** - Gigabyte Driver (Muy estable, ampliamente compatible)
- **procexp.sys** - Process Explorer de Microsoft (Respaldado oficialmente)

### 🎮 **Populares en Gaming**
- **RTCore64.sys** - MSI Afterburner (Común en sistemas gaming)
- **cpuz.sys** - CPU-Z (Frecuentemente instalado)

### 🔧 **Para Máxima Compatibilidad**
- **procexp.sys** - Compatible con Windows XP a 11
- **cpuz.sys** - Excelente soporte multiplataforma

### ⚡ **Para Acceso Avanzado**
- **WinRing0x64.sys** - Acceso completo de bajo nivel
- **AsIO.sys** - Driver específico de ASUS
- **amifldrv.sys** - AMI Flash Driver (BIOS/UEFI)
- **phymemx64.sys** - Manipulación directa de memoria

## 🚀 Instalación y Uso

### **Paso 1: Preparar el Entorno**

```bash
# Clonar o crear la estructura de carpetas
mkdir BYOVD_Professional_Suite
cd BYOVD_Professional_Suite

# Asegúrate de tener MinGW-w64 o MSYS2 instalado
# Descarga desde: https://www.mingw-w64.org/downloads/
```

### **Paso 2: Compilar el Backend**

```bash
# Ejecutar el script de compilación
build.bat

# El script creará BYOVD_Backend_v2.exe
```

### **Paso 3: Obtener Drivers Vulnerables Reales**

**⚠️ IMPORTANTE: Este paso requiere obtener drivers legítimos**

#### **Opción A: Drivers de Hardware Existente**
```bash
# Si tienes hardware Gigabyte/MSI/ASUS, instala su software oficial:
# - Gigabyte: App Center, RGB Fusion 2.0
# - MSI: Dragon Center, Afterburner  
# - ASUS: Armoury Crate, AI Suite III

# Luego copia los drivers desde:
# C:\Windows\System32\drivers\*.sys
```

#### **Opción B: Herramientas de Monitoreo**
```bash
# Instala herramientas legítimas de monitoreo:
# - CPU-Z desde https://www.cpuid.com/
# - HWiNFO64 desde https://www.hwinfo.com/
# - MSI Afterburner (para RTCore64.sys)

# Los drivers se instalan automáticamente al ejecutar estas herramientas
```

#### **Opción C: Localización Manual**
```bash
# Usa PowerShell para encontrar drivers existentes:
Get-ChildItem -Path "C:\Windows\System32\drivers\" -Filter "*.sys" | 
Where-Object {$_.Name -match "(gdrv|msio|AsIO|RTCore|cpuz|HWiNFO)"} | 
Select-Object Name, FullName, Length

# Copia los encontrados a la carpeta drivers/
```

### **Paso 4: Configurar DLLs de Cheat Engine**

```bash
# Descarga e instala Cheat Engine desde el sitio oficial
# Luego copia las DLLs desde:
# C:\Program Files\Cheat Engine X.X\
# 
# Archivos necesarios:
# - cheatengine-x86_64.dll (o cheatengine-i386.dll para 32-bit)
# - lua5.4.dll
# - speedhack-x86_64.dll
```

### **Paso 5: Verificar Integridad**

```bash
# Verifica que tienes al menos un driver .sys:
dir drivers\*.sys

# Verifica las firmas digitales:
signtool verify /pa drivers\gdrv.sys

# El sistema debe mostrar: "Successfully verified"
```

### **Paso 6: Ejecutar el Sistema**

```bash
# CRÍTICO: Ejecutar como Administrador
# Click derecho en BYOVD_Backend_v2.exe -> "Ejecutar como administrador"
BYOVD_Backend_v2.exe
```

## 💻 Uso del Sistema

### **Interfaz Web**
1. **Se abre automáticamente** al ejecutar el backend
2. **Selecciona un driver** de la lista disponible
3. **Carga el driver** con un clic
4. **Configura el proceso objetivo** y la DLL
5. **Ejecuta la inyección** cuando esté listo

### **Comandos de Consola**
```
BYOVD> list_drivers              # Lista todos los drivers disponibles
BYOVD> info gdrv                 # Información detallada del driver
BYOVD> load gdrv                 # Carga el driver especificado
BYOVD> unload                    # Descarga el driver actual
BYOVD> exit                      # Cierra el sistema
```

## 🔍 Características Avanzadas v2.0

### **Sistema de Detección Automática**
- Escanea la carpeta `drivers/` al iniciar
- Identifica automáticamente todos los archivos `.sys`
- Proporciona información detallada de cada driver
- Muestra compatibilidad y estado en tiempo real

### **Selección Dinámica**
- Cambia entre drivers sin reiniciar la aplicación
- Información detallada de cada driver antes de cargar
- Sistema de respaldo si un driver falla
- Detección de conflictos y problemas de compatibilidad

### **Interfaz Mejorada**
- Diseño completamente renovado con estilo moderno
- Animaciones y transiciones suaves
- Sistema de logs categorizado por color
- Estado del sistema en tiempo real
- Información contextual de cada driver

### **Robustez del Sistema**
- Manejo mejorado de errores y excepciones
- Limpieza automática al cerrar la aplicación
- Verificación de permisos y compatibilidad
- Logs detallados para debugging

## ⚠️ Consideraciones de Seguridad

### **Uso Responsable**
- **Solo para fines educativos** y de investigación
- **Úsalo únicamente en sistemas que poseas**
- **Respeta las leyes locales** y términos de uso
- **Siempre verifica** la legitimidad de los drivers

### **Verificación de Drivers**
- Descarga drivers solo de **fuentes oficiales**
- Usa **VirusTotal** para verificar archivos
- Verifica **firmas digitales** cuando sea posible
- Prueba primero en **entornos de laboratorio**

### **Precauciones del Sistema**
- **Ejecuta como administrador** para funcionalidad completa
- **Crea puntos de restauración** antes de usar
- **Cierra aplicaciones importantes** antes de las pruebas
- **Ten drivers de respaldo** listos

## 🔧 Solución de Problemas

### **Errores de Compilación**
```bash
# Error: g++ no encontrado
# Solución: Instalar MinGW-w64 o MSYS2

# Error: std::filesystem no disponible  
# Solución: Actualizar a g++ 8.0 o superior

# Error: Faltan librerías
# Solución: Instalar librerías de desarrollo de Windows
```

### **Errores de Ejecución**
```bash
# Error: No se puede cargar driver
# Solución: Ejecutar como administrador

# Error: Driver no encontrado
# Solución: Verificar que el archivo .sys esté en drivers/

# Error: Proceso no encontrado
# Solución: Verificar que el proceso esté en ejecución
```

## 📊 Comparación de Versiones

| Característica | v1.0 | v2.0 |
|---------------|------|------|
| Selección de drivers | Driver fijo | **Selección múltiple** |
| Detección automática | ❌ | ✅ |
| Información de drivers | ❌ | ✅ |
| Interfaz web | Básica | **Completamente renovada** |
| Comandos de consola | Básicos | **Sistema interactivo** |
| Sistema de logs | Simple | **Categorizado y mejorado** |
| Robustez | Básica | **Manejo avanzado de errores** |

## 🎯 Roadmap Futuro

- [ ] **Integración WebSocket real** para comunicación bidireccional
- [ ] **Sistema de plugins** para extensibilidad
- [ ] **Soporte para múltiples procesos** simultáneos
- [ ] **Interfaz de configuración avanzada**
- [ ] **Sistema de actualizaciones automáticas**
- [ ] **Soporte para drivers personalizados**

## 🤝 Contribuciones

Este proyecto es para fines educativos. Las contribuciones son bienvenidas siguiendo estas pautas:

- Mantén el enfoque educativo y de investigación
- Documenta todas las mejoras y cambios
- Respeta las consideraciones de seguridad
- Prueba exhaustivamente antes de enviar cambios

## 📜 Licencia

Este proyecto se proporciona únicamente para **fines educativos y de investigación**. El uso indebido del software es responsabilidad del usuario final.

---

**BYOVD Professional Suite v2.0** - La suite más avanzada para aprendizaje e investigación de técnicas BYOVD

*¿Preguntas o problemas? Consulta la documentación detallada en cada carpeta del proyecto.* 