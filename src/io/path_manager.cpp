#include "path_manager.hpp"

#include <khepri/io/exceptions.hpp>
#include <openglyph/utility/steam_paths.hpp>
namespace openeaw::io {

constexpr std::uint64_t STEAM_EAW_APPID = 32470;

/// @brief attempts to find and install path for EAW
/// @param type the Installation type to query
/// @return the install path on success
std::optional<std::filesystem::path> get_install_path_type(InstallationTypes type)
{
    switch (type) {
    case InstallationTypes::steam:
        return openglyph::utility::SteamPaths::get_steam_app_location(STEAM_EAW_APPID);
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> PathManager::get_install_path()
{
    static constexpr InstallationTypes types[] = {InstallationTypes::steam};

    for (auto type : types) {
        if (auto path = get_install_path_type(type)) {
            return path; // success
        }
    }

    return std::nullopt;
}

} // namespace openeaw::io
