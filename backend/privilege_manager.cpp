#include "privilege_manager.h"
#include <iostream>
#include <iomanip>

BOOL SetPrivilege(LPCWSTR lpszPrivilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES tp;
    HANDLE hToken = NULL;
    LUID luid;

    std::wcout << L"[PRIVILEGE] Intentando " << (bEnablePrivilege ? L"habilitar" : L"deshabilitar") 
               << L" privilegio: " << lpszPrivilege << std::endl;

    // 1. Abrir el token de acceso del proceso actual. Requerimos permisos para ajustar y consultar.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        DWORD error = GetLastError();
        std::cerr << "[PRIVILEGE] Error: OpenProcessToken falló. Codigo: " << error << std::endl;
        if (error == ERROR_ACCESS_DENIED) {
            std::cerr << "[PRIVILEGE] ERROR_ACCESS_DENIED - El proceso no tiene suficientes privilegios" << std::endl;
        }
        return FALSE;
    }

    // 2. Obtener el Identificador Único Local (LUID) para el privilegio que queremos.
    if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid)) {
        DWORD error = GetLastError();
        std::cerr << "[PRIVILEGE] Error: LookupPrivilegeValueW falló. Codigo: " << error << std::endl;
        if (error == ERROR_NO_SUCH_PRIVILEGE) {
            std::wcerr << L"[PRIVILEGE] ERROR_NO_SUCH_PRIVILEGE - El privilegio " << lpszPrivilege 
                       << L" no existe en este sistema" << std::endl;
        }
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege) {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    } else {
        tp.Privileges[0].Attributes = 0;
    }

    // 3. Aplicar el cambio de privilegios al token.
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
        DWORD error = GetLastError();
        std::cerr << "[PRIVILEGE] Error: AdjustTokenPrivileges falló. Codigo: " << error << std::endl;
        CloseHandle(hToken);
        return FALSE;
    }

    // 4. ¡Paso crucial! Verificar si el privilegio realmente fue asignado.
    // Si el usuario no es Administrador, AdjustTokenPrivileges puede "tener éxito" sin hacer nada,
    // pero GetLastError() devolverá ERROR_NOT_ALL_ASSIGNED.
    DWORD lastError = GetLastError();
    if (lastError == ERROR_NOT_ALL_ASSIGNED) {
        std::wcerr << L"[PRIVILEGE] ERROR_NOT_ALL_ASSIGNED - El privilegio " << lpszPrivilege 
                   << L" no pudo ser asignado." << std::endl;
        std::cerr << "[PRIVILEGE] CAUSA MAS PROBABLE: El programa no se ejecuta como Administrador" << std::endl;
        std::cerr << "[PRIVILEGE] SOLUCION: Ejecutar como 'Administrador' haciendo clic derecho en el .exe" << std::endl;
        CloseHandle(hToken);
        return FALSE;
    } else if (lastError != ERROR_SUCCESS) {
        std::cerr << "[PRIVILEGE] Error inesperado: " << lastError << std::endl;
        CloseHandle(hToken);
        return FALSE;
    }

    std::wcout << L"[PRIVILEGE] ✅ Privilegio '" << lpszPrivilege << L"' configurado correctamente." << std::endl;
    CloseHandle(hToken);
    return TRUE;
}

BOOL IsPrivilegeEnabled(LPCWSTR lpszPrivilege) {
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES* pTokenPrivileges = NULL;
    DWORD dwLength = 0;
    LUID luid;
    BOOL bResult = FALSE;

    // Abrir token del proceso
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    // Obtener LUID del privilegio
    if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    // Obtener información de privilegios
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLength);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(hToken);
        return FALSE;
    }

    pTokenPrivileges = (TOKEN_PRIVILEGES*)malloc(dwLength);
    if (!pTokenPrivileges) {
        CloseHandle(hToken);
        return FALSE;
    }

    if (GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, dwLength, &dwLength)) {
        // Buscar el privilegio específico
        for (DWORD i = 0; i < pTokenPrivileges->PrivilegeCount; i++) {
            if (pTokenPrivileges->Privileges[i].Luid.LowPart == luid.LowPart &&
                pTokenPrivileges->Privileges[i].Luid.HighPart == luid.HighPart) {
                bResult = (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) != 0;
                break;
            }
        }
    }

    free(pTokenPrivileges);
    CloseHandle(hToken);
    return bResult;
}

