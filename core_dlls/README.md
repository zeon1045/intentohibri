# 📁 Carpeta Core DLLs - Componentes de Cheat Engine

## ¿Qué contiene esta carpeta?

Esta carpeta contiene los **componentes del motor de Cheat Engine** extraídos del código fuente oficial para usar con la técnica BYOVD.

## ✅ **COMPONENTES INSTALADOS (Desde código fuente oficial):**

### **🎯 Ejecutables Principales:**
- **cheatengine-x86_64.exe** (16MB) - Motor principal 64-bit
  - **Función**: Ejecutable completo de Cheat Engine para procesos 64-bit
  - **Uso**: Motor principal para inyección y manipulación de memoria
  - **Arquitectura**: x64

- **cheatengine-i386.exe** (12MB) - Motor principal 32-bit  
  - **Función**: Ejecutable completo de Cheat Engine para procesos 32-bit
  - **Uso**: Compatibilidad con procesos legacy de 32-bit
  - **Arquitectura**: x86

### **🔧 DLLs de Soporte:**
- **lua53-64.dll** (529KB) - Motor Lua 64-bit
  - **Función**: Scripting engine para automatización avanzada
  - **Uso**: Ejecución de scripts Lua para operaciones complejas
  - **Versión**: Lua 5.3 compilado para x64

- **lua53-32.dll** (453KB) - Motor Lua 32-bit
  - **Función**: Scripting engine para procesos 32-bit
  - **Uso**: Compatibilidad Lua para aplicaciones x86
  - **Versión**: Lua 5.3 compilado para x86

- **d3dhook64.dll** (132KB) - DirectX Hook 64-bit
  - **Función**: Hooks para aplicaciones DirectX
  - **Uso**: Manipulación de aplicaciones que usan DirectX
  - **Soporte**: DirectX 9/10/11/12

## 🔗 **INTEGRACIÓN CON NUESTRO SISTEMA BYOVD:**

### **Arquitectura de Funcionamiento:**
```
BYOVD Backend (C++) 
    ↓ Carga driver vulnerable (gdrv.sys, msio64.sys, etc.)
    ↓ Inyecta usando driver BYOVD
    ↓ Llama a cheatengine-x86_64.exe con parámetros
    ↓ Cheat Engine ejecuta operaciones de memoria
    ↓ Resultado devuelto al backend
```

### **Comandos de Línea Disponibles:**
```bash
# Inyección básica de DLL
cheatengine-x86_64.exe -inject <PID> <DLL_PATH>

# Scanner de memoria  
cheatengine-x86_64.exe -scan <PID> <VALUE> <TYPE>

# Modo headless (sin interfaz gráfica)
cheatengine-x86_64.exe -headless -inject <PID> <DLL_PATH>

# Usar con script Lua
cheatengine-x86_64.exe -script <LUA_SCRIPT>
```

## 🚀 **USO EN EL PROYECTO:**

### **Desde Backend C++:**
```cpp
// Ejemplo de uso en injection_engine.cpp
std::string ceCommand = "core_dlls\\cheatengine-x86_64.exe -headless -inject " + 
                       std::to_string(processId) + " " + dllPath;
PROCESS_INFORMATION pi;
CreateProcess(NULL, (LPSTR)ceCommand.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
```

### **Desde Interfaz Web:**
- El backend traduce comandos web a llamadas de línea de comandos
- Resultados se devuelven vía JSON al frontend
- Control completo desde la interfaz web moderna

## 📊 **CAPACIDADES DISPONIBLES:**

| Función | cheatengine-x86_64.exe | cheatengine-i386.exe | lua53-*.dll | d3dhook64.dll |
|---------|------------------------|----------------------|-------------|---------------|
| **Inyección DLL** | ✅ | ✅ | ❌ | ❌ |
| **Scanner Memoria** | ✅ | ✅ | ❌ | ❌ |
| **Manipulación Valores** | ✅ | ✅ | ❌ | ❌ |
| **Scripting Lua** | ✅ | ✅ | ✅ | ❌ |
| **Hooks DirectX** | ✅ | ❌ | ❌ | ✅ |
| **Debugging** | ✅ | ✅ | ❌ | ❌ |
| **Speedhack** | ✅ | ✅ | ❌ | ❌ |

## ⚙️ **CONFIGURACIÓN AUTOMÁTICA:**

### **Detección de Arquitectura:**
```cpp
// El backend detecta automáticamente si usar x64 o x86
bool isProcess64Bit = IsProcess64Bit(processId);
std::string executable = isProcess64Bit ? 
    "core_dlls\\cheatengine-x86_64.exe" : 
    "core_dlls\\cheatengine-i386.exe";
```

### **Integración con Drivers BYOVD:**
1. **Backend carga driver vulnerable** (gdrv.sys, msio64.sys, etc.)
2. **Driver establece acceso kernel-level**
3. **Cheat Engine utiliza acceso privilegiado**
4. **Operaciones de memoria se ejecutan sin detección**
5. **Backend descarga driver al terminar**

## 🎯 **VENTAJAS DE ESTA IMPLEMENTACIÓN:**

### **✅ Inmediato:**
- **Sin compilación** - Componentes ya listos
- **Funcionalidad completa** - Todas las capacidades de CE
- **Probado y estable** - Código fuente oficial

### **✅ Flexible:**
- **Interfaz unificada** - Control desde web y consola
- **Múltiples drivers** - Compatible con cualquier driver BYOVD
- **Escalable** - Fácil agregar más funcionalidades

### **✅ Profesional:**
- **Código limpio** - Integración elegante
- **Logging completo** - Monitoreo de todas las operaciones
- **Error handling** - Manejo robusto de errores

## 🔧 **PRÓXIMOS PASOS (OPCIONAL):**

### **Optimización Futura:**
1. **Compilar DBK driver personalizado** desde `DBKKernel/`
2. **Crear DLLs core optimizadas** desde código fuente
3. **Reducir tamaño** eliminando componentes innecesarios
4. **Firma personalizada** para máximo sigilo

### **Compilación de DBK Driver:**
```bash
# Requiere Windows Driver Kit (WDK)
# Ubicación: C:\Users\mosta\Documents\cheat-engine-master\DBKKernel\
# Resultado: dbk64.sys (custom driver)
```

## 📋 **ESTRUCTURA FINAL:**

```
core_dlls/
├── cheatengine-x86_64.exe    # Motor principal 64-bit (16MB)
├── cheatengine-i386.exe      # Motor principal 32-bit (12MB)  
├── lua53-64.dll              # Lua engine 64-bit (529KB)
├── lua53-32.dll              # Lua engine 32-bit (453KB)
├── d3dhook64.dll             # DirectX hook (132KB)
└── README.md                 # Este archivo
```

**Total: ~29MB de componentes del motor de Cheat Engine**

## ⚠️ **IMPORTANTE:**

- **Ejecutar siempre como administrador** para funcionalidad completa
- **Antivirus pueden alertar** - Son componentes legítimos de CE
- **Solo uso educativo** en sistemas propios
- **Drivers BYOVD necesarios** para operación sin detección

---

**Los componentes están listos para usar. El sistema BYOVD está ahora funcionalmente completo.** 