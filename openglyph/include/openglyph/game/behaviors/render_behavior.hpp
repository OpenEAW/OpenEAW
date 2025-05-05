#pragma once

#include <khepri/scene/behavior.hpp>
#include <openglyph/renderer/render_model.hpp>

namespace openglyph {

class RenderBehavior : public khepri::scene::Behavior
{
public:
    struct Type
    {
        std::string model_name;
    };

    explicit RenderBehavior(const renderer::RenderModel& model) : m_model(model) {}

    const auto& model() const noexcept
    {
        return m_model;
    }

    [[nodiscard]] double scale() const noexcept
    {
        return m_scale;
    }

    void scale(double scale) noexcept
    {
        m_scale = scale;
    }

private:
    const renderer::RenderModel& m_model;
    double                       m_scale{1.0};
};

} // namespace openglyph
