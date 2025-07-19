#pragma once

#include <openglyph/game/environment.hpp>

namespace openglyph {

struct Map
{
    struct Header
    {
        // Map format version
        std::uint32_t version{0};
    };

    struct Object
    {
        std::uint32_t id{0};

        /// CRC of the uppercased name of the object type
        std::uint32_t type_crc{0};

        /// Position of the object on the map
        khepri::Vector3f position{0, 0, 0};

        /// Direction the object is facing
        khepri::Quaternionf facing{khepri::Quaternionf::IDENTITY};
    };

    Header header;

    std::vector<Environment> environments;
    std::uint32_t            active_environment{0};
    std::vector<Object>      objects;
};

} // namespace openglyph
