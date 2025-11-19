#include <khepri/exceptions.hpp>
#include <khepri/renderer/sprite.hpp>
#include <khepri/ui/canvas.hpp>

#include <gsl/gsl-lite.hpp>

#include <cassert>
namespace khepri::ui {

struct Canvas::SpriteGroup
{
    std::vector<khepri::renderer::Sprite> sprites;

    /// The material to render this sprite group with
    class khepri::renderer::Material* material;

    /// Material parameters for this sprite group
    khepri::renderer::Material::Param texture_param;

    /// Scissor rect to use for this group
    Rect scissor_rect{0, 0, 0, 0};
};

void Canvas::size(const Size& size) noexcept
{
    if (m_size != size) {
        m_size = size;

        // Re-layout widgets
        for (auto& widget : m_root_widgets) {
            widget->do_layout();
        }
    }
}

void Canvas::on_cursor_position(const MousePosition& position) noexcept
{
    Pointi p{position.x, position.y};
    if (m_cursor_position != p) {
        m_cursor_position = p;
        update_cursor_target();
    }
}

void Canvas::update_cursor_target()
{
    // Check which widget the cursor is hovering over and send appropriate enter/leave/over events
    MousePosition cursor_position{m_cursor_position.x, m_cursor_position.y};
    if (auto widget = find_mouse_target_widget(cursor_position)) {
        if (auto w = m_mouse_over_widget.lock()) {
            if (widget != w) {
                send_event(*w, MouseLeaveEvent{cursor_position});
                send_event(*widget, MouseEnterEvent{cursor_position});
                m_mouse_over_widget = widget;
            }
        } else {
            send_event(*widget, MouseEnterEvent{cursor_position});
            m_mouse_over_widget = widget;
        }
        send_event(*widget, MouseMoveEvent{cursor_position});
    } else if (auto w = m_mouse_over_widget.lock()) {
        send_event(*w, MouseLeaveEvent{cursor_position});
        m_mouse_over_widget.reset();
    }
}

void Canvas::on_mouse_button(const MousePosition& position, const MouseButton& button, bool pressed)
{
    on_cursor_position(position);
    if (auto widget = find_mouse_target_widget(position)) {
        if (pressed) {
            send_event(*widget, MousePressEvent{position, button});
        } else {
            send_event(*widget, MouseReleaseEvent{position, button});
        }
    }
}

void Canvas::on_mouse_scroll(const MousePosition& position, const Vector2& offset)
{
    on_cursor_position(position);
    if (auto widget = find_mouse_target_widget(position)) {
        send_event(*widget, MouseScrollEvent{position, offset.x, offset.y});
    }
}

void Canvas::add(const std::shared_ptr<Widget>& widget)
{
    if (widget->m_parent != nullptr || widget->m_canvas != nullptr) {
        throw ArgumentError();
    }
    assert(std::find(m_root_widgets.begin(), m_root_widgets.end(), widget) == m_root_widgets.end());
    m_root_widgets.push_back(widget);
    set_canvas_recursive(*widget, this);

    // Layout the widget and its children
    widget->do_layout();

    // The added widget may now have mouse-over status, re-check it
    update_cursor_target();
}

void Canvas::remove(const std::shared_ptr<Widget>& widget)
{
    if (widget->m_canvas != this) {
        throw ArgumentError();
    }
    const auto it = std::remove(m_root_widgets.begin(), m_root_widgets.end(), widget);
    assert(it != m_root_widgets.end());
    m_root_widgets.erase(it, m_root_widgets.end());
    set_canvas_recursive(*widget, nullptr);

    // The removed widget may have had mouse-over status, re-check it
    update_cursor_target();
}

void Canvas::render()
{
    std::vector<Renderer::Quad> quads;
    for (auto& widget : m_root_widgets) {
        // Root clip rect is the entire canvas
        const Rect clip_rect = {0, 0, m_size.width, m_size.height};
        append_widget(quads, *widget, clip_rect);
    }
    m_renderer.render_quads(quads, m_size);
}

void Canvas::append_widget(std::vector<Renderer::Quad> quads, Widget& widget,
                           Rect clip_rect) const noexcept
{
    if (!widget.visible()) {
        return;
    }

    if (const auto widget_clip_rect = widget.clip()) {
        const auto& rect                 = widget.calculated_layout();
        const auto  abs_widget_clip_rect = offset(*widget_clip_rect, {rect.x, rect.y});
        const auto  new_clip_rect        = intersect(clip_rect, abs_widget_clip_rect);
        if (!new_clip_rect) {
            // The widget's clip rect does not intersect with the parent's, so there's nothing
            // to render.
            return;
        }
        clip_rect = *new_clip_rect;
    }

    for (const auto& quad : widget.render(clip_rect)) {
        quads.push_back(quad);
    }

    for (auto& child : widget.children()) {
        append_widget(quads, *child, clip_rect);
    }
}

// Finds the widget that should receive the mouse event at the specified point
std::shared_ptr<Widget> Canvas::find_mouse_target_widget(const MousePosition& point)
{
    if (auto widget = m_capture_widget.lock()) {
        return widget;
    }

    return find_visible_widget(m_root_widgets, {point.x, point.y});
}

// Finds the deepest visible widget in the tree at the specified point
std::shared_ptr<Widget>
Canvas::find_visible_widget(const std::vector<std::shared_ptr<Widget>>& widgets,
                            const Pointi&                               point)
{
    // Scan in reverse because later widgets visually overlap earlier widgets
    for (auto it = widgets.rbegin(); it != widgets.rend(); ++it) {
        if ((*it)->visible()) {
            if (auto child_widget = find_visible_widget((*it)->m_children, point)) {
                return child_widget;
            }

            if (inside(point, (*it)->calculated_layout())) {
                return *it;
            }
        }
    }
    return nullptr;
}

void Canvas::send_event(Widget& widget, const Event& event)
{
    // Find the highest inactive (hidden and/or disabled) widget above the target widget.
    // If found, its parent will receive the event.
    class Widget* event_target = &widget;
    /*    if (auto* highest_inactive = find_highest_inactive_widget(widget)) {
            event_target = highest_inactive->m_parent;
        }*/

    if (event_target != nullptr) {
        // Send the event
        trickle_event_down(*event_target, event);
        bubble_event_up(*event_target, event);
    }
}

Widget* Canvas::find_highest_inactive_widget(Widget& widget)
{
    if (widget.m_parent) {
        if (auto* result = find_highest_inactive_widget(*widget.m_parent)) {
            return result;
        }
    }
    if (!widget.enabled() || !widget.visible()) {
        return &widget;
    }
    return nullptr;
}

void Canvas::trickle_event_down(Widget& widget, const Event& event)
{
    if (widget.m_parent) {
        trickle_event_down(*widget.m_parent, event);
    }

    {
        if (std::holds_alternative<MousePressEvent>(event)) {
            // Store that this widget is handling the mouse event, for mouse capture
            m_mouse_event_widget = widget.weak_from_this();
        }
        auto _ = gsl::finally([this] { m_mouse_event_widget.reset(); });

        widget.pre_event(event);
    }
}

void Canvas::bubble_event_up(Widget& widget, const Event& event)
{
    {
        if (std::holds_alternative<MousePressEvent>(event)) {
            // Store that this widget is handling the mouse event, for mouse capture
            m_mouse_event_widget = widget.weak_from_this();
        }
        auto _ = gsl::finally([this] { m_mouse_event_widget.reset(); });

        widget.on_event(event);
    }

    if (widget.m_parent) {
        bubble_event_up(*widget.m_parent, event);
    }
}

void Canvas::set_canvas_recursive(Widget& widget, class Canvas* canvas) noexcept
{
    widget.m_canvas = canvas;
    for (const auto& child : widget.children()) {
        set_canvas_recursive(*child, canvas);
    }
}

void Canvas::set_capture(const Widget& widget)
{
    if (auto w = m_mouse_event_widget.lock()) {
        if (w.get() == &widget) {
            // This widget can set the capture
            m_capture_widget = w;
        }
    }
}

void Canvas::release_capture(const Widget& widget)
{
    if (m_capture_widget.lock().get() == &widget) {
        m_capture_widget.reset();

        // We may now be above another widget; notify them
        update_cursor_target();
    }
}

} // namespace khepri::ui
