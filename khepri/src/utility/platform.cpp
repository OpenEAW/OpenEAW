#include <khepri/exceptions.hpp>
#include <khepri/utility/platform.hpp>
namespace khepri::utility {
#if defined(_WIN32) || defined(_WIN64)
std::optional<std::string> get_registry_key(HKEY key, std::string_view subkey,
                                            std::string_view value)
{
    DWORD type = REG_SZ;
    TCHAR result[255];
    DWORD bufsize = 255;
    LONG  status  = RegOpenKey(key, subkey.data(), &key);

    if (status == ERROR_FILE_NOT_FOUND) {
        return std::nullopt;
    }

    RegQueryValueEx(key, value.data(), nullptr, &type, (LPBYTE)&result, &bufsize);
    RegCloseKey(key);
    return std::string(result);
}
#endif
} // namespace khepri::utility
