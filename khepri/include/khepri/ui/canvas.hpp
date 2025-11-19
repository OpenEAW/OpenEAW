#pragma once

#include "events.hpp"
#include "renderer.hpp"
#include "widget.hpp"

#include <khepri/math/math.hpp>
#include <khepri/math/size.hpp>
#include <khepri/renderer/material.hpp>
#include <khepri/renderer/renderer.hpp>

#include <memory>
#include <vector>

namespace khepri::ui {

/**
 * UI Canvas.
 *
 * The canvas is the container for all UI widgets and handles rendering and input event dispatching
 * for all widgets added to the canvas. UI widgets are isolated to a single canvas. Widgets in
 * different canvases cannot interact.
 *
 * A canvas has a virtual pixel size. If this size doesn't match the render target's resolution,
 * the UI if effectively up- or downscaled. All positions and sizes of widgets in the canvas are in
 * its virtual space.
 */
class Canvas final
{
    friend class Widget;

public:
    /**
     * Constructs a canvas.
     *
     * \param[in] size virtual canvas size.
     * \param[in] renderer renderer for rendering the UI with.
     */
    explicit Canvas(Size& size, Renderer& renderer) : m_size(size), m_renderer(renderer) {}

    /**
     * Returns the size of the canvas.
     */
    const Size& size() const noexcept
    {
        return m_size;
    }

    /**
     * Sets the size of the canvas.
     */
    void size(const Size& size) noexcept;

    /**
     * Sets the cursor position, relative to the canvas.
     *
     * \param[in] position the mouse cursor at the time of the event.
     *
     * This will generate and dispatch related mouse UI events to the relevant widgets attached to
     * the canvas based on their position.
     */
    void on_cursor_position(const MousePosition& position) noexcept;

    /**
     * Handles a mouse button event.
     *
     * \param[in] position the mouse cursor at the time of the event.
     * \param[in] button the mouse button that the event applies to.
     * \param[in] pressed indicates if the button was pressed or released.
     *
     * This will generate and dispatch related mouse UI events to the relevant widgets attached to
     * the canvas based on their position.
     */
    void on_mouse_button(const MousePosition& position, const MouseButton& button, bool pressed);

    /**
     * Handles a mouse scroll event.
     *
     * \param[in] position the mouse cursor at the time of the event.
     * \param[in] offset the mouse scroll offset.
     *
     * This will generate and dispatch related mouse UI events to the relevant widgets attached to
     * the canvas based on their position.
     */
    void on_mouse_scroll(const MousePosition& position, const Vector2& offset);

    /**
     * Adds a widget to the root of the canvas.
     *
     * \param[in] widget widget to add
     * \throws #khepri::ArgumentError if the widget is already added to a canvas or widget.
     *
     * The canvas holds a reference to the widget as long as its added.
     */
    void add(const std::shared_ptr<Widget>& widget);

    /**
     * Removes a widget from the root of the canvas.
     *
     * \param[in] widget widget to remove
     * \throws #khepri::ArgumentError if the widget is not attached to the root of this canvas.
     */
    void remove(const std::shared_ptr<Widget>& widget);

    /**
     * Renders the canvas
     */
    void render();

private:
    struct SpriteGroup;

    // Called by widgets when their visibility/enabled state changes
    void update_cursor_target();

    void append_widget(std::vector<Renderer::Quad> quads, Widget& widget, Rect clip_rect) const noexcept;

    static void set_canvas_recursive(Widget& widget, Canvas* canvas) noexcept;

    std::shared_ptr<Widget> find_mouse_target_widget(const MousePosition& point);

    std::shared_ptr<Widget> find_visible_widget(const std::vector<std::shared_ptr<Widget>>& widgets,
                                                const Pointi&                               point);

    void send_event(Widget& widget, const Event& event);

    static Widget* find_highest_inactive_widget(Widget& widget);

    void trickle_event_down(Widget& widget, const Event& event);

    void bubble_event_up(Widget& widget, const Event& event);

    void set_capture(const Widget& widget);
    void release_capture(const Widget& widget);

private:
    Size m_size;

    Renderer& m_renderer;

    std::vector<std::shared_ptr<Widget>> m_root_widgets;

    //
    // Dynamic state
    //

    // Current cursor position;
    Pointi m_cursor_position;

    // Current mouse-over widget, used to send mouse-enter/mouse-leave events
    std::weak_ptr<Widget> m_mouse_over_widget;

    // Current widget that's capturing mouse events
    std::weak_ptr<Widget> m_capture_widget;

    // Widget that's currently processing the current mouse event
    std::weak_ptr<Widget> m_mouse_event_widget;
};

} // namespace khepri::ui
