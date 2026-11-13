#pragma once

#include <khepri/io/stream.hpp>

#include <openglyph/assets/mega_texture.hpp>

namespace openglyph::io {

/**
 * @brief Loads a mega texture.
 *
 * Reads a stream containing a binary mega texture and returns the loaded mega texture.
 */
openglyph::MegaTexture read_mega_texture(khepri::io::Stream& stream);

} // namespace openglyph::io
