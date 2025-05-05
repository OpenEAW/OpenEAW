#pragma once

#include <khepri/io/stream.hpp>
#include <openglyph/renderer/model.hpp>

namespace openglyph::io {

/**
 * @brief Loads an ALO model
 *
 * Reads a stream containing a binary Alamo Object (ALO) model and returns a renderer-agnostic
 * description of the model.
 */
openglyph::renderer::Model read_model(khepri::io::Stream& stream);

} // namespace openglyph::io
