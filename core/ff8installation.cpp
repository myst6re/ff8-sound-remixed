#include "ff8installation.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <winbase.h>
// #include <winerror.h>
#include <winreg.h>
#endif

FF8Installation::FF8Installation() {}


QString FF8Installation::regValue(const QString &regPath, const QString &regKey,
                       RegTarget target, RegLocation loc)
{
#ifdef Q_OS_WIN
    HKEY phkResult;
    LONG error;
    REGSAM flags = KEY_READ;
    HKEY hkey = HKEY_LOCAL_MACHINE;
    
    if (target == Target32) {
#ifdef KEY_WOW64_32KEY
        flags |= KEY_WOW64_32KEY; // if you compile in 64-bit, force reg search into 32-bit entries
#endif
    } else if (target == Target64) {
#ifdef KEY_WOW64_64KEY
        flags |= KEY_WOW64_64KEY;
#endif
    }
    
    // Open regPath relative to HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER
    if (loc == LocationUser) {
        hkey = HKEY_CURRENT_USER;
    }
    error = RegOpenKeyExW(hkey, QDir::toNativeSeparators("SOFTWARE/" % regPath).toStdWString().c_str(), 0, flags, &phkResult);
    if (ERROR_SUCCESS == error) {
        BYTE value[MAX_PATH];
        DWORD cValue = MAX_PATH, type;
        
        // Open regKey which must is a string value (REG_SZ)
        RegQueryValueExW(phkResult, regKey.toStdWString().c_str(), nullptr, &type, value, &cValue);
        if (ERROR_SUCCESS == error && type == REG_SZ) {
            RegCloseKey(phkResult);
            return QString::fromUtf16((char16_t *)value);
        }
        RegCloseKey(phkResult);
    }
#else
    Q_UNUSED(regPath)
    Q_UNUSED(regKey)
    Q_UNUSED(target)
    Q_UNUSED(loc)
#endif
    
    return QString();
}

QString FF8Installation::regValue(const QString &regPath, const QString &regKey,
                       RegLocation loc)
{
    QString ret;

#if defined(KEY_WOW64_32KEY) || defined(KEY_WOW64_64KEY)
    ret = regValue(regPath, regKey, Target32, loc);
    if (ret.isEmpty()) {
        ret = regValue(regPath, regKey, Target64, loc);
    }
#else
    ret = regValue(regPath, regKey, TargetNone, loc);
#endif
    
    return ret;
}
