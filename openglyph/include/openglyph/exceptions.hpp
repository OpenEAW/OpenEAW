#pragma once

#include <stdexcept>

namespace openglyph {

class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace openglyph
