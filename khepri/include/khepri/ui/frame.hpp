#pragma once

#include "widget.hpp"

#include <khepri/math/bits.hpp>
#include <khepri/math/rect.hpp>

#include <algorithm>

namespace khepri::ui {

/**
 * A static frame with optional borders
 *
 * A frame is a widget that has a collection of textures for rendering.
 * A frame is defined following the 9-slice principle: imagine two horizontal and two vertical
 * lines interecting the frame. The frame is divided into 9 rectangular segments, each with their
 * own texture: the middle, top, bottom, left, right, top-left, bottom-left, top-right and
 * bottom-right. Textures are specified per segment. The corner textures are stretched into the
 * segment size. The textures for the side segments are tiled along the side's direction and
 * stretched along the orthogonal direction.
 *
 * For instance, the "top" and "bottom" textures are stretched in the Y direction and tiled in X
 * direction. Vica versa for the "left" and "right" textures.
 *
 * The middle texture is always stretched.
 *
 * Additionally, each of the four "side" segments has up to three textures: a main texture and two
 * transitions textures. The transition textures replace the initial and last parts of the tiling in
 * that border.
 */
class Frame : public Widget
{
public:
    /// Describes the margins of the frame, in pixels
    struct Margins
    {
        /// Left margin
        unsigned long left{0};

        /// Top margin
        unsigned long top{0};

        /// Right margin
        unsigned long right{0};

        /// Bottom margin
        unsigned long bottom{0};
    };

    /// Describes a (subregion of a) texture
    struct TextureSource
    {
        /// The texture
        Renderer::TextureId texture{Renderer::INVALID_TEXTURE};

        /// The area from the texture to use (in texels)
        Rect tex_rect;
    };

    /// Identifies a texture slice of the frame
    enum class FrameSlice
    {
        /// The background covers the entire frame, including the borders (stretched to fill)
        background = 0,

        /// The middle fills out the frame between the borders (tiled to fill)
        middle,

        /// Top-left corner (stretched to fill)
        top_left,

        /// Top-right corner (stretched to fill)
        top_right,

        /// Bottom-left corner (stretched to fill)
        bottom_left,

        /// Bottom-right corner (stretched to fill)
        bottom_right,

        // Left side transition slice above the middle (scaled to fit width)
        left_above,

        // Left side transition slice below the middle (scaled to fit width)
        left_below,

        // Left side middle slice (scaled to fill width, tiled to fill height)
        left,

        // Top side transition slice left of middle (scaled to fit height)
        top_first,

        // Top side transition slice right of middle (scaled to fit height)
        top_last,

        // Top side middle slice (scaled to fill height, tiled to fill width)
        top,

        // Right side transition slice above the middle (scaled to fit width)
        right_above,

        // Right side transition slice above the middle (scaled to fit width)
        right_below,

        // Right side middle slice (scaled to fill width, tiled to fill height)
        right,

        // Bottom side transition slice left of middle (scaled to fit height)
        bottom_first,

        // Bottom side transition slice right of middle (scaled to fit height)
        bottom_last,

        // Bottom side middle slice (scaled to fill height, tiled to fill width)
        bottom,
    };

    /// The number of slices in the frame
    static constexpr int NUM_SLICES = 18;

    /**
     * An array of texture sources, for easily specifying all slice textures.
     *
     * If a slice's texture is nullptr, that slice is not set.
     */
    using TextureSources = std::array<TextureSource, NUM_SLICES>;

    struct Style
    {
        /**
         * The margins of the frame
         */
        Margins margins;

        /**
         * The texture for the frame's slices
         */
        TextureSources slice_textures;
    };

    /**
     * Constructs the frame
     * \param layout the layout for this frame
     */
    explicit Frame(const Layout& layout) noexcept : Widget(layout) {}

    /**
     * Sets the frame's style
     */
    void style(const Style style) noexcept
    {
        margins(style.margins);
        slice_textures(style.slice_textures);
    }

    /**
     * Sets the margins of the frame.
     * These determine the lines of that create the 9 slice.
     */
    void margins(const Margins& margins) noexcept
    {
        m_margins = margins;
        calculate_quads();
    }

    /**
     * Sets the textures for all of the frame's slices.
     */
    void slice_textures(const TextureSources& sources) noexcept
    {
        for (int slice_index = 0; slice_index < NUM_SLICES; ++slice_index) {
            slice_texture(static_cast<FrameSlice>(slice_index), sources[slice_index]);
        }
    }

    /**
     * Sets the texture for one of the frame's slices.
     */
    void slice_texture(FrameSlice slice, const TextureSource& source) noexcept
    {
        if (source.texture != nullptr) {
            // Slice should be active
            unsigned int qi;
            if (!slice_active(slice)) {
                qi = activate_slice(slice);
            } else {
                qi = quad_index(slice);
            }
            m_quads[qi].texture  = source.texture;
            m_quads[qi].tex_rect = source.tex_rect;
            calculate_quads();
        } else {
            // Slice should be inactive
            if (slice_active(slice)) {
                deactivate_slice(slice);
            }
            calculate_quads();
        }
    }

    /// \see #khepri::ui::Widget::render
    gsl::span<const Renderer::Quad> render(const Rect& clip_rect) noexcept override
    {
        return {m_quads.data(), m_quads.data() + m_quad_count};
    }

    using Widget::add;
    using Widget::remove;

protected:
    void on_layout() noexcept override
    {
        calculate_quads();
    }

private:
    // Returns the quad index of the specified slice
    unsigned int quad_index(FrameSlice slice) const noexcept
    {
        // The number of bits set before the slice_index-th bit indicates the offset into m_quads of
        // the slice
        const auto slice_index = static_cast<unsigned int>(slice);
        return bitcount(m_slices & ((1u << slice_index) - 1));
    }

