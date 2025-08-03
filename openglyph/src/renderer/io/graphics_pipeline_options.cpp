#include "graphics_pipeline_options.hpp"

#include <openglyph/parser/parsers.hpp>

namespace openglyph {
template <>
struct Parser<khepri::renderer::GraphicsPipelineOptions::CullMode>
{
    using CullMode = khepri::renderer::GraphicsPipelineOptions::CullMode;

    static std::optional<CullMode> parse(std::string_view str) noexcept
    {
        if (khepri::case_insensitive_equals(str, "none")) {
            return CullMode::none;
        }
        if (khepri::case_insensitive_equals(str, "back")) {
            return CullMode::back;
        }
        if (khepri::case_insensitive_equals(str, "front")) {
            return CullMode::front;
        }
        return {};
    }
};

template <>
struct Parser<khepri::renderer::GraphicsPipelineOptions::AlphaBlendMode>
{
    using AlphaBlendMode = khepri::renderer::GraphicsPipelineOptions::AlphaBlendMode;

    static std::optional<AlphaBlendMode> parse(std::string_view str) noexcept
    {
        if (khepri::case_insensitive_equals(str, "none")) {
            return AlphaBlendMode::none;
        }
        if (khepri::case_insensitive_equals(str, "blend_src")) {
            return AlphaBlendMode::blend_src;
        }
        if (khepri::case_insensitive_equals(str, "additive")) {
            return AlphaBlendMode::additive;
        }
        return {};
    }
};

template <>
struct Parser<khepri::renderer::GraphicsPipelineOptions::ComparisonFunc>
{
    using ComparisonFunc = khepri::renderer::GraphicsPipelineOptions::ComparisonFunc;

    static std::optional<ComparisonFunc> parse(std::string_view str) noexcept
    {
        if (khepri::case_insensitive_equals(str, "never")) {
            return ComparisonFunc::never;
        }
        if (khepri::case_insensitive_equals(str, "less")) {
            return ComparisonFunc::less;
        }
        if (khepri::case_insensitive_equals(str, "equal")) {
            return ComparisonFunc::equal;
        }
        if (khepri::case_insensitive_equals(str, "less_equal")) {
            return ComparisonFunc::less_equal;
        }
        if (khepri::case_insensitive_equals(str, "greater")) {
            return ComparisonFunc::greater;
        }
        if (khepri::case_insensitive_equals(str, "not_equal")) {
            return ComparisonFunc::not_equal;
        }
        if (khepri::case_insensitive_equals(str, "greater_equal")) {
            return ComparisonFunc::greater_equal;
        }
        if (khepri::case_insensitive_equals(str, "always")) {
            return ComparisonFunc::always;
        }
        return {};
    }
};

namespace renderer::io {
khepri::renderer::GraphicsPipelineOptions
parse_graphics_pipeline_options(const openglyph::XmlParser::Node& node)
{
    khepri::renderer::GraphicsPipelineOptions options;
    options.cull_mode = parse<khepri::renderer::GraphicsPipelineOptions::CullMode>(
        optional_child(node, "Cull_Mode"));
    options.front_ccw        = parse<bool>(optional_child(node, "Front_CCW"));
    options.alpha_blend_mode = parse<khepri::renderer::GraphicsPipelineOptions::AlphaBlendMode>(
        optional_child(node, "Alpha_Blend"));
    options.depth_enable = parse<bool>(optional_child(node, "Depth_Enable"));
    options.depth_comparison_func =
        parse<khepri::renderer::GraphicsPipelineOptions::ComparisonFunc>(
            optional_child(node, "Depth_Func"));
    options.depth_write_enable = parse<bool>(optional_child(node, "Depth_Write_Enable"));
    return options;
}
} // namespace renderer::io
} // namespace openglyph
