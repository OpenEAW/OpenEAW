#pragma once

#include <khepri/math/point.hpp>
#include <khepri/math/size.hpp>
#include <khepri/ui/align.hpp>
#include <khepri/ui/button.hpp>
#include <khepri/ui/combobox.hpp>
#include <khepri/ui/listbox.hpp>
#include <khepri/ui/widget.hpp>

#include <gsl/gsl-lite.hpp>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace openglyph::ui {

struct CommonControlDefinition
{
    // The control's ID.
    int id{-1};

    // The control's name.
    std::string name;

    // The control's position, relative to the owning dialog.
    khepri::Pointi position;

    // The control's size.
    khepri::Size size;
};

struct PushButtonControlDefinition : CommonControlDefinition
{
    std::string           text;
    khepri::ui::TextAlign text_alignment{khepri::ui::TextAlign::center};
};

struct FrameControlDefinition : CommonControlDefinition
{};

struct LabelControlDefinition : CommonControlDefinition
{
    std::string           text;
    khepri::ui::TextAlign text_alignment{khepri::ui::TextAlign::left};
};

struct EditBoxControlDefinition : CommonControlDefinition
{
    khepri::ui::TextAlign text_alignment{khepri::ui::TextAlign::left};
};

struct ComboBoxControlDefinition : CommonControlDefinition
{};

struct ListBoxControlDefinition : CommonControlDefinition
{};

struct ScrollBarControlDefinition : CommonControlDefinition
{
    bool is_horizontal{false};
};

struct CheckBoxControlDefinition : CommonControlDefinition
{
    std::string text;
};

struct RadioButtonControlDefinition : CommonControlDefinition
{
    std::string text;
    bool        is_push_like{false};
};

struct TrackBarControlDefinition : CommonControlDefinition
{
    bool is_horizontal{true};
};

struct ProgressBarControlDefinition : CommonControlDefinition
{};

struct ImageControlDefinition : CommonControlDefinition
{
    std::string image_name;
};

using ControlDefinition =
    std::variant<PushButtonControlDefinition, FrameControlDefinition, LabelControlDefinition,
                 EditBoxControlDefinition, ComboBoxControlDefinition, ListBoxControlDefinition,
                 ScrollBarControlDefinition, CheckBoxControlDefinition,
                 RadioButtonControlDefinition, TrackBarControlDefinition,
                 ProgressBarControlDefinition, ImageControlDefinition>;

struct DialogDefinition
{
    int          id;
    std::string  name;
    khepri::Size size;
    std::string  title;
    std::string  font_name;

    std::vector<ControlDefinition> controls;
};

class UserInterface;

class Dialog final
{
public:
    /// Listener that's called when the dialog is closed.
    using CloseListener = std::function<void()>;

    Dialog(UserInterface& ui, std::shared_ptr<khepri::ui::Widget> root_widget)
        : m_ui(ui), m_root_widget(root_widget)
    {
    }

    /// Returns the user interface that this dialog belongs to.
    [[nodiscard]] UserInterface& ui() const noexcept
    {
        return m_ui;
    }

    void add_close_listener(const CloseListener& listener)
    {
        m_close_listeners.push_back(listener);
    }

    void show()
    {
        m_root_widget->visible(true);
    }

    void hide()
    {
        m_root_widget->visible(false);
    }

    // Adds a click listener to the specified button.
    // If the button with the specified name is not found, this function does nothing.
    template <typename Callable>
    void on_button(std::string_view button_name, Callable&& callable)
    {
        if (auto button = m_root_widget->find_child<khepri::ui::Button>(button_name)) {
            button->add_click_listener(std::forward<Callable>(callable));
        }
    }

    // Show or hide a widget.
    // If the widget with the specified name is not found, this function does nothing.
    void visible(std::string_view widget_name, bool visible)
    {
        if (auto widget = m_root_widget->find_child(widget_name)) {
            widget->visible(visible);
        }
    }

    // Disable a widget
    // If the widget with the specified name is not found, this function does nothing.
    void disable(std::string_view widget_name)
    {
        enabled(widget_name, false);
    }

    // Enable a widget
    // If the widget with the specified name is not found, this function does nothing.
    void enable(std::string_view widget_name)
    {
        enabled(widget_name, true);
    }

    // Enable or disable a widget
    // If the widget with the specified name is not found, this function does nothing.
    void enabled(std::string_view widget_name, bool enabled)
    {
        if (auto widget = m_root_widget->find_child(widget_name)) {
            widget->enabled(enabled);
        }
    }

    // Clears the contents (text, items) of the widget
    void clear(std::string_view widget_name)
    {
        if (auto widget = m_root_widget->find_child(widget_name)) {
            if (auto label = std::dynamic_pointer_cast<khepri::ui::Label>(widget)) {
                label->clear();
            } else if (auto button = std::dynamic_pointer_cast<khepri::ui::Button>(widget)) {
                button->clear();
            } else if (auto listbox = std::dynamic_pointer_cast<khepri::ui::Listbox>(widget)) {
                listbox->clear();
            } else if (auto combobox = std::dynamic_pointer_cast<khepri::ui::Combobox>(widget)) {
                combobox->clear();
            }
        }
    }

    // Sets the items on a multi-item select widget (e.g. combobox or listbox)
    void items(std::string_view widget_name, gsl::span<const std::u16string> items)
    {
        if (auto widget = m_root_widget->find_child(widget_name)) {
            if (auto listbox = std::dynamic_pointer_cast<khepri::ui::Listbox>(widget)) {
                listbox->items(items);
            } else if (auto combobox = std::dynamic_pointer_cast<khepri::ui::Combobox>(widget)) {
                combobox->items(items);
            }
        }
    }

    void close();

private:
    UserInterface&                      m_ui;
    std::shared_ptr<khepri::ui::Widget> m_root_widget;
    std::vector<CloseListener>          m_close_listeners;
};

// Base class for dialog controllers, which contain the logic for a dialog.
class DialogController
{
public:
    explicit DialogController(openglyph::ui::Dialog& dialog) : m_dialog(dialog) {}

    virtual ~DialogController() = default;

    /**
     * Called when the dialog is activated (i.e. when it is created and added to the
     * canvas), or when it is re-activated after being deactivated (i.e. when another dialog
     * has been closed).
     */
    virtual void on_activated() {}

    /**
     * Called when the dialog is deactivated (i.e. when another dialog is activated on top
     * of it, or when it is closed).
     */
    virtual void on_deactivated() {}

protected:
    [[nodiscard]] openglyph::ui::Dialog& dialog() const noexcept
    {
        return m_dialog;
    }

private:
    openglyph::ui::Dialog& m_dialog;
};

/**
 * Position of a dialog relative to the canvas.
 */
enum class DialogPosition
{
    center,
    top_left,
    top_middle,
    top_right,
    left_middle,
    right_middle,
    bottom_left,
    bottom_middle,
    bottom_right,
    full_screen,
};

} // namespace openglyph::ui