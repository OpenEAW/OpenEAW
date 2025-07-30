#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace khepri::renderer {

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

    struct DepthBufferDesc
    {
        /// Depth test comparison function, if any.
        /// Set to \a std::nullopt to disable depth testing.
        ComparisonFunc comparison_func{ComparisonFunc::less};

        /// Enable depth-buffer writing
        bool write_enable{true};
    };

    /// The type of materials to render in this pass. Must match
    /// #khepri::renderer::MaterialDesc::type.
    std::string material_type;

    /// Face culling mode of this render pass.
    CullMode cull_mode{CullMode::none};

    /// How to depth-sort the objects assigned to this render pass.
    DepthSorting depth_sorting{DepthSorting::none};

    /// Type of alpha blending to use for this render pass.
    AlphaBlendMode alpha_blend_mode{AlphaBlendMode::none};

    /// Depth-buffer settings to use for this render pass.
    std::optional<DepthBufferDesc> depth_buffer;
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
