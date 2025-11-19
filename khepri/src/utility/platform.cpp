#include <khepri/exceptions.hpp>
#include <khepri/utility/platform.hpp>
namespace khepri::utility {
std::string get_registry_key(std::int64_t key, std::string_view subkey, std::string_view value)
{
#ifdef _MSC_VER
    DWORD type = REG_SZ;
    TCHAR result[255];
    DWORD bufsize = 255;
    LONG  status  = RegOpenKey((HKEY)key, subkey.data(), (PHKEY)&key);

    if (status == ERROR_FILE_NOT_FOUND) {
        return "";
    }

    RegQueryValueEx((HKEY)key, value.data(), nullptr, &type, (LPBYTE)&result, &bufsize);
    RegCloseKey((HKEY)key);
    return std::string(result);
#else
    throw khepri::Error("Registry access is only supported on Windows.");
    return "";
#endif
}

} // namespace khepri::utility