BOOL InitializeBelzebubPrivileges() {
    std::cout << "\n[PRIVILEGE] ================================================" << std::endl;
    std::cout << "[PRIVILEGE] 🔧 Inicializando privilegios de Belzebub..." << std::endl;
    std::cout << "[PRIVILEGE] ================================================" << std::endl;

    BOOL allPrivilegesOk = TRUE;
    
    // Lista de privilegios necesarios
    struct {
        LPCWSTR name;
        LPCWSTR description;
        BOOL required;
    } privileges[] = {
        {L"SeLoadDriverPrivilege", L"Cargar drivers del kernel", TRUE},
        {L"SeDebugPrivilege", L"Depurar procesos", TRUE},
        {L"SeIncreaseQuotaPrivilege", L"Aumentar cuotas de memoria", FALSE},
        {L"SeSecurityPrivilege", L"Gestión de seguridad", FALSE}
    };

    for (int i = 0; i < sizeof(privileges) / sizeof(privileges[0]); i++) {
        std::wcout << L"[PRIVILEGE] Configurando: " << privileges[i].description << L"..." << std::endl;
        
        BOOL result = SetPrivilege(privileges[i].name, TRUE);
        
        if (result) {
            std::wcout << L"[PRIVILEGE] ✅ " << privileges[i].description << L" - HABILITADO" << std::endl;
        } else {
            std::wcout << L"[PRIVILEGE] ❌ " << privileges[i].description << L" - FALLO" << std::endl;
            if (privileges[i].required) {
                allPrivilegesOk = FALSE;
                std::wcout << L"[PRIVILEGE] ⚠️  PRIVILEGIO REQUERIDO FALLÓ: " << privileges[i].name << std::endl;
            }
        }
    }

    std::cout << "\n[PRIVILEGE] ================================================" << std::endl;
    if (allPrivilegesOk) {
        std::cout << "[PRIVILEGE] ✅ TODOS LOS PRIVILEGIOS REQUERIDOS HABILITADOS" << std::endl;
        std::cout << "[PRIVILEGE] 🔥 Belzebub listo para operaciones avanzadas" << std::endl;
    } else {
        std::cout << "[PRIVILEGE] ❌ ALGUNOS PRIVILEGIOS REQUERIDOS FALLARON" << std::endl;
        std::cout << "[PRIVILEGE] ⚠️  Funcionalidad limitada disponible" << std::endl;
        std::cout << "[PRIVILEGE] 💡 SOLUCION: Ejecutar como Administrador" << std::endl;
    }
    std::cout << "[PRIVILEGE] ================================================\n" << std::endl;

    return allPrivilegesOk;
}

BOOL IsUserAdmin() {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    // Método 1: Verificar mediante CheckTokenMembership
    if (AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &administratorsGroup)) {
        
        if (CheckTokenMembership(NULL, administratorsGroup, &isAdmin)) {
            FreeSid(administratorsGroup);
            if (isAdmin) {
                return TRUE;
            }
        } else {
            FreeSid(administratorsGroup);
        }
    }

    // Método 2: Verificar elevación del token (más robusto)
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
            if (isAdmin) {
                CloseHandle(hToken);
                return TRUE;
            }
        }
        CloseHandle(hToken);
    }

    // Método 3: Verificar acceso al SCM (el que funciona en la consola)
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm != NULL) {
        CloseServiceHandle(scm);
        return TRUE;  // Si podemos abrir SCM, somos admin
    }

    return FALSE;
}

BOOL SetLoadDriverPrivilege(BOOL enable) {
    return SetPrivilege(L"SeLoadDriverPrivilege", enable);
} 