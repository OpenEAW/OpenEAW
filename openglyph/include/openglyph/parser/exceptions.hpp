#pragma once

#include <openglyph/exceptions.hpp>

namespace openglyph {

class ParseError : public Error
{
public:
    using Error::Error;
};

} // namespace openglyph
