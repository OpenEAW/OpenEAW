#include <khepri/version_info.hpp>

namespace openeaw {

namespace {
const ::khepri::VersionInfo BUILD_VERSION_INFO{OPENEAW_VERSION_MAJOR, OPENEAW_VERSION_MINOR,
                                               OPENEAW_VERSION_PATCH, OPENEAW_VERSION_STRING,
                                               OPENEAW_VERSION_CLEAN, OPENEAW_VERSION_COMMIT};
}

const ::khepri::VersionInfo& version() noexcept
{
    return BUILD_VERSION_INFO;
}

} // namespace openeaw
