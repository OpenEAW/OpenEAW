#pragma once

#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/renderer.hpp>

#include <openglyph/game/scene.hpp>

namespace openglyph {

class SceneRenderer
{
public:
    explicit SceneRenderer(khepri::renderer::Renderer& renderer) : m_renderer(renderer) {}
    ~SceneRenderer() = default;

    SceneRenderer(const SceneRenderer&)                = delete;
    SceneRenderer(SceneRenderer&&) noexcept            = delete;
    SceneRenderer& operator=(const SceneRenderer&)     = delete;
    SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

    void render_scene(const openglyph::Scene& scene, const khepri::renderer::Camera& camera);

private:
    void render_scene(const khepri::scene::Scene& scene, const openglyph::Environment& environment,
                      const khepri::renderer::Camera& camera);

    khepri::renderer::Renderer& m_renderer;
};

} // namespace openglyph
