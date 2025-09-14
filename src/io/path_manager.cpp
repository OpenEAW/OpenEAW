#include "path_manager.hpp"

#include <khepri/io/exceptions.hpp>
#include <openglyph/steam/steam_paths.hpp>
namespace openeaw::io {

std::filesystem::path PathManager::get_install_path(InstallationTypes type)
{
    switch (type) {
    case InstallationTypes::steam:
        return openglyph::steam::SteamPaths::get_steam_app_location(32470);
    }
    throw khepri::io::NotSupportedError();
}
} // namespace openeaw::io
