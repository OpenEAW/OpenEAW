#include <cstdint>
#include <string>

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace khepri::utility {

/// @brief enum containing all the supported platforms
enum Platform : std::uint8_t
{
    windows,
    linux,
};

constexpr Platform get_current_platform()
{
#if defined(_WIN32) || defined(_WIN64)
    return Platform::windows;
#elif defined(__linux__)
    return Platform::linux;
#else
#error "Unsupported platform"
#endif
}

std::string get_registry_key(std::int64_t key, std::string_view subkey, std::string_view value);
} // namespace khepri::utility
