#pragma once

#include <khepri/scene/behavior.hpp>

#include <openglyph/renderer/render_model.hpp>

namespace openglyph {

class RenderBehavior : public khepri::scene::Behavior
{
public:
    enum class RenderLayer
    {
        // Background layer
        background,
        // Foreground layer
        foreground
    };

    explicit RenderBehavior(const renderer::RenderModel& model) : m_model(model) {}

    [[nodiscard]] const auto& model() const noexcept
    {
        return m_model;
    }

    [[nodiscard]] double scale() const noexcept
    {
        return m_scale;
    }

    [[nodiscard]] RenderLayer render_layer() const noexcept
    {
        return m_render_layer;
    }

    void scale(double scale) noexcept
    {
        m_scale = scale;
    }

    void render_layer(RenderLayer render_layer) noexcept
    {
        m_render_layer = render_layer;
    }

private:
    const renderer::RenderModel& m_model;
    double                       m_scale{1.0};
    RenderLayer                  m_render_layer{RenderLayer::foreground};
};

} // namespace openglyph
