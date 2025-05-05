#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/model_desc.hpp>

#include <memory>
#include <string>

namespace khepri::renderer::io {

/**
 * Loads a model description from a stream with Khepri Model File data.
 * @throw khepri::argument_error the stream is not readable and seekable.
 */
ModelDesc load_kmf(khepri::io::Stream& stream);

/**
 * Writes a model description to a stream as Khepri Model File data.
 * @throw khepri::argument_error the stream is not writable and seekable.
 */
void write_kmf(const ModelDesc& model, khepri::io::Stream& stream);

} // namespace khepri::renderer::io
