#include "vdf_fileparser.hpp"

#include <khepri/io/exceptions.hpp>
#include <khepri/utility/platform.hpp>

#include <openglyph/utility/steam_paths.hpp>

#include <iostream>
namespace openglyph::utility {

/// @brief Returns the usual Steam root installation path for the current platform.
/// @return The root Steam installation directory as a std::filesystem::path.
std::filesystem::path get_steam_root_path()
{
    switch (khepri::utility::get_current_platform()) {
    case khepri::utility::Platform::windows:
#if defined(_WIN32) || defined(_WIN64)
    {
        std::filesystem::path steampath =
            khepri::utility::get_registry_key(HKEY_LOCAL_MACHINE,
                                              "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath")
                .value_or("C:\\Program Files (x86)\\steam");
        if (!std::filesystem::exists(steampath)) {
            throw khepri::io::FileNotFoundError();
        }

        if (!std::filesystem::exists(steampath / "steam.exe")) {
            throw khepri::io::FileNotFoundError();
        }
        return steampath;
    }
#else
        return "C:\\Program Files (x86)\\steam";
#endif
    case khepri::utility::Platform::linux: {
        std::filesystem::path steampath = "~/.steam";

        if (!std::filesystem::exists(steampath)) {
            throw khepri::io::FileNotFoundError();
        }

        if (!std::filesystem::exists(steampath / "steam")) {
            throw khepri::io::FileNotFoundError();
        }
        return steampath;
    }
    }
    throw khepri::Error("platform not supported");
}

/// @brief Returns the path to the Steam 'libraryfolders.vdf' file, which contains additional
/// Steam library locations.
/// @return The full path to the 'libraryfolders.vdf' file as a std::filesystem::path.
std::filesystem::path get_steam_library_folders_path()
{
    return get_steam_root_path() / "steamapps" / "libraryfolders.vdf";
}

/// @brief Returns the folder locations for all the steam library folders
/// @return The vector containing all the library folder paths
std::vector<std::filesystem::path> get_steam_library_folders()
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

} // namespace openglyph::utility