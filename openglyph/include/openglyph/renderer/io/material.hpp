#pragma once

#include <khepri/io/stream.hpp>

#include <openglyph/renderer/material_desc.hpp>

#include <vector>

namespace openglyph::renderer::io {

std::vector<MaterialDesc> load_materials(khepri::io::Stream& xml_stream);

} // namespace openglyph::renderer::io
