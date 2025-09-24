#include <cstdint>
#include <filesystem>
#include <vector>
namespace openglyph::steam {
/**
 * @brief this class groups alls steam path functions
 */
class SteamPaths
{
private:
public:
    /// @brief Returns the usual Steam root installation path for the current platform.
    /// @return The root Steam installation directory as a std::filesystem::path.
    static std::filesystem::path get_steam_root_path();

    /// @brief Returns the path to the Steam 'steamapps' directory.
    /// @return The 'steamapps' directory path as a std::filesystem::path.
    static std::filesystem::path get_steam_apps_path();

    /// @brief Returns the path to the Steam 'libraryfolders.vdf' file, which contains additional
    /// Steam library locations.
    /// @return The full path to the 'libraryfolders.vdf' file as a std::filesystem::path.
    static std::filesystem::path get_steam_library_folders_path();

    static std::vector<std::filesystem::path> get_steam_library_folders();

    static std::filesystem::path get_steam_app_location(std::uint64_t appId);
};

} // namespace openglyph::steam