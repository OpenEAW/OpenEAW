#pragma once

#include <cstdint>
#include <filesystem>

namespace openeaw::io {

enum InstallationTypes : std::uint8_t
{
    steam = 1,
};

/**
 * @brief this class handles path management and finding of installed versions and mods.
 */
class PathManager
{
private:
    /* data */

public:
    static std::filesystem::path get_install_path(InstallationTypes type);
};
} // namespace openeaw::io
