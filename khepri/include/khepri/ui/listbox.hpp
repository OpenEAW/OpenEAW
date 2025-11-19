#pragma once

#include "align.hpp"
#include "frame.hpp"
#include "scroll_panel.hpp"
#include "widget.hpp"

#include <khepri/exceptions.hpp>
#include <khepri/font/font.hpp>
#include <khepri/math/color_rgba.hpp>

#include <memory>
#include <string_view>

namespace khepri::ui {

/**
 * A widget with selectable items
 */
class Listbox : public Widget
{
public:
    using Layout = ScrollPanel::Layout;

    /**
     * Properties that define a listbox's style
     */
    struct Style
    {
        /// The style of the listbox's scroll panel
        ScrollPanel::Style scroll_panel;

        /// The font for the listbox
        std::shared_ptr<khepri::font::Font> font;

        /// Backgound color of selected items
        ColorRGBA selection_color{1.0f, 1.0f, 1.0f, 0.5f};
    };

    /**
     * Constructs the listbox
     *
     * \param layout the layout for this listbox
     */
    explicit Listbox(const Layout& layout) noexcept
        : Widget(layout)
        , m_scroll_panel(std::make_shared<ScrollPanel>(
              ScrollPanel::Layout{Layout::fill(), layout.scrollbar_width,
                                  layout.updown_button_height, layout.track_button_size}))
    {
        add(m_scroll_panel);

        items({u"Lorem 1 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 2 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 3 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 4 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 5 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 6 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 7 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 8 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 9 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 10 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 11 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 12 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 13 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 14 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 15 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 16 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 17 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 18 ipsum dolor sit amet, consectetuer adipiscing elit.",
               u"Lorem 19 ipsum dolor sit amet, consectetuer adipiscing elit."});
    }

    /**
     * Sets the listbox style.
     *
     * \param[in] style the listbox's style.
     */
    void style(const Style& style) noexcept
    {
        m_scroll_panel->style(style.scroll_panel);
        m_font = style.font;
        update_items_layout();
    }

    /**
     * Retrieves the listbox's items
     */
    const auto& items() const noexcept
    {
        return m_items;
    }

    /**
     * Sets the listbox's items
     *
     * \param[in] items the new items
     */
    void items(const std::vector<std::u16string>& items)
    {
        clear();
        m_items = items;
        m_scroll_panel->clear();

        for (const auto& text : m_items) {
            auto label = std::make_shared<Label>(Label::Layout::top_left({0, 0}, {0, 0}));
            label->text(text);
            m_labels.push_back(label);
            m_scroll_panel->add(label);
        }
        update_items_layout();
    }

    /**
     * Clears the listbox.
     *
     * Removes all items from the listbox.
     */
    void clear()
    {
        m_items.clear();
        m_labels.clear();
        m_scroll_panel->clear();
    }

    /**
     * Selects the item at the specified index.
     *
     * @param index item to select, or nullopt to clear the selection.

     * @throws #khepri::ArgumentError if the index is out of range.
     */
    void select(std::optional<int> index)
    {
        if (index && *index < 0 && *index >= m_items.size()) {
            throw khepri::ArgumentError();
        }

        if (m_selection != index) {
            m_selection = index;
            update_selection_quad();
        }
    }

    gsl::span<const Quad> render(khepri::renderer::Renderer& renderer) noexcept override
    {
        if (m_selection) {
            return {&m_selection_quad, 1};
        }
        return {};
    }

private:
    /// \see Widget::on_event()
    void on_event(const Event& e) override
    {
        if (auto* mpe = std::get_if<MousePressEvent>(&e)) {
            if (mpe->button == MouseButton::left) {
                if (m_font) {
                    const auto line_height = m_font->height();

                    int y = mpe->cursor_position.y - m_scroll_panel->calculated_layout().y +
                            m_scroll_panel->scroll_position();
                    int index = y / line_height;
                    if (index >= 0 && index < m_items.size()) {
                        select(index);
                    }
                }
            }
        }
    }

    void on_layout() noexcept override
    {
        update_selection_quad();
    }

    void update_selection_quad()
    {
        /*if (m_selection && m_font) {
            const auto line_height = m_font->height();

            const auto& layout = calculated_layout();
            const auto  selection_top =
                layout.y - m_scroll_panel->scroll_position() + *m_selection * line_height;

            m_selection_quad.area = Rect{layout.x, selection_top, layout.width, line_height};
        }*/
    }

    void update_items_layout()
    {
        if (m_font) {
            const auto line_height = m_font->height();

            int y = 0;
            for (const auto& label : m_labels) {
                label->layout(Label::Layout::top_left({0, y}, {0, line_height}));
                label->style({m_font, TextAlign::left});
                y += line_height;
            }
        }
    }

    std::shared_ptr<khepri::font::Font> m_font;
    std::vector<std::u16string>         m_items;
    std::vector<std::shared_ptr<Label>> m_labels;

    std::optional<int> m_selection;
    Quad               m_selection_quad;

    std::shared_ptr<ScrollPanel> m_scroll_panel;
};

} // namespace khepri::ui
