#include <cstdint>

namespace khepri::application {

/// @brief enum containing all the supported platforms
enum Platform : std::uint8_t
{
    windows,
    linux,
};

Platform get_current_platform();
} // namespace khepri::application