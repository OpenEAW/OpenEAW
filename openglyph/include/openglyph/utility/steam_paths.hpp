#include <cstdint>
#include <filesystem>
#include <vector>
namespace openglyph::utility {
/**
 * @brief this class groups alls steam path functions
 */
class SteamPaths
{
public:
    /// @brief Returns the steam application folder for the given application id
    /// @param appId steam id of the application
    /// @return The path containing the path.
    /// @throw khepri::io::FileNotFoundError when the application is not found
    static std::filesystem::path get_steam_app_location(std::uint64_t appId);
};

} // namespace openglyph::utility