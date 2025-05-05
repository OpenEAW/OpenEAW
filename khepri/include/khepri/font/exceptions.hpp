#pragma once

#include <khepri/exceptions.hpp>

namespace khepri::font {

/**
 * \brief An error happened during a font operation
 */
class FontError : public Error
{
public:
    using Error::Error;
};

} // namespace khepri::font
