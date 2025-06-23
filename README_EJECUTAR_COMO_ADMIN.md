# 🔥 Belzebub Professional Suite - Guía de Ejecución con Privilegios

## ✅ Estado Actual
- **Servidor HTTP**: ✅ Funcionando correctamente
- **Detección de Admin**: ✅ Detecta privilegios de administrador
- **Carga de Drivers**: ⚠️ Requiere privilegios adicionales

## 🔧 Para Funcionalidad Completa

### Opción 1: Ejecutar como Administrador (Recomendado)
1. **Clic derecho** en `BelzebubAdmin.bat`
2. Seleccionar **"Ejecutar como administrador"**
3. Confirmar en el diálogo UAC
4. El navegador se abrirá automáticamente en `http://localhost:12345`

### Opción 2: Desde PowerShell/CMD como Admin
```bash
# Abrir PowerShell como administrador y ejecutar:
cd "C:\Users\mosta\Desktop\intentogibrido"
.\Belzebub.exe
```

### Opción 3: Desde Explorador de Windows
1. **Clic derecho** en `Belzebub.exe`
2. Seleccionar **"Ejecutar como administrador"**
3. Confirmar en el diálogo UAC

## 🎯 Verificación de Estado

Una vez ejecutando, accede a `http://localhost:12345` y verifica en el panel de estado:

- **Running as Admin**: ✅ True
- **Debug Privilege**: ✅ True  
- **Load Driver Privilege**: ✅ True

## ⚠️ Notas Importantes

- **Windows 10/11**: Requiere confirmar UAC
- **Antivirus**: Puede detectar como "herramienta de hacking" (normal)
- **Firewall**: Puede pedir permiso para el servidor HTTP (permitir)

## 🔥 Funcionalidades Disponibles con Privilegios Completos

- ✅ **Carga de drivers del kernel**
- ✅ **Inyección de DLLs en procesos protegidos**
- ✅ **Acceso completo a memoria de procesos**
- ✅ **Manipulación de Cheat Tables (.ct)**
- ✅ **Control de velocidad de juegos (speedhack)**
- ✅ **Scripts Lua avanzados** 