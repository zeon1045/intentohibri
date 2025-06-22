# 🚀 **BYOVD Professional Suite v3.0**

## **🎯 Sistema Completo de Bring Your Own Vulnerable Driver**

**BYOVD Professional Suite v3.0** es la versión más avanzada y funcional del proyecto, ofreciendo un control programático completo de Cheat Engine con interfaz web profesional y funcionalidades de nivel comercial.

---

## **✨ NUEVAS CARACTERÍSTICAS v3.0**

### **🔥 SERVIDOR WEB REAL**
- **API REST completa** con endpoints para todas las funcionalidades
- **Interfaz web moderna** que se comunica directamente con el backend
- **Dashboard profesional** con control en tiempo real
- **Puerto configurable** (por defecto: `http://localhost:12345`)

### **🎮 CONTROL PROGRAMÁTICO DE CHEAT ENGINE**
- **Carga y parseo** de archivos `.CT` (Cheat Tables)
- **Activación/desactivación** de entradas programáticamente
- **Modificación de valores** desde la interfaz web
- **Speedhack integrado** con control deslizante
- **Integración completa** con el motor oficial de Cheat Engine

### **🛡️ SISTEMA BYOVD PROFESIONAL**
- **Base de datos** de 6 drivers vulnerables documentados
- **Nombres de servicio aleatorios** para evadir detección
- **Gestión automática** de carga/descarga de drivers
- **Información detallada** de CVEs y fuentes oficiales

---

## **📁 ESTRUCTURA DEL PROYECTO**

```
belbel/
├── 📂 backend/                    # Motor C++ con servidor web
│   ├── injection_engine.h        # Interfaz principal del motor
│   ├── injection_engine.cpp      # Implementación completa
│   └── main.cpp                   # Servidor web con API REST
├── 📂 core_dlls/                  # Componentes de Cheat Engine (29MB)
│   ├── cheatengine-x86_64.exe    # CE 64-bit (16MB)
│   ├── cheatengine-i386.exe      # CE 32-bit (12MB)
│   ├── lua53-64.dll              # Motor Lua 64-bit
│   ├── lua53-32.dll              # Motor Lua 32-bit
│   └── d3dhook64.dll             # Hooks DirectX
├── 📂 drivers/                    # Drivers vulnerables (.sys)
├── 📂 frontend/                   # Interfaz web profesional
│   └── dashboard.html             # Dashboard completo y funcional
├── 📂 libs/                       # Librerías C++
│   ├── httplib.h                 # Servidor web (347KB)
│   └── json.hpp                  # Manejo JSON (937KB)
├── 🔧 build.bat                   # Script de compilación
├── 🔧 obtenerdrivers.ps1         # Asistente de drivers
└── 📚 Documentación completa
```

---

## **🚀 INSTALACIÓN Y USO**

### **PASO 1: Preparación**
```powershell
# 1. Descargar dependencias (ya incluidas)
# Las librerías httplib.h y json.hpp ya están descargadas

# 2. Obtener drivers vulnerables (LEGAL)
.\obtenerdrivers.ps1

# 3. Compilar el proyecto
.\build.bat
```

### **PASO 2: Ejecución**
```powershell
# Navegar a la carpeta de compilación
cd build

# Ejecutar como ADMINISTRADOR (REQUERIDO)
.\BYOVD_Professional_v3.exe
```

### **PASO 3: Uso de la Interfaz**
1. **Abre automáticamente**: `http://localhost:12345`
2. **Carga un driver**: Selecciona de la lista y haz clic en "Cargar"
3. **Busca un proceso**: Escribe el nombre (ej: `notepad.exe`)
4. **Inyecta DLLs**: Especifica la ruta de tu DLL
5. **Carga Cheat Tables**: Arrastra archivos `.CT` y controla entradas
6. **Usa Speedhack**: Desliza para cambiar la velocidad del juego

---

## **🔧 API REST ENDPOINTS**

La suite expone una API completa para integración programática:

### **Sistema y Drivers**
- `GET /api/status` - Estado del sistema
- `GET /api/drivers` - Lista de drivers disponibles
- `POST /api/load_driver` - Carga driver por ID
- `POST /api/unload_driver` - Descarga driver actual

### **Procesos e Inyección**
- `POST /api/find_process` - Busca procesos por nombre
- `POST /api/inject_dll` - Inyecta DLL en proceso

### **Cheat Tables (.CT)**
- `POST /api/load_ct` - Carga archivo .CT
- `POST /api/get_ct_entries` - Obtiene entradas de la tabla
- `POST /api/control_cheat` - Activa/desactiva entrada
- `POST /api/set_cheat_value` - Modifica valor de entrada

### **Herramientas**
- `POST /api/set_speedhack` - Establece velocidad de speedhack

---

## **🎮 COMPATIBILIDAD CON CHEAT TABLES**

### **Funcionalidades Soportadas**
- ✅ **Carga de archivos .CT** - Parser XML integrado
- ✅ **Control de entradas** - Activación/desactivación programática
- ✅ **Modificación de valores** - Cambio de valores desde interfaz
- ✅ **Ejecución de scripts Lua** - Motor Lua completo integrado
- ✅ **Speedhack** - Control de velocidad de juego
- ✅ **Hooks DirectX** - Para aplicaciones gráficas

