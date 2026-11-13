#include <khepri/exceptions.hpp>
#include <khepri/ui/canvas.hpp>
#include <khepri/ui/combobox.hpp>

namespace khepri::ui {

Combobox::Combobox(const Layout& layout)
    : Widget(layout)
    , m_frame(std::make_shared<Frame>(Layout::fill()))
    , m_text(std::make_shared<Label>(khepri::ui::Layout{
          {0.0, 0}, {0.0, 0}, {1.0, -static_cast<long>(layout.dropdown_button_width)}, {1.0, 0}}))
    , m_button(std::make_shared<Button>(khepri::ui::Layout{
          {1.0, -static_cast<long>(layout.dropdown_button_width)}, {0.0, 0}, {1.0, 0}, {1.0, 0}}))
    , m_dropdown_scrollbar_width(layout.dropdown_scrollbar_width)
    , m_dropdown_updown_button_height(layout.dropdown_updown_button_height)
    , m_dropdown_track_button_size(layout.dropdown_track_button_size)
{
    m_frame->add(m_text);
    m_frame->add(m_button);
    add(m_frame);

    m_button->add_click_listener([this] {
        if (m_listbox) {
            close_listbox();
        } else {
            open_listbox();
        }
    });
}

/**
 * Sets the combobox's style
 *
 * \param[in] style the combobox's style.
 */
void Combobox::style(const Style& style) noexcept
{
    m_frame->style(style.frame);
    m_text->style(style.label);
    m_button->style(style.dropdown_button);
    m_listbox_style = style.dropdown_listbox;
}

/**
 * Sets the combobox's items
 *
 * \param[in] items the new items
 */
void Combobox::items(gsl::span<const std::u16string> items)
{
    m_items = {items.begin(), items.end()};
}

/**
 * Clears the combobox's items and selection
 */
void Combobox::clear()
{
    items({});
}

/// \see Widget::enabled()
void Combobox::enabled(bool enabled) noexcept
{
    Widget::enabled(enabled);
    m_button->enabled(enabled);
}

/**
 * Selects the item at the specified index.
 *
 * @param index item to select, or nullopt to clear the selection.
 *
 * @throws #khepri::ArgumentError if the index is out of range.
 */
void Combobox::select(std::optional<int> index)
{
    if (index && *index < 0 && *index >= m_items.size()) {
        throw khepri::ArgumentError();
    }

    if (m_selection != index) {
        m_selection = index;

        m_text->text(m_selection ? m_items[*m_selection] : u"");

        m_selection_changed();
    }
}

/**
 * Adds a selection listener.
 *
 * The listener is notified whenever the selection changes.
 */
Connection Combobox::add_selection_listener(const Slot<void()>& listener)
{
    return m_selection_changed.connect(listener);
}

void Combobox::open_listbox()
{
    if (!m_listbox) {
        const auto& bounds = calculated_bounds();
        m_listbox          = std::make_shared<khepri::ui::Listbox>(khepri::ui::Listbox::Layout{
            khepri::ui::Layout::top_left({bounds.x, bounds.y + static_cast<long>(bounds.height)},
                                                  {bounds.width, 200}),
            m_dropdown_scrollbar_width, m_dropdown_updown_button_height,
            m_dropdown_track_button_size});

        m_listbox->style(m_listbox_style);
        m_listbox->items(m_items);
        m_listbox_connections.add(m_listbox->add_selection_listener([this] {
            assert(m_listbox);
            select(m_listbox->selection());
            // Choosing an item in the listbox closes it
            close_listbox();
        }));
        canvas()->add(m_listbox);
    }
    m_listbox->focus();
}

void Combobox::close_listbox()
{
    if (m_listbox) {
        m_listbox_connections = ScopedConnections();
        canvas()->remove(m_listbox);
        m_listbox = nullptr;
    }
}

} // namespace khepri::ui
