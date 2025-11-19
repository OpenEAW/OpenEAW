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
    /// @brief Returns the usual Steam root installation path for the current platform.
    /// @return The root Steam installation directory as a std::filesystem::path.
    static std::filesystem::path get_steam_root_path();

    /// @brief Returns the path to the Steam 'libraryfolders.vdf' file, which contains additional
    /// Steam library locations.
    /// @return The full path to the 'libraryfolders.vdf' file as a std::filesystem::path.
    static std::filesystem::path get_steam_library_folders_path();

    /// @brief Returns the folder locations for all the steam library folders
    /// @return The vector containing all the library folder paths
    static std::vector<std::filesystem::path> get_steam_library_folders();

public:
    /// @brief Returns the steam application folder for the given application id
    /// @param appId steam id of the application
    /// @return The path containing the path.
    /// @throw khepri::io::FileNotFoundError when the application is not found
    static std::filesystem::path get_steam_app_location(std::uint64_t appId);
};

} // namespace openglyph::steam