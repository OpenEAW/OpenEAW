#include "graphics_pipeline_options.hpp"

#include <khepri/log/log.hpp>

#include <openglyph/parser/parsers.hpp>
#include <openglyph/parser/xml_parser.hpp>
#include <openglyph/renderer/io/render_pipeline.hpp>

namespace openglyph {
template <>
struct Parser<khepri::renderer::RenderPassDesc::DepthSorting>
{
    using DepthSorting = khepri::renderer::RenderPassDesc::DepthSorting;

    static std::optional<DepthSorting> parse(std::string_view str) noexcept
    {
        if (khepri::case_insensitive_equals(str, "none")) {
            return DepthSorting::none;
        }
        if (khepri::case_insensitive_equals(str, "front_to_back")) {
            return DepthSorting::front_to_back;
        }
        if (khepri::case_insensitive_equals(str, "back_to_front")) {
            return DepthSorting::back_to_front;
        }
        return {};
    }
};

namespace renderer::io {
namespace {
auto load_render_pass(const openglyph::XmlParser::Node& node)
{
    khepri::renderer::RenderPassDesc render_pass;
    render_pass.material_type = optional_child(node, "Material_Type", "");
    render_pass.depth_sorting = parse<khepri::renderer::RenderPassDesc::DepthSorting>(
        optional_child(node, "Depth_Sort", "None"));
    render_pass.default_graphics_pipeline_options = parse_graphics_pipeline_options(node);
    return render_pass;
}

auto load_render_pipeline(const openglyph::XmlParser::Node& node)
{
    khepri::renderer::RenderPipelineDesc render_pipeline_desc;
    render_pipeline_desc.name = require_attribute(node, "Name");
    for (const auto& child : node.nodes()) {
        render_pipeline_desc.render_passes.push_back(load_render_pass(child));
    }
    return render_pipeline_desc;
}

constexpr khepri::log::Logger LOG("renderer");
} // namespace

std::vector<khepri::renderer::RenderPipelineDesc>
load_render_pipelines(khepri::io::Stream& xml_stream)
{
    std::vector<khepri::renderer::RenderPipelineDesc> render_pipelines;
    try {
        const openglyph::XmlParser xml(xml_stream);
        if (const auto& root = xml.root()) {
            for (const auto& node : root->nodes()) {
                render_pipelines.push_back(load_render_pipeline(node));
            }
        }
    } catch (const openglyph::ParseError& e) {
        LOG.error("parse error: {}", e.what());
    }
    return render_pipelines;
}

} // namespace renderer::io
} // namespace openglyph
