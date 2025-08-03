#include <khepri/renderer/render_pipeline_desc.hpp>

#include <openglyph/parser/xml_parser.hpp>

namespace openglyph::renderer::io {

khepri::renderer::GraphicsPipelineOptions
parse_graphics_pipeline_options(const openglyph::XmlParser::Node& node);

} // namespace openglyph::renderer::io
