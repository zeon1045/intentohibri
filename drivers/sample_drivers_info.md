# 🔧 Base de Datos de Drivers Vulnerables Legítimos

## Arsenal Completo de Drivers Firmados para BYOVD

La técnica **Bring Your Own Vulnerable Driver (BYOVD)** utiliza drivers legítimos y firmados digitalmente por Microsoft, pero que contienen vulnerabilidades conocidas. Esta técnica permite ejecutar código con privilegios de kernel desde modo usuario, evadiendo protecciones de seguridad.

## 🏆 CATEGORÍA 1: DRIVERS PREMIUM (SIGILO MÁXIMO)

### **gdrv.sys** - Gigabyte Graphics Driver
- **Hash MD5**: Varía por versión (verificar siempre)
- **Firma Digital**: Gigabyte Technology Co., Ltd.
- **Vulnerabilidad**: Arbitrary Read/Write en memoria física
- **CVE**: CVE-2018-19320, CVE-2018-19321
- **Funciones Vulnerables**: 
  - `DeviceIoControl` con códigos específicos
  - `MmMapIoSpace` sin validación adecuada
- **Obtención**: 
  - Gigabyte App Center
  - RGB Fusion 2.0
  - EasyTune (herramienta de overclocking)
- **Ubicación Post-Instalación**: `C:\Windows\System32\drivers\gdrv.sys`
- **Tamaño Típico**: ~13-15 KB
- **Compatibilidad**: Windows 7/8/10/11 (x64)
- **Nivel de Riesgo**: ⭐⭐ (Bajo-Medio) - Difícil de bloquear sin falsos positivos

### **msio64.sys** - MSI IO Driver  
- **Firma Digital**: Micro-Star International Co., Ltd.
- **Vulnerabilidad**: Physical memory read/write access
- **Funciones Vulnerables**:
  - IOCTL para acceso directo a memoria
  - Manipulación de registros del procesador
- **Obtención**:
  - MSI Dragon Center
  - MSI Mystic Light
  - MSI Afterburner (versiones específicas)
- **Ubicación**: `C:\Windows\System32\drivers\msio64.sys`
- **Compatibilidad**: Windows 10/11 (x64)
- **Nivel de Riesgo**: ⭐⭐ (Bajo-Medio) - Excelente alternativa a gdrv.sys

### **AsIO.sys / AsIO2.sys / AsIO3.sys** - ASUS IO Driver
- **Firma Digital**: ASUSTeK Computer Inc.
- **Vulnerabilidad**: Unrestricted physical memory access
- **Versiones Disponibles**:
  - AsIO.sys (más antigua, más compatible)
  - AsIO2.sys (versión intermedia)
  - AsIO3.sys (más reciente, potencialmente más segura)
- **Obtención**:
  - ASUS AI Suite III
  - Armoury Crate
  - ASUS Aura Sync
- **Ubicación**: `C:\Windows\System32\drivers\AsIO*.sys`
- **Nivel de Riesgo**: ⭐⭐⭐ (Medio) - Diferentes versions, diferentes niveles de detección

## ⚙️ CATEGORÍA 2: DRIVERS ESTÁNDAR (ALTA COMPATIBILIDAD)

### **RTCore64.sys** - Unwinder MSI Afterburner
- **Firma Digital**: UNWINDER
- **Vulnerabilidad**: Unrestricted MSR/PCI access
- **CVE**: CVE-2019-16098
- **Funciones Comprometidas**:
  - Model Specific Register (MSR) access
  - PCI configuration space access
  - Physical memory mapping
- **Obtención**:
  - MSI Afterburner (oficial)
  - EVGA Precision X1
- **Ubicación**: 
  - `C:\Program Files (x86)\MSI Afterburner\RTCore64.sys`
  - A veces en System32\drivers\
- **Tamaño**: ~4 KB
- **Nivel de Riesgo**: ⭐⭐⭐ (Medio) - Popular, conocido por anti-cheats

### **cpuz_x64.sys** - CPU-Z Driver
- **Firma Digital**: CPUID
- **Vulnerabilidad**: Physical memory read/write
- **Versiones Vulnerables**: Múltiples versiones históricas
- **Obtención**:
  - CPU-Z desde https://www.cpuid.com/
