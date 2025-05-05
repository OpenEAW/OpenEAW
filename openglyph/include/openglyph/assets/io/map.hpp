#pragma once

#include <khepri/io/stream.hpp>
#include <openglyph/assets/map.hpp>

namespace openglyph::io {

/**
 * @brief Loads a map.
 *
 * Reads a stream containing a binary map and returns the loaded map information.
 */
openglyph::Map read_map(khepri::io::Stream& stream);

} // namespace openglyph::io
