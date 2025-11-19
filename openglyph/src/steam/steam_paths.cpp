#include "vdf_fileparser.hpp"

#include <khepri/io/exceptions.hpp>
#include <khepri/utility/platform.hpp>

#include <openglyph/steam/steam_paths.hpp>

#include <iostream>
namespace openglyph::steam {
std::filesystem::path SteamPaths::get_steam_root_path()
{
    switch (khepri::utility::get_current_platform())
    case khepri::utility::windows: {
#ifdef _MSC_VER
        std::filesystem::path steampath =
            khepri::utility::get_registry_key((std::uint64_t)HKEY_LOCAL_MACHINE,
                                              "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath");
        if (steampath.empty()) {
            steampath = "C:\\Program Files (x86)\\steam";
        }

        if (!std::filesystem::exists(steampath)) {
            throw khepri::io::FileNotFoundError();
        }

        if (!std::filesystem::exists(steampath / "steam.exe")) {
            throw khepri::io::FileNotFoundError();
        }
        return steampath;
#else
        return "C:\\Program Files (x86)\\steam";
#endif
    }
    case khepri::utility::linux:
        return "~/.steam/steam";
}
return "";
}

std::filesystem::path SteamPaths::get_steam_library_folders_path()
{
    return get_steam_root_path() / "steamapps" / "libraryfolders.vdf";
}
std::vector<std::filesystem::path> SteamPaths::get_steam_library_folders()
{
    const std::filesystem::path libraryfolders_path = get_steam_library_folders_path();

    if (!std::filesystem::exists(libraryfolders_path)) {
        throw khepri::io::FileNotFoundError();
    }

    std::ifstream file(libraryfolders_path);
    if (!file.is_open()) {
        throw khepri::io::Error("Could not open file");
    }

    const auto root = tyti::vdf::read(file);
    if (root.name != "libraryfolders") {
        throw khepri::io::InvalidFormatError();
    }

    std::vector<std::filesystem::path> folders;

    for (const auto& folder : root.childs) {
        folders.push_back(folder.second->attribs["path"]);
    }
    return folders;
}

std::filesystem::path SteamPaths::get_steam_app_location(std::uint64_t appId)
{
    std::vector<std::filesystem::path> libraryFolders = get_steam_library_folders();

    auto pathfinder = [&](int appId) -> std::pair<std::filesystem::path, std::filesystem::path> {
        const std::string filename = "appmanifest_" + std::to_string(appId) + ".acf";

        for (const auto& folder : libraryFolders) {
            auto manifestPath = folder / "steamapps" / filename;
            if (std::filesystem::exists(manifestPath)) {
                return {folder, manifestPath}; // return both at once
            }
        }
        throw khepri::io::FileNotFoundError();
    };
    const auto    paths = pathfinder(appId);
    std::ifstream file(paths.second);

    try {
        auto root = tyti::vdf::read(file);

        if (root.name != "AppState") {
            throw khepri::io::InvalidFormatError();
        }

        return paths.first / "steamapps" / "common" / root.attribs["installdir"];
    } catch (const std::runtime_error e) {
        // we can't rethrow here because we would be stuck in a catch loop!
    }
    throw khepri::io::Error("Unable to read steam file");
}

} // namespace openglyph::steam