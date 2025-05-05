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

    Header header;

    std::vector<Environment> environments;
    std::uint32_t            active_environment;
};

} // namespace openglyph
