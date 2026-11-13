#include <khepri/ui/label.hpp>

namespace khepri::ui {

void Label::render(Renderer& renderer, const Rect& clip_rect) noexcept
{
    m_text.area = calculated_layout();
    renderer.render_texts({&m_text, 1}, clip_rect);
}

void Label::style(const Style& style) noexcept
{
    m_text.font_id     = style.font;
    m_text.material_id = style.material;
    m_text.text_align  = style.text_align;
}

void Label::text(std::u16string_view text)
{
    m_text.string = {text.begin(), text.end()};
}

void Label::clear()
{
    text({});
}

} // namespace khepri::ui
