#pragma once

#include "renderer.hpp"
#include "widget.hpp"

#include <khepri/math/rect.hpp>

namespace khepri::ui {

/**
 * A static frame with optional borders
 *
 * A frame is a widget that has a collection of textures for rendering.
 * A frame is defined following the 9-slice principle: imagine two horizontal and two vertical
 * lines intersecting the frame. The frame is divided into 9 rectangular segments, each with their
 * own texture: the middle, top, bottom, left, right, top-left, bottom-left, top-right and
 * bottom-right. Textures are specified per segment. Most textures are always placed with their
 * original aspect ratio, aligned with the outside of the frame (so they may overlap the 'middle' of
 * the frame if they are larger than the margin). The textures for four side segments are stretched
 * along the side's direction.
 *
 * The middle texture is always stretched in both directions.
 *
 * Additionally, each of the four "side" segments has up to three textures: a main texture and two
 * transitions textures. The transition textures replace the first and last parts of the tiling in
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
        Renderer::TextureId texture_id{Renderer::INVALID_TEXTURE};

        /// The area from the texture to use (in texels)
        Rect tex_rect{0, 0, 0, 0};
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

    struct Style
    {
        /// The margins of the frame
        Margins margins;

        /// The frame's material
        Renderer::MaterialId material{Renderer::INVALID_MATERIAL};

        /// The background covers the entire frame, including the borders (stretched to fill)
        TextureSource background_slice;

        /// The middle fills out the frame between the borders (tiled to fill)
        TextureSource middle_slice;

        /// Top-left corner (stretched to fill)
        TextureSource top_left_slice;

        /// Top-right corner (stretched to fill)
        TextureSource top_right_slice;

        /// Bottom-left corner (stretched to fill)
        TextureSource bottom_left_slice;

        /// Bottom-right corner (stretched to fill)
        TextureSource bottom_right_slice;

        // Left side transition slice above the middle (scaled to fit width)
        TextureSource left_above_slice;

        // Left side transition slice below the middle (scaled to fit width)
        TextureSource left_below_slice;

        // Left side middle slice (scaled to fill width, tiled to fill height)
        TextureSource left_slice;

        // Top side transition slice left of middle (scaled to fit height)
        TextureSource top_first_slice;

        // Top side transition slice right of middle (scaled to fit height)
        TextureSource top_last_slice;

        // Top side middle slice (scaled to fill height, tiled to fill width)
        TextureSource top_slice;

        // Right side transition slice above the middle (scaled to fit width)
        TextureSource right_above_slice;

        // Right side transition slice above the middle (scaled to fit width)
        TextureSource right_below_slice;

        // Right side middle slice (scaled to fill width, tiled to fill height)
        TextureSource right_slice;

        // Bottom side transition slice left of middle (scaled to fit height)
        TextureSource bottom_first_slice;

        // Bottom side transition slice right of middle (scaled to fit height)
        TextureSource bottom_last_slice;

        // Bottom side middle slice (scaled to fill height, tiled to fill width)
        TextureSource bottom_slice;
    };

    using Widget::Widget;

    /**
     * Sets the frame's style
     */
    void style(const Style style) noexcept;

    /**
     * Sets the margins of the frame.
     * These determine the lines of that create the 9 slice.
     */
    void margins(const Margins& margins) noexcept;

    /**
     * Sets the texture for one of the frame's slices.
     */
    void slice_texture(FrameSlice slice, const TextureSource& source) noexcept;

    /// \see #khepri::ui::Widget::render
    void render(Renderer& renderer, const Rect& clip_rect) noexcept override;

    using Widget::add;
    using Widget::remove;

protected:
    void on_layout() noexcept override;

private:
    // Returns the quad index of the specified slice
    unsigned int quad_index(FrameSlice slice) const noexcept;
    bool         slice_active(FrameSlice slice) const noexcept;
    unsigned int activate_slice(FrameSlice slice) noexcept;
    void         deactivate_slice(FrameSlice slice) noexcept;
    void         calculate_quads() noexcept;

    Margins              m_margins;
    Renderer::MaterialId m_material_id;

    // Bitmask of active slices.
    // Cheaper to store than duplicating the texture info per slice
    unsigned int m_slices{0};

    // Calculated quads. At most one per frame_slice.
    // Positions are relative to widget's top-left position.
    std::array<Renderer::Quad, NUM_SLICES> m_quads;

    // Number of valid calculated quads
    std::size_t m_quad_count{0};
};

} // namespace khepri::ui
