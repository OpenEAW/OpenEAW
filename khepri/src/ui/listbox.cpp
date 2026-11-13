#include <khepri/exceptions.hpp>
#include <khepri/ui/listbox.hpp>

namespace khepri::ui {

Listbox::Listbox(const Layout& layout) noexcept
    : Widget(layout)
    , m_scroll_panel(std::make_shared<ScrollPanel>(
          ScrollPanel::Layout{Layout::fill(), layout.scrollbar_width, layout.updown_button_height,
                              layout.track_button_size}))
    , m_selection_rect(
          std::make_shared<Rectangle>(Rectangle::Layout{{0, 0}, {0, 0}, {1, 0}, {0, 0}}))
{
    m_scroll_panel->add(m_selection_rect);
    add(m_scroll_panel);
}

void Listbox::style(const Style& style) noexcept
{
    m_scroll_panel->style(style.scroll_panel);
    m_selection_rect->style({style.selection_material, style.selection_color});
    m_labels_style = style.labels;
    m_line_height  = style.line_height;
    update_items_layout();
}

void Listbox::items(gsl::span<const std::u16string> items)
{
    clear();
    m_items = {items.begin(), items.end()};

    for (const auto& text : m_items) {
        auto label = std::make_shared<Label>(
            Label::Layout::top_left({static_cast<long>(m_left_padding), 0}, {0, 0}));
        label->text(text);
        m_labels.push_back(label);
        m_scroll_panel->add(label);
    }
    update_items_layout();
}

void Listbox::clear()
{
    select(std::nullopt);
    m_items.clear();
    m_labels.clear();
    m_scroll_panel->clear();
    m_scroll_panel->add(m_selection_rect);
}

void Listbox::select(std::optional<int> index)
{
    if (index && *index < 0 && *index >= m_items.size()) {
        throw khepri::ArgumentError();
    }

    if (m_selection != index) {
        m_selection = index;

        if (!m_selection) {
            m_selection_rect->visible(false);
        } else if (m_labels_style.font != Renderer::INVALID_FONT) {
            const auto selection_top = *m_selection * m_line_height;

            // Note #1: can't use {1.0, 0} for the right side because the content of the scroll
            // panel is placed in a 0x0-sized container.
            // Note #2: calculated_layout() is probably too wide, since the frame margins and
            // the scrollbar are also part of it, but the clipping will take care of that, so it
            // should be fine.
            m_selection_rect->visible(true);
            m_selection_rect->layout({{0, 0},
                                      {0, static_cast<long>(selection_top)},
                                      {0, static_cast<long>(calculated_layout().width)},
                                      {0, static_cast<long>(selection_top + m_line_height)}});
        }

        m_selection_changed();
    }
}

Connection Listbox::add_selection_listener(const Slot<void()>& listener)
{
    return m_selection_changed.connect(listener);
}

void Listbox::on_event(const Event& e)
{
    if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
        if (mpe->button == MouseButton::left) {
            if (m_labels_style.font != Renderer::INVALID_FONT) {
                int y = mpe->cursor_position.y - m_scroll_panel->calculated_layout().y +
                        m_scroll_panel->scroll_position();
                int index = y / m_line_height;
                if (index >= 0 && index < m_items.size()) {
                    select(index);
                }
            }
        }
    }
}

void Listbox::update_items_layout()
{
    if (m_labels_style.font != Renderer::INVALID_FONT) {
        int y = 0;
        for (const auto& label : m_labels) {
            label->layout(Label::Layout::top_left({static_cast<long>(m_left_padding), y},
                                                  {0, m_line_height}));
            label->style(m_labels_style);
            y += m_line_height;
        }
    }
}

} // namespace khepri::ui
