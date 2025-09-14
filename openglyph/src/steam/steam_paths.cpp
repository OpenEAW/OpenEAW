#include <khepri/application/platform.hpp>
#include <khepri/io/exceptions.hpp>

#include <openglyph/steam/steam_paths.hpp>
#include <openglyph/steam/vdf_fileparser.hpp>

#include <iostream>
namespace openglyph::steam {
std::filesystem::path SteamPaths::get_steam_root_path()
{
    switch (khepri::application::get_current_platform()) {
    case khepri::application::windows:
        return "C:\\Program Files (x86)\\steam";
    case khepri::application::linux:
        return "~/.steam/steam";
    }
    return "";
}

std::filesystem::path SteamPaths::get_steam_apps_path()
{
    return get_steam_root_path() / "steamapps";
}
std::filesystem::path SteamPaths::get_steam_library_folders_path()
{
    return get_steam_apps_path() / "libraryfolders.vdf";
}
std::vector<std::filesystem::path> SteamPaths::get_steam_library_folders()
{
    std::vector<std::filesystem::path> folders;

    std::filesystem::path libraryfolders_path = get_steam_library_folders_path();

    if (!std::filesystem::exists(libraryfolders_path)) {
        throw khepri::io::FileNotFoundError();
    }

    std::ifstream file(libraryfolders_path);
    if (!file.is_open()) {
        throw khepri::io::Error("Could not open file");
    }

    auto ROOT = tyti::vdf::read(file);
    if (ROOT.name != "libraryfolders") {
        throw khepri::io::InvalidFormatError();
    }

    for (auto folder : ROOT.childs) {
        folders.emplace_back(folder.second->attribs["path"]);
    }
    return folders;
}
} // namespace openglyph::steam

std::filesystem::path openglyph::steam::SteamPaths::get_steam_app_location(std::uint64_t appId)
{
    std::vector<std::filesystem::path> libraryFolders = get_steam_library_folders();

    std::filesystem::path manifestPath;
    std::filesystem::path folderPath;
    for (std::size_t i = 0; i < libraryFolders.size(); ++i) {
        std::string filename = "appmanifest_" + std::to_string(appId) + ".acf";
        manifestPath         = libraryFolders[i] / "steamapps" / filename;

        if (std::filesystem::exists(manifestPath)) {
            folderPath = libraryFolders[i];
            break;
        }

        if(i == libraryFolders.size() - 1){
            throw khepri::io::FileNotFoundError();
        }
    }

    std::ifstream file(manifestPath);

    auto ROOT = tyti::vdf::read(file);

    if (ROOT.name != "AppState") {
        throw khepri::io::InvalidFormatError();
    }
    return folderPath / "steamapps"/ "common" / ROOT.attribs["installdir"];
}
