#pragma once

#include "button.hpp"
#include "frame.hpp"
#include "label.hpp"
#include "listbox.hpp"

#include <khepri/utility/signal.hpp>

namespace khepri::ui {

class Combobox : public Widget
{
public:
    struct Layout : public Widget::Layout
    {
        /**
         * Width of the dropdown button, in canvas pixels.
         */
        unsigned long dropdown_button_width{0};

        /**
         * Width of the dropdown listbox' scrollbar, in canvas pixels.
         */
        unsigned long dropdown_scrollbar_width;

        /**
         * Height of the dropdown listbox' up/down button, in canvas pixels.
         */
        unsigned long dropdown_updown_button_height;

        /**
         * Size of the dropdown listbox' track button, in canvas pixels.
         * The track button will be centered on the track.
         */
        Size dropdown_track_button_size;
    };

    /**
     * Properties that define a combobox's style
     */
    struct Style
    {
        /// Style of the combobox's frame
        Frame::Style frame;

        /// Style of the listbox's label
        Label::Style label;

        /// Style for the dropdown button
        Button::Style dropdown_button;

        /// Style for the dropdown listbox
        Listbox::Style dropdown_listbox;
    };

    /**
     * Constructs the combobox
     * \param layout the layout for this combobox
     */
    explicit Combobox(const Layout& layout);

    /**
     * Sets the combobox's style
     *
     * \param[in] style the combobox's style.
     */
    void style(const Style& style) noexcept;

    /**
     * Sets the combobox's items
     *
     * \param[in] items the new items
     */
    void items(gsl::span<const std::u16string> items);

    /**
     * Clears the combobox's items and selection
     */
    void clear();

    /// \see Widget::enabled()
    void enabled(bool enabled) noexcept override;

    /**
     * Returns the index of the current selection, if any.
     */
    std::optional<int> selection() const noexcept
    {
        return m_selection;
    }

    /**
     * Selects the item at the specified index.
     *
     * @param index item to select, or nullopt to clear the selection.
     *
     * @throws #khepri::ArgumentError if the index is out of range.
     */
    void select(std::optional<int> index);

    /**
     * Adds a selection listener.
     *
     * The listener is notified whenever the selection changes.
     */
    Connection add_selection_listener(const Slot<void()>& listener);

protected:
    /// \see Widget::can_focus
    bool can_focus() override
    {
        return true;
    }

private:
    void open_listbox();

    void close_listbox();

    std::shared_ptr<Frame>  m_frame;
    std::shared_ptr<Label>  m_text;
    std::shared_ptr<Button> m_button;

    std::vector<std::u16string> m_items;
    std::optional<int>          m_selection;
    khepri::Signal<void()>      m_selection_changed;

    unsigned long              m_dropdown_scrollbar_width;
    unsigned long              m_dropdown_updown_button_height;
    khepri::Size               m_dropdown_track_button_size;
    khepri::ui::Listbox::Style m_listbox_style;
    // The listbox, when opened
    std::shared_ptr<khepri::ui::Listbox> m_listbox;
    // Connections for listbox events
    khepri::ScopedConnections m_listbox_connections;
};

} // namespace khepri::ui
