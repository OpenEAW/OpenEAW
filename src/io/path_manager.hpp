#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace openeaw::io {

/// @brief enum containing all the possible installation types
enum class InstallationTypes : std::uint8_t
{
    steam,
    maxinstalltypes
};

/**
 * @brief this class handles path management and finding of installed versions and mods.
 */
class PathManager
{
public:
    /// @brief attempts to find and install path for EAW
    /// @return returns the first install path that is valid.
    static std::optional<std::filesystem::path> get_install_path();
};
} // namespace openeaw::io
