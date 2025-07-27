#pragma once

#include "material.hpp"
#include "material_desc.hpp"

#include <memory>

namespace khepri::renderer {

/**
 * \brief A render pipeline.
 *
 * A render pipeline is a collection of steps (render passes) that collectively render meshes to the
 * screen. Render pipelines are created by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_render_pipeline.
 */
class RenderPipeline
{
public:
    RenderPipeline()          = default;
    virtual ~RenderPipeline() = default;

protected:
    RenderPipeline(const RenderPipeline&)            = default;
    RenderPipeline(RenderPipeline&&)                 = default;
    RenderPipeline& operator=(const RenderPipeline&) = default;
    RenderPipeline& operator=(RenderPipeline&&)      = default;
};

} // namespace khepri::renderer
