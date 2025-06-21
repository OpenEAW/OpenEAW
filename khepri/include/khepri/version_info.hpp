#pragma once

#include <string>
#include <string_view>

namespace khepri {

/**
 * \brief Defines version information for a software component.
 *
 * A components's version is specified as major.minor.patch.
 *
 * Since multiple commits could be related to a version, it also includes the exact VCS commit ID
 * that the component was built from, with optional 'dirty' suffix if it was built from a repository
 * with local changes.
 */
struct VersionInfo final
{
    /// The major version
    int major_version;

    /// The minor version
    int minor_version;

    /// The patch version
    int patch_version;

    /// The stringified major.minor.patch version
    std::string_view version_string;

    /// Indicates if the version is built from a clean workspace or not
    bool is_version_clean;

    /// The commit of the VCS that the version is based on
    std::string_view build_commit;
};

/**
 * \brief Converts version information into a human-readable string
 *
 * The string is in the following format: "major.minor.patch (commit_id[-dirty])""
 */
std::string to_string(const VersionInfo& info);

} // namespace khepri
