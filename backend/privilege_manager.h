#ifndef PRIVILEGE_MANAGER_H
#define PRIVILEGE_MANAGER_H

#include <windows.h>

/**
 * @brief Habilita o deshabilita un privilegio para el proceso actual.
 * @param lpszPrivilege El nombre del privilegio a modificar (ej. SE_LOAD_DRIVER_NAME).
 * @param bEnablePrivilege TRUE para habilitar el privilegio, FALSE para deshabilitarlo.
 * @return TRUE si la operación fue exitosa, FALSE en caso contrario.
 */
BOOL SetPrivilege(LPCWSTR lpszPrivilege, BOOL bEnablePrivilege);

/**
 * @brief Verifica si un privilegio específico está habilitado para el proceso actual.
 * @param lpszPrivilege El nombre del privilegio a verificar.
 * @return TRUE si el privilegio está habilitado, FALSE en caso contrario.
 */
BOOL IsPrivilegeEnabled(LPCWSTR lpszPrivilege);

/**
 * @brief Inicializa todos los privilegios necesarios para Belzebub.
 * @return TRUE si todos los privilegios fueron habilitados exitosamente.
 */
BOOL InitializeBelzebubPrivileges();

/**
 * @brief Comprueba si el proceso actual se está ejecutando con privilegios de administrador.
 * @return TRUE si el usuario es administrador, FALSE en caso contrario.
 */
BOOL IsUserAdmin();

/**
 * @brief Habilita o deshabilita el privilegio para cargar/descargar drivers.
 * @param enable TRUE para habilitar, FALSE para deshabilitar.
 * @return TRUE si la operación fue exitosa, FALSE en caso contrario.
 */
BOOL SetLoadDriverPrivilege(BOOL enable);

#endif // PRIVILEGE_MANAGER_H 