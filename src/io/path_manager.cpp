#include "path_manager.hpp"

#include <khepri/io/exceptions.hpp>
#include <openglyph/steam/steam_paths.hpp>
namespace openeaw::io {

constexpr std::uint64_t STEAM_EAW_APPID = 32470;

std::filesystem::path PathManager::get_install_path_auto()
{
    for (std::uint8_t i = 0; i < InstallationTypes::maxinstalltypes; ++i) {
        try {
            return get_install_path((InstallationTypes)i);
        } catch (const khepri::io::FileNotFoundError&) {
            // ignore and try next type
        }
    }

    // none succeeded
    throw khepri::io::FileNotFoundError();
}

std::filesystem::path PathManager::get_install_path(InstallationTypes type)
{
    switch (type) {
    case InstallationTypes::steam:
        return openglyph::steam::SteamPaths::get_steam_app_location(STEAM_EAW_APPID);
    }
    throw khepri::io::NotSupportedError();
}
} // namespace openeaw::io
