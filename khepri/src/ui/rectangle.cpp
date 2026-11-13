#include <khepri/ui/rectangle.hpp>

namespace khepri::ui {

Rectangle::Rectangle(const Layout& layout) noexcept : Widget(layout) {}

void Rectangle::render(Renderer& renderer, const Rect& clip_rect) noexcept
{
    renderer.render_rects({&m_rect, 1}, clip_rect);
}

void Rectangle::style(const Style& style) noexcept
{
    m_rect.material_id = style.material;
    m_rect.color       = style.color;
}

void Rectangle::on_layout() noexcept
{
    m_rect.area = calculated_layout();
}

} // namespace khepri::ui
