#pragma once

#include <khepri/io/stream.hpp>

#include <openglyph/assets/string_database.hpp>

namespace openglyph::io {

/**
 * @brief Loads a string database.
 *
 * Reads a stream containing a binary string database and returns the loaded string database.
 */
openglyph::StringDatabase read_string_database(khepri::io::Stream& stream);

} // namespace openglyph::io
