#include <khepri/ui/scroll_panel.hpp>

namespace khepri::ui {
namespace {
// Scrolling the amount of pixels indicated by the scrollwheel feels too slow, so speed it up.
static constexpr auto SCROLL_MULTIPLIER = 10;

// Default increment for the scrollbar. Scroll a bunch of pixels at a time.
static constexpr auto DEFAULT_SCROLL_STEP = 20;
} // namespace

// Container that clips the scroll panel's content
class ScrollPanel::ClipContainer : public Widget
{
public:
    using Widget::Widget;

    using Widget::add;
    using Widget::clear;
    using Widget::remove;

    std::optional<Rect> clip() const noexcept override
    {
        // Clip the child widgets to this container
        auto rect = calculated_layout();
        rect.x    = 0;
        rect.y    = 0;
        return rect;
    }
};

// Container for all the content in the scroll panel
class ScrollPanel::ContentContainer : public Widget
{
public:
    using Widget::Widget;

    using Widget::add;
    using Widget::clear;
    using Widget::remove;
};

ScrollPanel::ScrollPanel(const Layout& layout) noexcept
    : Widget(layout)
    , m_scrollbar_width(layout.scrollbar_width)
    , m_frame(std::make_shared<Frame>(Frame::Layout::fill()))
    , m_clip_container(std::make_shared<ClipContainer>(ClipContainer::Layout::fill()))
    , m_content(
          std::make_shared<ContentContainer>(ContentContainer::Layout::top_left({0, 0}, {0, 0})))
    , m_scrollbar(std::make_shared<Scrollbar>(Scrollbar::Layout{
          {{1, -static_cast<long>(layout.scrollbar_width)}, {0, 0}, {1, 0}, {1, 0}},
          layout.updown_button_height,
          layout.track_button_size}))
{
    Widget::add(m_frame);
    Widget::add(m_clip_container);
    Widget::add(m_scrollbar);

    // The ContentContainer is a convenient way to offset all content on scroll events.
    // It's actually 0x0 in size, but since content overflows, that's not a problem.
    m_clip_container->add(m_content);

    m_scrollbar->add_scroll_listener([this] { on_scroll(); });
    m_scrollbar->scroll_step(DEFAULT_SCROLL_STEP);

    resize_clip_container();
}

void ScrollPanel::add(const std::shared_ptr<Widget>& widget)
{
    m_content->add(widget);
}

void ScrollPanel::remove(const std::shared_ptr<Widget>& widget)
{
    m_content->remove(widget);
}

void ScrollPanel::clear()
{
    m_content->clear();
}

void ScrollPanel::style(const Style& style) noexcept
{
    m_style = style;
    m_frame->style(style.frame);
    m_scrollbar->style(style.scrollbar);
    resize_clip_container();
}

long ScrollPanel::scroll_position() const noexcept
{
    return m_scrollbar->scroll_position();
}

void ScrollPanel::resize_clip_container()
{
    // Size the content panel based on the frame margins and scrollbar size
    m_clip_container->layout(
        {{0.0f, static_cast<long>(m_style.frame.margins.left)},
         {0.0f, static_cast<long>(m_style.frame.margins.top)},
         {1.0f, -static_cast<long>(m_style.frame.margins.right + m_scrollbar_width)},
         {1.0f, -static_cast<long>(m_style.frame.margins.bottom)}});
}

void ScrollPanel::on_event(const Event& e)
{
    if (auto* mse = std::get_if<MouseScrollEvent>(&e)) {
        // The offset is positive when the wheel scrolls "up" (for end-user UX). In this case,
        // we want the scroll position to decrement.
        m_scrollbar->scroll_position(m_scrollbar->scroll_position() -
                                     static_cast<long>(mse->scroll_y) * SCROLL_MULTIPLIER);
    }
}

void ScrollPanel::on_layout() noexcept
{
    // After the new layout has been updated, update our scroll range and position in case
    // the content size changed.
    m_content_bounds          = m_clip_container->calculated_layout();
    const auto content_height = static_cast<long>(m_content->calculated_bounds().height);
    const auto view_height    = static_cast<long>(m_content_bounds.height);
    m_scrollbar->scroll_range(0, std::max(content_height - view_height, 0L));
}

void ScrollPanel::on_scroll()
{
    const auto scroll_pos = m_scrollbar->scroll_position();
    m_content->layout({{0.0f, 0}, {0.0f, -scroll_pos}, {0.0f, 0}, {0.0f, -scroll_pos}});
}

} // namespace khepri::ui
