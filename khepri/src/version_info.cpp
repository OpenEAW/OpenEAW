#include <khepri/version_info.hpp>

namespace khepri {

std::string to_string(const VersionInfo& info)
{
    std::string commit_id = std::string(info.build_commit);
    if (!info.is_version_clean) {
        commit_id += "-dirty";
    }

    return std::string(info.version_string) + " (commit " + commit_id + ")";
}

} // namespace khepri
