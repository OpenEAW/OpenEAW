#pragma once

#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/renderer.hpp>

#include <openglyph/game/scene.hpp>

namespace openglyph {

class SceneRenderer
{
public:
    SceneRenderer(khepri::renderer::Renderer&             renderer,
                  const khepri::renderer::RenderPipeline& render_pipeline)
        : m_renderer(renderer), m_render_pipeline(render_pipeline)
    {
    }
    ~SceneRenderer() = default;

    SceneRenderer(const SceneRenderer&)                = delete;
    SceneRenderer(SceneRenderer&&) noexcept            = delete;
    SceneRenderer& operator=(const SceneRenderer&)     = delete;
    SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

    void render_scene(const openglyph::Scene& scene, const khepri::renderer::Camera& camera);

private:
    void render_scene(const khepri::scene::Scene& scene, const openglyph::Environment& environment,
                      const khepri::renderer::Camera& camera);

    khepri::renderer::Renderer&             m_renderer;
    const khepri::renderer::RenderPipeline& m_render_pipeline;
};

} // namespace openglyph
