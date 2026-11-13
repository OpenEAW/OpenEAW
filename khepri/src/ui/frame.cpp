#include <khepri/math/bits.hpp>
#include <khepri/ui/frame.hpp>

#include <algorithm>

namespace khepri::ui {

void Frame::style(const Style style) noexcept
{
    m_material_id = style.material;
    margins(style.margins);
    slice_texture(FrameSlice::background, style.background_slice);
    slice_texture(FrameSlice::middle, style.middle_slice);
    slice_texture(FrameSlice::top_left, style.top_left_slice);
    slice_texture(FrameSlice::top_right, style.top_right_slice);
    slice_texture(FrameSlice::bottom_left, style.bottom_left_slice);
    slice_texture(FrameSlice::bottom_right, style.bottom_right_slice);
    slice_texture(FrameSlice::left_above, style.left_above_slice);
    slice_texture(FrameSlice::left_below, style.left_below_slice);
    slice_texture(FrameSlice::left, style.left_slice);
    slice_texture(FrameSlice::top_first, style.top_first_slice);
    slice_texture(FrameSlice::top_last, style.top_last_slice);
    slice_texture(FrameSlice::top, style.top_slice);
    slice_texture(FrameSlice::right_above, style.right_above_slice);
    slice_texture(FrameSlice::right_below, style.right_below_slice);
    slice_texture(FrameSlice::right, style.right_slice);
    slice_texture(FrameSlice::bottom_first, style.bottom_first_slice);
    slice_texture(FrameSlice::bottom_last, style.bottom_last_slice);
    slice_texture(FrameSlice::bottom, style.bottom_slice);
}

void Frame::margins(const Margins& margins) noexcept
{
    m_margins = margins;
    calculate_quads();
}

void Frame::slice_texture(FrameSlice slice, const TextureSource& source) noexcept
{
    if (source.texture_id != Renderer::INVALID_TEXTURE) {
        // Slice should be active
        unsigned int qi;
        if (!slice_active(slice)) {
            qi = activate_slice(slice);
        } else {
            qi = quad_index(slice);
        }
        m_quads[qi].material_id = m_material_id;
        m_quads[qi].texture_id  = source.texture_id;
        m_quads[qi].tex_rect    = source.tex_rect;
        calculate_quads();
    } else {
        // Slice should be inactive
        if (slice_active(slice)) {
            deactivate_slice(slice);
        }
        calculate_quads();
    }
}

void Frame::render(Renderer& renderer, const Rect& clip_rect) noexcept
{
    return renderer.render_quads({m_quads.data(), m_quad_count}, clip_rect);
}

void Frame::on_layout() noexcept
{
    calculate_quads();
}

unsigned int Frame::quad_index(FrameSlice slice) const noexcept
{
    // The number of bits set before the slice_index-th bit indicates the offset into m_quads of
    // the slice
    const auto slice_index = static_cast<unsigned int>(slice);
    return bitcount(m_slices & ((1u << slice_index) - 1));
}

bool Frame::slice_active(FrameSlice slice) const noexcept
{
    const auto slice_index = static_cast<unsigned int>(slice);
    assert(slice_index < m_quads.size());
    return (m_slices & (1u << slice_index)) != 0;
}

unsigned int Frame::activate_slice(FrameSlice slice) noexcept
{
    const auto slice_index = static_cast<unsigned int>(slice);
    assert(slice_index < m_quads.size());
    // Mark it active
    m_slices |= (1u << slice_index);

    const auto qi = quad_index(slice);
    assert(qi < m_quads.size());
    // Make room for the new slice's quad
    std::move_backward(m_quads.begin() + qi, m_quads.begin() + m_quad_count,
                       m_quads.begin() + m_quad_count + 1);
    m_quad_count++;
    return qi;
}

void Frame::deactivate_slice(FrameSlice slice) noexcept
{
    const auto slice_index = static_cast<unsigned int>(slice);
    assert(slice_index < m_quads.size());
    const auto qi = quad_index(slice);
    assert(qi < m_quads.size());

    // Remove the slice's quad
    std::move(m_quads.begin() + qi + 1, m_quads.begin() + m_quad_count, m_quads.begin() + qi);
    m_quad_count--;

    // Mark it inactive
    m_slices &= ~(1u << slice_index);
}

void Frame::calculate_quads() noexcept
{
    const auto& widget_rect = calculated_layout();
    const Size  widget_size = {widget_rect.width, widget_rect.height};

    const auto left_margin   = std::min(m_margins.left, widget_size.width);
    const auto top_margin    = std::min(m_margins.top, widget_size.height);
    const auto right_margin  = std::min(m_margins.right, widget_size.width);
    const auto bottom_margin = std::min(m_margins.bottom, widget_size.height);

    auto* q = m_quads.data();

    const auto& make_rect = [&](auto x, auto y, auto width, auto height) {
        return Rect{widget_rect.x + static_cast<long>(x), widget_rect.y + static_cast<long>(y),
                    static_cast<unsigned long>(width), static_cast<unsigned long>(height)};
    };

    // Background
    if (slice_active(FrameSlice::background)) {
        q->area = widget_rect;
        q++;
    }

    // Middle
    if (slice_active(FrameSlice::middle)) {
        q->area = make_rect(left_margin, top_margin, widget_size.width - right_margin - left_margin,
                            widget_size.height - bottom_margin - top_margin);
        q++;
    }

    long left_top     = 0;                  // Offset for the top of the left border slices
    long top_left     = 0;                  // Offset for the left of the top border slices
    long right_top    = 0;                  // Offset for the top of the right border slices
    long top_right    = widget_size.width;  // Offset for the right of the top border slices
    long left_bottom  = widget_size.height; // Offset for the bottom of the left border slices
    long bottom_left  = 0;                  // Offset for the left of the bottom border slices
    long right_bottom = widget_size.height; // Offset for the bottom of the right border slices
    long bottom_right = widget_size.width;  // Offset for the right of the bottom border slices

    // Corners
    if (slice_active(FrameSlice::top_left)) {
        q->area = make_rect(0, 0, q->tex_rect.width, q->tex_rect.height);
        top_left += q->tex_rect.width;
        left_top += q->tex_rect.height;
        q++;
    }

    if (slice_active(FrameSlice::top_right)) {
        q->area = make_rect(widget_size.width - q->tex_rect.width, 0, q->tex_rect.width,
                            q->tex_rect.height);
        top_right -= q->tex_rect.width;
        right_top += q->tex_rect.height;
        q++;
    }

    if (slice_active(FrameSlice::bottom_left)) {
        q->area = make_rect(0, widget_size.height - q->tex_rect.height, q->tex_rect.width,
                            q->tex_rect.height);
        bottom_left += q->tex_rect.width;
        left_bottom -= q->tex_rect.height;
        q++;
    }

    if (slice_active(FrameSlice::bottom_right)) {
        q->area = make_rect(widget_size.width - q->tex_rect.width,
                            widget_size.height - q->tex_rect.height, q->tex_rect.width,
                            q->tex_rect.height);
        bottom_right -= q->tex_rect.width;
        right_bottom -= q->tex_rect.height;
        q++;
    }

    // Left border
    if (slice_active(FrameSlice::left_above)) {
        q->area = make_rect(0, left_top, q->tex_rect.width, q->tex_rect.height);
        left_top += q->tex_rect.height;
        q++;
    }

    auto left_middle_bottom = widget_size.height - bottom_margin;
    if (slice_active(FrameSlice::left_below)) {
        left_bottom -= q->tex_rect.height;
        q->area = make_rect(0, left_bottom, q->tex_rect.width, q->tex_rect.height);
        q++;
    }

    if (slice_active(FrameSlice::left)) {
        q->area = make_rect(0, left_top, q->tex_rect.width, left_bottom - left_top);
        q++;
    }

    // Top border
    auto top_middle_left = left_margin;
    if (slice_active(FrameSlice::top_first)) {
        q->area = make_rect(top_left, 0, q->tex_rect.width, q->tex_rect.height);
        top_left += q->tex_rect.width;
        q++;
    }

    if (slice_active(FrameSlice::top_last)) {
        top_right -= q->tex_rect.width;
        q->area = make_rect(top_right, 0, q->tex_rect.width, q->tex_rect.height);
        q++;
    }

    if (slice_active(FrameSlice::top)) {
        q->area = make_rect(top_left, 0, top_right - top_left, q->tex_rect.height);
        q++;
    }

    // Right border
    if (slice_active(FrameSlice::right_above)) {
        q->area = make_rect(widget_size.width - q->tex_rect.width, right_top, q->tex_rect.width,
                            q->tex_rect.height);
        right_top += q->tex_rect.height;
        q++;
    }

    if (slice_active(FrameSlice::right_below)) {
        right_bottom -= q->tex_rect.height;
        q->area = make_rect(widget_size.width - q->tex_rect.width, right_bottom, q->tex_rect.width,
                            q->tex_rect.height);
        q++;
    }

    if (slice_active(FrameSlice::right)) {
        q->area = make_rect(widget_size.width - right_margin, right_top, q->tex_rect.width,
                            right_bottom - right_top);
        q++;
    }

    // Bottom border
    if (slice_active(FrameSlice::bottom_first)) {
        q->area = make_rect(bottom_left, widget_size.height - q->tex_rect.height, q->tex_rect.width,
                            q->tex_rect.height);
        bottom_left += q->tex_rect.width;
        q++;
    }

    if (slice_active(FrameSlice::bottom_last)) {
        bottom_right -= q->tex_rect.width;
        q->area = make_rect(bottom_right, widget_size.height - q->tex_rect.height,
                            q->tex_rect.width, q->tex_rect.height);
        q++;
    }

    if (slice_active(FrameSlice::bottom)) {
        q->area = make_rect(bottom_left, widget_size.height - q->tex_rect.height,
                            bottom_right - bottom_left, q->tex_rect.height);
        q++;
    }
}

} // namespace khepri::ui
