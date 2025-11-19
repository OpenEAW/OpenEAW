#pragma once

#include <cstdint>
#include <filesystem>

namespace openeaw::io {

/// @brief enum containing all the possible installation types
enum InstallationTypes : std::uint8_t
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
    static std::filesystem::path get_install_path_auto();

    /// @brief attempts to find and install path for EAW
    /// @param type the Installation type to query
    /// @return the install path on success
    /// @throw khepri::io::FileNotFoundError when the application is not found
    static std::filesystem::path get_install_path(InstallationTypes type);
};
} // namespace openeaw::io