- **Ubicaciones Posibles**:
  - `C:\Windows\System32\drivers\cpuz*.sys`
  - Carpeta temporal durante ejecución
- **Tamaño**: ~27 KB aproximadamente
- **Nivel de Riesgo**: ⭐⭐⭐ (Medio-Alto) - Ampliamente conocido

### **HWiNFO64A.SYS** - HWiNFO Hardware Monitor
- **Firma Digital**: Martin Malik - REALiX
- **Vulnerabilidad**: Low-level hardware access
- **Capacidades**:
  - Direct physical memory access
  - Hardware register manipulation
- **Obtención**:
  - HWiNFO64 desde https://www.hwinfo.com/
- **Ubicación**: `C:\Windows\System32\drivers\HWiNFO64A.SYS`
- **Nivel de Riesgo**: ⭐⭐⭐ (Medio) - Respetada herramienta de diagnóstico

### **WinRing0x64.sys** - OpenLibSys WinRing0
- **Firma Digital**: OpenLibSys.org
- **Vulnerabilidad**: Complete ring-0 access
- **Capacidades Extensas**:
  - MSR read/write
  - PCI access
  - Physical memory access
  - Port I/O access
- **Obtención**:
  - GPU-Z
  - Open Hardware Monitor
  - Muchas herramientas de overclocking
- **Ubicación**: Variable según aplicación
- **Nivel de Riesgo**: ⭐⭐⭐⭐ (Alto) - Muy potente, conocido

## ⚠️ CATEGORÍA 3: DRIVERS DE ALTO RIESGO (DESARROLLO ÚNICAMENTE)

### **dbk64.sys** - Cheat Engine Kernel Module
- **Firma Digital**: Cheat Engine
- **Vulnerabilidad**: Designed for memory manipulation
- **Estado**: **INMEDIATAMENTE DETECTADO**
- **Obtención**: Instalación de Cheat Engine
- **Ubicación**: `C:\Program Files\Cheat Engine X.X\dbk64.sys`
- **Uso Recomendado**: Solo para desarrollo offline
- **Nivel de Riesgo**: ⭐⭐⭐⭐⭐ (Máximo) - Banneo instantáneo

### **kprocesshacker.sys** - Process Hacker Kernel Driver
- **Firma Digital**: Process Hacker
- **Vulnerabilidad**: Advanced process manipulation
- **Capacidades**:
  - Process memory access
  - Handle manipulation
  - Kernel object access
- **Obtención**: Process Hacker 2/3
- **Nivel de Riesgo**: ⭐⭐⭐⭐⭐ (Máximo) - Detección inmediata

## 🔬 ANÁLISIS TÉCNICO DE VULNERABILIDADES

### **Vectores de Ataque Comunes:**

1. **IOCTL Handler Vulnerabilities**
   ```cpp
   // Ejemplo típico de vulnerabilidad
   case IOCTL_READ_PHYSICAL_MEMORY:
       // Sin validación de permisos
       MmMapIoSpace(PhysicalAddress, Size, MmNonCached);
   ```

2. **Arbitrary Read/Write Primitives**
   - Lectura/escritura arbitraria en memoria física
   - Acceso sin restricciones a espacio kernel
   - Bypass de SMEP/SMAP en sistemas modernos

3. **MSR Access Vulnerabilities**
   ```cpp
   // Acceso directo a registros del modelo específico
   __readmsr(register_number);  // Sin validación
   __writemsr(register_number, value);
   ```

### **Métodos de Explotación:**

1. **Physical Memory Manipulation**
   - Localización de estructuras kernel críticas
   - Modificación de tokens de proceso
   - Bypass de mitigaciones de seguridad

2. **EPROCESS Token Manipulation**
   ```cpp
   // Escalación típica de privilegios
   // 1. Encontrar EPROCESS actual
   // 2. Localizar EPROCESS de SYSTEM
   // 3. Copiar token de SYSTEM al proceso actual
   ```

## 📊 MATRIZ DE DETECCIÓN POR ANTI-CHEAT