### **Limitaciones Actuales**
- ⚠️ **Congelamiento de memoria** - Pendiente implementación
- ⚠️ **Escaneo de punteros** - Requiere funcionalidad adicional
- ⚠️ **Auto-attach** - Implementación manual requerida

---

## **🛡️ DRIVERS SOPORTADOS**

| Driver | Archivo | Tier | CVE | Fuente Oficial |
|--------|---------|------|-----|----------------|
| **Gigabyte GDrv** | `gdrv.sys` | Premium | CVE-2018-19320 | Gigabyte App Center |
| **MSI MSIo** | `msio64.sys` | Premium | CVE-2019-16098 | MSI Dragon Center |
| **MSI RTCore** | `RTCore64.sys` | Standard | CVE-2019-16098 | MSI Afterburner |
| **CPU-Z** | `cpuz159_x64.sys` | Standard | CVE-2017-15302 | CPU-Z Official |
| **HWiNFO64** | `HWiNFO_x64_205.sys` | Standard | CVE-2018-8960 | HWiNFO Official |
| **Process Hacker** | `kprocesshacker.sys` | High-Risk | CVE-2020-13833 | Process Hacker |

---

## **🔥 CARACTERÍSTICAS AVANZADAS**

### **Sigilo y Evasión**
- **Nombres de servicio aleatorios** - Evita patrones de detección
- **Paths obfuscados** - Scripts Lua con rutas ofuscadas
- **Ventanas ocultas** - Ejecución silenciosa de CE
- **Integración nativa** - Usa componentes oficiales

### **Robustez**
- **Manejo de errores JSON** - Respuestas estructuradas
- **Detección automática** de arquitectura (32/64-bit)
- **Timeout configurables** - Para operaciones de larga duración
- **Limpieza automática** - Scripts temporales auto-eliminados

### **Profesionalismo**
- **Logging completo** - Registro detallado de actividades
- **Interfaz responsiva** - Funciona en diferentes resoluciones
- **Estado en tiempo real** - Actualizaciones automáticas
- **Validación de entrada** - Prevención de errores comunes

---

## **🚨 IMPORTANTE - USO LEGAL**

### **⚠️ SOLO PARA FINES EDUCATIVOS**
Este software está diseñado exclusivamente para:
- **Investigación de seguridad**
- **Pentesting autorizado**
- **Educación en ciberseguridad**
- **Análisis de vulnerabilidades**

### **📋 REQUISITOS LEGALES**
- ✅ **Solo usar en sistemas propios**
- ✅ **Obtener drivers de fuentes oficiales**
- ✅ **Seguir todas las leyes locales**
- ❌ **NO usar para actividades maliciosas**
- ❌ **NO distribuir drivers vulnerables**

---

## **🔧 SOLUCIÓN DE PROBLEMAS**

### **Error de Compilación**
```powershell
# Verificar que g++ está instalado
g++ --version

# Ejecutar setup del compilador
.\setup_compiler.ps1
```

### **Error "No hay drivers"**
```powershell
# Ejecutar asistente de drivers
.\obtenerdrivers.ps1

# Verificar permisos de administrador
# Ejecutar como administrador
```

### **Error de Conexión Web**
- Verificar que el puerto 12345 no esté en uso
- Comprobar firewall de Windows
- Ejecutar como administrador

---

## **📈 ROADMAP FUTURO**

### **v3.1 - Mejoras de Funcionalidad**
- [ ] Congelamiento de memoria
- [ ] Escaneo avanzado de punteros
- [ ] Auto-attach a procesos
- [ ] Templates de cheat tables

### **v3.2 - Características Avanzadas**
- [ ] WebSocket para comunicación en tiempo real
- [ ] Plugin system para extensiones
- [ ] Múltiples sesiones simultáneas
- [ ] Integración con debuggers

### **v4.0 - Profesional Enterprise**
- [ ] Base de datos de procesos conocidos
- [ ] Sistema de perfiles y configuraciones
- [ ] Análisis automático de vulnerabilidades
- [ ] Reportes de seguridad integrados

---

## **👥 CONTRIBUCIONES**

Las contribuciones son bienvenidas siguiendo las siguientes pautas:
1. **Fork** del repositorio
2. **Crear branch** para tu feature
3. **Commit** con mensajes descriptivos
4. **Pull request** con descripción detallada

---

## **📄 LICENCIA**

Este proyecto está bajo licencia **MIT** para fines educativos únicamente.

**DESCARGO DE RESPONSABILIDAD**: Los autores no se hacen responsables del mal uso de este software. El usuario es completamente responsable de cumplir con todas las leyes aplicables.

---

## **🌟 RECONOCIMIENTOS**

- **Cheat Engine** - Por el motor base y componentes
- **cpp-httplib** - Por el servidor web embebido
- **nlohmann-json** - Por el manejo de JSON
- **Comunidad de seguridad** - Por la investigación de vulnerabilidades

---

<div align="center">

**🚀 BYOVD Professional Suite v3.0**  
*El sistema BYOVD más avanzado y funcional disponible*

</div> 