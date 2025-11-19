#include <khepri/exceptions.hpp>
#include <khepri/ui/canvas.hpp>
#include <khepri/ui/widget.hpp>

namespace khepri::ui {

Widget::~Widget()
{
    // Make sure we do not track this widget for mouse event capturing
    release_capture();
}

void Widget::layout(const Layout& layout) noexcept
{
    m_layout = layout;
    do_layout();
}

void Widget::visible(bool visible) noexcept
{
    if (m_visible != visible) {
        m_visible = visible;
        if (m_canvas) {
            // We want the canvas to re-evaluate the target of mouse events
            m_canvas->update_cursor_target();
        }
    }
}

void Widget::enabled(bool enabled) noexcept
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        if (m_canvas) {
            // We want the canvas to re-evaluate the target of mouse events
            m_canvas->update_cursor_target();
        }
    }
}

void Widget::add(const std::shared_ptr<Widget>& widget)
{
    if (widget.get() == this || widget->m_parent != nullptr || widget->m_canvas != nullptr) {
        throw ArgumentError();
    }
    assert(std::find(m_children.begin(), m_children.end(), widget) == m_children.end());
    m_children.push_back(widget);
    widget->m_parent = this;
    set_canvas_recursive(*widget, m_canvas);

    widget->do_layout();
}

void Widget::remove(const std::shared_ptr<Widget>& widget)
{
    if (widget->m_parent != this) {
        throw ArgumentError();
    }
    assert(widget->m_canvas == m_canvas);
    const auto it = std::remove(m_children.begin(), m_children.end(), widget);
    assert(it != m_children.end());
    m_children.erase(it, m_children.end());
    widget->m_parent = nullptr;
    set_canvas_recursive(*widget, nullptr);
}

void Widget::clear()
{
    for (auto& child : m_children) {
        child->m_parent = nullptr;
        set_canvas_recursive(*child, nullptr);
    }
    m_children.clear();
}

std::shared_ptr<Widget> Widget::find_child_widget(std::string_view name) const noexcept
{
    for (const auto& child_widget : m_children) {
        if (child_widget->name() == name) {
            return child_widget;
        }

        if (auto child = child_widget->find_child(name)) {
            return child;
        }
    }
    return nullptr;
}

// Calculate the laid out position based on layout and parent rect
void Widget::do_layout() noexcept
{
    if (m_canvas == nullptr) {
        // If the widget's not attached, there's no point in laying it out
        return;
    }

    khepri::Rect parent_rect;
    if (m_parent != nullptr) {
        parent_rect = m_parent->calculated_layout();
    } else {
        const auto& size = m_canvas->size();
        parent_rect      = {0, 0, size.width, size.height};
    }

    const long parent_right  = parent_rect.x + parent_rect.width;
    const long parent_bottom = parent_rect.y + parent_rect.height;

    const auto& apply_layout = [](const Anchor& anchor, long pos0, long pos1) {
        return khepri::lerp(pos0, pos1, anchor.parent_frac) + anchor.offset;
    };

    // Calculate our borders based on our layout
    auto left   = apply_layout(m_layout.left, parent_rect.x, parent_right);
    auto top    = apply_layout(m_layout.top, parent_rect.y, parent_bottom);
    auto right  = apply_layout(m_layout.right, parent_rect.x, parent_right);
    auto bottom = apply_layout(m_layout.bottom, parent_rect.y, parent_bottom);

    // Sanity correction
    if (left >= right) {
        left = right = (left + right) / 2;
    }

    if (top >= bottom) {
        top = bottom = (top + bottom) / 2;
    }

    // This is the bounding box for the widget itself, disregarding possibly overflowing child
    // widgets
    m_calculated_layout = {left, top, static_cast<unsigned long>(right - left),
                           static_cast<unsigned long>(bottom - top)};
    m_calculated_bounds = m_calculated_layout;

    // Our bbox has changed; layout our children too
    for (const auto& child_widget : m_children) {
        child_widget->do_layout();

        m_calculated_bounds = combine(m_calculated_bounds, child_widget->calculated_bounds());
    }

    // Notify the widget that the layout has changed
    on_layout();
}

void Widget::set_canvas_recursive(Widget& widget, Canvas* canvas) noexcept
{
    widget.m_canvas = canvas;
    for (const auto& child : widget.children()) {
        set_canvas_recursive(*child, canvas);
    }
}

void Widget::set_capture()
{
    assert(m_canvas != nullptr);
    m_canvas->set_capture(*this);
}

/**
 * Releases mouse capture if it was currently enabled on this widget with #set_capture().
 */
void Widget::release_capture()
{
    if (m_canvas != nullptr) {
        m_canvas->release_capture(*this);
    }
}

} // namespace khepri::ui
