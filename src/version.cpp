#include <khepri/version_info.hpp>

namespace openeaw {

namespace {
const ::khepri::VersionInfo build_version_info{OPENEAW_VERSION_MAJOR, OPENEAW_VERSION_MINOR,
                                               OPENEAW_VERSION_PATCH, OPENEAW_VERSION_STRING,
                                               OPENEAW_VERSION_CLEAN, OPENEAW_VERSION_COMMIT};
}

const ::khepri::VersionInfo& version() noexcept
{
    return build_version_info;
}

} // namespace openeaw