| Driver | BattlEye | EAC | Vanguard | VAC | Manual Detection |
|--------|----------|-----|----------|-----|------------------|
| gdrv.sys | ❓ | ❓ | ❓ | ❌ | Baja |
| msio64.sys | ❓ | ❓ | ❓ | ❌ | Baja |
| AsIO.sys | ❓ | ❓ | ⚠️ | ❌ | Media |
| RTCore64.sys | ⚠️ | ⚠️ | ⚠️ | ❌ | Media |
| cpuz_x64.sys | ⚠️ | ⚠️ | ⚠️ | ❓ | Alta |
| WinRing0x64.sys | ⚠️ | ⚠️ | ⚠️ | ⚠️ | Alta |
| dbk64.sys | ✅ | ✅ | ✅ | ✅ | Máxima |

**Leyenda:**
- ❌ = No detectado
- ❓ = Detección variable  
- ⚠️ = Posible detección
- ✅ = Detectado siempre

## 🛠️ GUÍA DE OBTENCIÓN TÉCNICA

### **Script PowerShell para Localización Automática:**

```powershell
# Buscar drivers vulnerables conocidos
$drivers = @("gdrv.sys", "msio64.sys", "AsIO*.sys", "RTCore64.sys", "cpuz*.sys")
$locations = @(
    "C:\Windows\System32\drivers\",
    "C:\Program Files\",
    "C:\Program Files (x86)\"
)

foreach($driver in $drivers) {
    foreach($location in $locations) {
        Get-ChildItem -Path $location -Filter $driver -Recurse -ErrorAction SilentlyContinue |
        Select-Object FullName, Length, CreationTime
    }
}
```

### **Verificación de Firma Digital:**

```cmd
# Verificar autenticidad del driver
signtool verify /pa /v driver.sys

# Mostrar información detallada del certificado
certutil -dump driver.sys
```

### **Análisis de Superficie de Ataque:**

```python
# Ejemplo de análisis básico de IOCTL codes
import struct

def parse_ioctl_codes(driver_path):
    with open(driver_path, 'rb') as f:
        data = f.read()
        # Buscar patrones típicos de IOCTL handlers
        # CTL_CODE macro patterns, etc.
        pass
```

## 🎯 ESTRATEGIA DE ROTACIÓN

### **Perfil de Rotación Recomendado:**

1. **Semana 1-2**: gdrv.sys (máximo sigilo)
2. **Semana 3-4**: msio64.sys (alternativa premium)  
3. **Semana 5-6**: AsIO.sys (cambio de fabricante)
4. **Semana 7-8**: Volver a evaluar detecciones

### **Indicadores de Compromiso:**

- Bans súbitos sin explicación
- Detecciones en foros especializados
- Actualizaciones de anti-cheat coincidentes
- Reportes de la comunidad

## ⚡ CONSIDERACIONES AVANZADAS

### **Evasión de Detección:**

1. **Renombrado de Archivos**
   ```bash
   # Renombrar para evitar detección por nombre
   copy gdrv.sys my_custom_driver.sys
   ```

2. **Modificación de Metadatos**
   - Cambiar timestamps
   - Modificar version info (cuidado con firmas)

3. **Carga Diferida**
   - No cargar inmediatamente al inicio
   - Usar técnicas de timing aleatorio

### **Limpieza Post-Uso:**

```cpp
// Siempre limpiar trazas
SC_HANDLE hService = OpenService(...);
ControlService(hService, SERVICE_CONTROL_STOP, &status);
DeleteService(hService);
// Eliminar archivos temporales
// Limpiar logs de eventos
```

## 🔒 OPSEC (OPERATIONAL SECURITY)

### **Mejores Prácticas:**

1. **Nunca** usar en cuentas principales
2. **Siempre** probar en entornos aislados
3. **Rotar** drivers regularmente
4. **Monitorear** foros de la comunidad
5. **Mantener** drivers actualizados pero vulnerables

### **Red Flags a Evitar:**

- Usar drivers detectados públicamente
- Patrones de uso predecibles
- Combinar múltiples drivers simultáneamente
- Usar en streamers/contenido público

---

**Esta base de datos se actualiza constantemente. La información de detección puede cambiar rápidamente.** 