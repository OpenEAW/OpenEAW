#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/render_pipeline_desc.hpp>

#include <vector>

namespace openglyph::renderer::io {

std::vector<khepri::renderer::RenderPipelineDesc>
load_render_pipelines(khepri::io::Stream& xml_stream);

} // namespace openglyph::renderer::io
