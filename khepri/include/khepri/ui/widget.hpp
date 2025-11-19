#pragma once

#include "events.hpp"
#include "layout.hpp"
#include "renderer.hpp"

#include <gsl/gsl-lite.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace khepri::ui {

class Canvas;

/**
 * A generic UI widget.
 *
 * A widget is the he fundamental type in the UI system.
 * Everything in a canvas is a widget: whether interactive, or static. Visible or invisible.
 * All widgets have a position and (rectangular) size. Any widget can have child widgets.
 *
 * All widgets can see and handle input events (key presses, mouse events, etc.). These events first
 * trickle down from the root of the canvas to the target of the event (based on event position or
 * current focus), then bubble up. Any widget in the chain has the chance to see or handle the event
 * at either stage.
 *
 * Widgets can be disabled. A widget that is disabled or a descendant of a disabled widget cannot
 * receive focus or input events. A focused widget loses focus when disabled.
 *
 * Widgets can be hidden. A widget that is hidden or a descendant of a hidden widget will not be
 * rendered and is effectively the same as a disabled widget for input handling.
 */
class Widget : public std::enable_shared_from_this<Widget>
{
    friend class Canvas;

public:
    using Layout = khepri::ui::Layout;

    /**
     * Constructs the widget
     *
     * \param layout layout information for the widget
     */
    Widget(const Layout& layout) noexcept : m_layout(layout) {}
    virtual ~Widget();

    /**
     * Sets the name of the widget.
     *
     * A widget's name is not used by the UI itself, but is free to be used by the application.
     */
    void name(std::string_view name)
    {
        m_name = std::string(name);
    }

    /**
     * Retrieves the name of the widget.
     *
     * A widget's name is not used by the UI itself, but is free to be used by the application.
     * By default, a widget's name is empty.
     */
    std::string_view name() const noexcept
    {
        return m_name;
    }

    /// Indicates if the widget is currently enabled
    auto enabled() const noexcept
    {
        return m_enabled;
    }

    /// Enables or disables the widget
    virtual void enabled(bool enabled) noexcept;

    /// Returns the widget own visibility state
    auto visible() const noexcept
    {
        return m_visible;
    }

    /// Shows or hides the widget
    void visible(bool visible) noexcept;

    /// Returns the widget's parent.
    /// Returns nullptr for root widgets.
    Widget* parent() const noexcept
    {
        return m_parent;
    }

    /// Returns the widget's children
    const auto& children() const noexcept
    {
        return m_children;
    }

    /**
     * Finds a named widget of specific type as (grand)child of this widget.
     *
     * \tparam WidgetType the expected type of the widget.
     * \param[in] name the name of the widget to find.
     *
     * \return the child widget with specified name and specified type, or nullptr if not found.
     */
    template <typename WidgetType = Widget>
    std::shared_ptr<WidgetType> find_child(std::string_view name) const noexcept
    {
        return std::dynamic_pointer_cast<WidgetType>(find_child_widget(name));
    }

    /**
     * Changes the widget's layout
     *
     * \param[in] layout the new layout
     */
    void layout(const Layout& layout) noexcept;

    /**
     * Returns the calculated layout rectangle for this widget.
     *
     * The rectangle can change on layout events or over time for animated widgets.
     * \note To get the size of widget including possibly overflowing descendent widgets, use \ref
     * calculated_bounds.
     */
    auto calculated_layout() const noexcept
    {
        return m_calculated_layout;
    }

    /**
     * Returns the calculated bounding box for this widget and its child widgets.
     *
     * This rectangle includes the size of possibly overflowing descendent widgets.
     * The rectangle can change on layout events or over time for animated widgets.
     */
    auto calculated_bounds() const noexcept
    {
        return m_calculated_bounds;
    }

    /**
     * Returns the quads needed to render this widget at the given size.
     * Renderable widgets should override this method and return their quads.
     *
     * \param[in] clip_rect the active clip rect for the rendering context (in canvas space).
     */
    virtual gsl::span<const Renderer::Quad> render(const Rect& clip_rect) noexcept
    {
        return {};
    }

    /**
     * Returns the rectangle used to clip this widget and its children.
     * Returns no rectangle if this widget and its children are allowed to overflow the widget
     * bounds.
     *
     * \note the rectangle is relative to the top-left coordinate of the widget.
     */
    virtual std::optional<Rect> clip() const noexcept
    {
        return {};
    }

    /**
     * Call this method during the handling of a mousedown event to retarget all mouse events to
     * this element until the mouse button is released or #release_capture() is called.
     */
    void set_capture();

    /**
     * Releases mouse capture if it was currently enabled on this widget with #set_capture().
     */
    void release_capture();

protected:
    /**
     * Adds a widget as a child of this widget.
     *
     * \param widget widget to add
     * \throws #khepri::ArgumentError if the widget is already added to a canvas or widget, or when
     * trying to add a widge to itself.
     *
     * This widget holds a reference to the child widget as long as its added.
     */
    void add(const std::shared_ptr<Widget>& widget);

    /**
     * Removes a widget as a child from this widget.
     *
     * \param widget widget to remove
     * \throws #khepri::ArgumentError if the widget is not a child of this widget
     */
    void remove(const std::shared_ptr<Widget>& widget);

    /**
     * Removes all child widgets.
     */
    void clear();

    /**
     * Called when the widget's layout has changed.
     *
     * This method can be overriden by widgets to trigger layout-dependent logic.
     * Use \ref calculated_layout() to get the widget's new layout.
     *
     * \note \a on_layout of a widget's children are called before its own \a on_layout is
     * called, but after its own calculated rectangle has been updated.
     */
    virtual void on_layout() noexcept {}

    /**
     * Called before an event is dispatched to the widget or its children.
     *
     * Override this method to intercept and block events from reaching the widget's children.
     */
    virtual void pre_event(const Event& event) {}

    /**
     * Called as an event is dispatched to the widget.
     *
     * Override this method to handle an event.
     */
    virtual void on_event(const Event& event) {}

private:
    // Calculate the laid out position based on layout and parent rect
    void do_layout() noexcept;

    static void set_canvas_recursive(Widget& widget, Canvas* canvas) noexcept;

    std::shared_ptr<Widget> find_child_widget(std::string_view name) const noexcept;

private:
    Layout      m_layout;
    bool        m_enabled{true};
    bool        m_visible{true};
    std::string m_name;

    Canvas* m_canvas{nullptr};
    Widget* m_parent{nullptr};

    // Calculated bounding box (excludes child widgets) based on layout
    Rect m_calculated_layout{0, 0, 0, 0};

    // Calculated bounding box (includes child widgets) based on layout
    Rect m_calculated_bounds{0, 0, 0, 0};

    std::vector<std::shared_ptr<Widget>> m_children;
};

} // namespace khepri::ui