    bool slice_active(FrameSlice slice) const noexcept
    {
        const auto slice_index = static_cast<unsigned int>(slice);
        assert(slice_index < m_quads.size());
        return (m_slices & (1u << slice_index)) != 0;
    }

    unsigned int activate_slice(FrameSlice slice) noexcept
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

    void deactivate_slice(FrameSlice slice) noexcept
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

    void calculate_quads() noexcept
    {
        const auto& widget_rect = calculated_layout();
        const Size  widget_size = {widget_rect.width, widget_rect.height};

        const auto left_margin   = std::min(m_margins.left, widget_size.width);
        const auto top_margin    = std::min(m_margins.top, widget_size.height);
        const auto right_margin  = std::min(m_margins.right, widget_size.width);
        const auto bottom_margin = std::min(m_margins.bottom, widget_size.height);

        Quad* q = m_quads.data();

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
            q->area =
                make_rect(left_margin, top_margin, widget_size.width - right_margin - left_margin,
                          widget_size.height - bottom_margin - top_margin);
            q++;
        }

        // Corners
        if (slice_active(FrameSlice::top_left)) {
            q->area = make_rect(0, 0, left_margin, top_margin);
            q++;
        }

        if (slice_active(FrameSlice::top_right)) {
            q->area = make_rect(widget_size.width - right_margin, 0, right_margin, top_margin);
            q++;
        }

        if (slice_active(FrameSlice::bottom_left)) {
            q->area = make_rect(0, widget_size.height - bottom_margin, left_margin, bottom_margin);
            q++;
        }

        if (slice_active(FrameSlice::bottom_right)) {
            q->area = make_rect(widget_size.width - right_margin,
                                widget_size.height - bottom_margin, right_margin, bottom_margin);
            q++;
        }

        // Left border
        auto left_middle_top = top_margin;
        if (slice_active(FrameSlice::left_above)) {
            q->area = make_rect(0, left_middle_top, left_margin, q->tex_rect.height);
            left_middle_top += q->tex_rect.height;
            q++;
        }

        auto left_middle_bottom = widget_size.height - bottom_margin;
        if (slice_active(FrameSlice::left_below)) {
            left_middle_bottom -= q->tex_rect.height;
            q->area = make_rect(0, left_middle_bottom, left_margin, q->tex_rect.height);
            q++;
        }

        if (slice_active(FrameSlice::left)) {
            const auto height =
                (left_middle_top < left_middle_bottom) ? left_middle_bottom - left_middle_top : 0;
            q->area = make_rect(0, left_middle_top, left_margin, height);
            q++;
        }

        // Top border
        auto top_middle_left = left_margin;
        if (slice_active(FrameSlice::top_first)) {
            q->area = make_rect(top_middle_left, 0, q->tex_rect.width, top_margin);
            top_middle_left += q->tex_rect.width;
            q++;
        }

        auto top_middle_right = widget_size.width - right_margin;
        if (slice_active(FrameSlice::top_last)) {
            top_middle_right -= q->tex_rect.width;
            q->area = make_rect(top_middle_right, 0, q->tex_rect.width, top_margin);
            q++;
        }

        if (slice_active(FrameSlice::top)) {
            const auto width =
                (top_middle_left < top_middle_right) ? top_middle_right - top_middle_left : 0;
            q->area = make_rect(top_middle_left, 0, width, top_margin);
            q++;
        }

        // Right border
        auto right_middle_top = top_margin;
        if (slice_active(FrameSlice::right_above)) {
            q->area = make_rect(widget_size.width - right_margin, right_middle_top, right_margin,
                                q->tex_rect.height);
            right_middle_top += q->tex_rect.height;
            q++;
        }

        auto right_middle_bottom = widget_size.height - bottom_margin;
        if (slice_active(FrameSlice::right_below)) {
            right_middle_bottom -= q->tex_rect.height;
            q->area = make_rect(widget_size.width - right_margin, right_middle_bottom, right_margin,
                                q->tex_rect.height);
            q++;
        }

        if (slice_active(FrameSlice::right)) {
            const auto height = (right_middle_top < right_middle_bottom)
                                    ? right_middle_bottom - right_middle_top
                                    : 0;
            q->area =
                make_rect(widget_size.width - right_margin, right_middle_top, right_margin, height);
            q++;
        }

        // Bottom border
        auto bottom_middle_left = left_margin;
        if (slice_active(FrameSlice::bottom_first)) {
            q->area = make_rect(bottom_middle_left, widget_size.height - bottom_margin,
                                q->tex_rect.width, bottom_margin);
            bottom_middle_left += q->tex_rect.width;
            q++;
        }

        auto bottom_middle_right = widget_size.width - right_margin;
        if (slice_active(FrameSlice::bottom_last)) {
            bottom_middle_right -= q->tex_rect.width;
            q->area = make_rect(bottom_middle_right, widget_size.height - bottom_margin,
                                q->tex_rect.width, bottom_margin);
            q++;
        }

        if (slice_active(FrameSlice::bottom)) {
            const auto width = (bottom_middle_left < bottom_middle_right)
                                   ? bottom_middle_right - bottom_middle_left
                                   : 0;
            q->area = make_rect(bottom_middle_left, widget_size.height - bottom_margin, width,
                                bottom_margin);
            q++;
        }
    }

    Margins m_margins;

    // Bitmask of active slices.
    // Cheaper to store than duplicating the texture info per slice
    unsigned int m_slices{0};

    // Calculated quads. At most one per frame_slice.
    // Positions are relative to widget's top-left position.
    std::array<Quad, NUM_SLICES> m_quads;

    // Number of valid calculated quads
    std::size_t m_quad_count{0};
};

} // namespace khepri::ui
