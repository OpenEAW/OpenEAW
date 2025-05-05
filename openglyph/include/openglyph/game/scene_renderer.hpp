#pragma once

#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/renderer.hpp>
#include <openglyph/game/scene.hpp>

namespace openglyph {

class SceneRenderer
{
public:
    explicit SceneRenderer(khepri::renderer::Renderer& renderer) : m_renderer(renderer) {}

    void render_scene(const openglyph::Scene& scene, const khepri::renderer::Camera& camera);

private:
    khepri::renderer::Renderer& m_renderer;
};

} // namespace openglyph
