#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace khepri::renderer {

/**
 * Options to configure the graphics pipeline with.
 *
 * All properties are optional because this type is re-used by #RenderPipelineDesc and
 * #MaterialDesc, where the latter's options can override the former's.
 *
 * The renderer has built-in defaults if both the render pipeline and material don't specify the
 * option. The defaults are specified in the comments for each property.
 */
struct GraphicsPipelineOptions
{
    /// The type of face culling
    enum class CullMode : std::uint8_t
    {
        none,
        back,
        front,
    };

    /// Comparison function for depth or stencil buffer operations
    enum class ComparisonFunc : std::uint8_t
    {
        never,
        less,
        equal,
        less_equal,
        greater,
        not_equal,
        greater_equal,
        always,
    };

    /// The type of alpha blending
    enum class AlphaBlendMode : std::uint8_t
    {
        /// Do not alpha blend.
        none,

        /// Source and destination are blended according to source alpha.
        blend_src,

        /// Source is added on top of destination.
        additive,
    };

    /// Face culling mode of this render pass (default=back).
    std::optional<CullMode> cull_mode;

    /// Do front-faces have counter-clockwise winding order? If not, they have clock-wise winding
    /// order (default=false)
    std::optional<bool> front_ccw;

    /// Type of alpha blending to use for this render pass (default=none).
    std::optional<AlphaBlendMode> alpha_blend_mode;

    /// Whether depth buffer interactions are enabled (default=true).
    std::optional<bool> depth_enable;

    /// Depth-buffer test comparison function (default=less). Ignored if \a depth_enable is false.
    std::optional<ComparisonFunc> depth_comparison_func;

    /// Whether to enable depth-buffer writing (default=true). Ignored if \a depth_enable is false.
    std::optional<bool> depth_write_enable;
};

/**
 * Description of a render pass in a pipeline.
 *
 * A render pass renders meshes in a certain way.
 */
struct RenderPassDesc
{
    /// How to sort objects by depth during this render pass
    enum class DepthSorting : std::uint8_t
    {
        /// Objects are rendered in arbitrary order.
        none,

        /// Objects are rendered front-to-back, i.e. those closer to the camera are rendered first.
        front_to_back,

        /// Objects are rendered back-to-front, i.e. those closer to the camera are rendered last.
        back_to_front,
    };

    /// The type of materials to render in this pass. Must match
    /// #khepri::renderer::MaterialDesc::type.
    std::string material_type;

    /// How to depth-sort the objects assigned to this render pass.
    DepthSorting depth_sorting{DepthSorting::none};

    /// Default values for the graphics pipeline options. Materials can override these.
    GraphicsPipelineOptions default_graphics_pipeline_options;
};

/**
 * \brief Description of a render pipeline
 *
 * A render pipeline describes how to render the meshes assigned for rendering. It defines
 * properties like post-processing and other complex effects. A pipeline is split into render passes
 * that render meshes in a certain way. Each pass has its own properties. A pass can render only
 * a selection of meshes based on the mesh's material.
 */
struct RenderPipelineDesc
{
    /// Name of the pipeline; used for debugging purposes
    std::string name;

    /// Collection of render passes; these are rendered in order.
    std::vector<RenderPassDesc> render_passes;
};

} // namespace khepri::renderer
