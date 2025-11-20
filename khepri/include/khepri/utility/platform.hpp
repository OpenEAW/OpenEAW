#include <cstdint>
#include <string>
#include <optional>
#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace khepri::utility {

/// @brief enum containing all the supported platforms
enum class Platform : std::uint8_t
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

#if defined(_WIN32) || defined(_WIN64)
std::optional<std::string> get_registry_key(HKEY key, std::string_view subkey, std::string_view value);
#endif
} // namespace khepri::utility
