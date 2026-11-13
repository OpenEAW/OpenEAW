#pragma once

#include "frame.hpp"
#include "rectangle.hpp"
#include "scroll_panel.hpp"
#include "widget.hpp"

#include <khepri/font/font.hpp>
#include <khepri/math/color_rgba.hpp>
#include <khepri/utility/signal.hpp>

#include <memory>

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

        /// The style for the listbox labels
        Label::Style labels;

        /// The material for rendering the selection
        Renderer::MaterialId selection_material{Renderer::INVALID_MATERIAL};

        /// Backgound color of selected items
        ColorRGBA selection_color{1.0f, 1.0f, 1.0f, 0.5f};

        /// Height of each line in the listbox, in canvas pixels.
        unsigned long line_height{20};

        // Left padding of the text in the listbox, in canvas pixels.
        unsigned long left_padding{3};
    };

    /**
     * Constructs the listbox
     *
     * \param layout the layout for this listbox
     */
    explicit Listbox(const Layout& layout) noexcept;

    /**
     * Sets the listbox style.
     *
     * \param[in] style the listbox's style.
     */
    void style(const Style& style) noexcept;

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
    void items(gsl::span<const std::u16string> items);

    /**
     * Clears the listbox's items and selection.
     */
    void clear();

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

    /// \see Widget::on_event()
    void on_event(const Event& e) override;

private:
    void update_items_layout();

    Label::Style                        m_labels_style;
    std::vector<std::u16string>         m_items;
    std::vector<std::shared_ptr<Label>> m_labels;

    // Style values
    unsigned long m_line_height{20};
    unsigned long m_left_padding{3};

    std::optional<int>         m_selection;
    std::shared_ptr<Rectangle> m_selection_rect;

    khepri::Signal<void()> m_selection_changed;

    std::shared_ptr<ScrollPanel> m_scroll_panel;
};

} // namespace khepri::ui
