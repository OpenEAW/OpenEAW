#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/shader_desc.hpp>

namespace khepri::renderer::io {

/**
 * Loads a shader description from a stream.
 *
 * Since shader descriptions are just a wrapper around a file's content to be interpreted by the
 * renderer, this function is just a convenience function for reading the stream. It exists mainly
 * for clarity and symmetry with other asset-reading functions.
 *
 * @throw khepri::argument_error the stream is not readable and seekable.
 */
ShaderDesc load_shader(khepri::io::Stream& stream);

} // namespace khepri::renderer::io
