#pragma once

#include "align.hpp"

#include <khepri/math/color_rgba.hpp>
#include <khepri/math/point.hpp>
#include <khepri/math/rect.hpp>
#include <khepri/math/size.hpp>

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <string>

namespace khepri::ui {

/**
 * UI Renderer.
 *
 * This is an interface for rendering UI elements. Pass an implementation of this interface to
 * #khepri::ui::Canvas so it can render its contents.
 *
 * The definition is purposely decoupled from any actual graphics renderers in line with the modular
 * architecture. As such, resources (fonts, textures) are identified by opaque IDs that must be
 * managed by the implementation of this interface.
 *
 * During rendering, the canvas breaks down the UI widgets into two types of elements: quads
 * (textured rectangles), and text (strings rendered with a font).
 */
class Renderer
{
public:
    /**
     * Identifier for a material.
     *
     * This is an opaque type used to refer to loaded materials. It is set on widgets when defining
     * the UI and passed back to the renderer during rendering.
     * This allows different widgets to be drawn with different effects, if so desired.
     */
    using MaterialId = std::size_t;

    static constexpr MaterialId INVALID_MATERIAL = static_cast<MaterialId>(-1);

    /**
     * Identifier for a texture.
     *
     * This is an opaque type used to refer to loaded textures. It is set on widgets when defining
     * the UI and passed back to the renderer during rendering.
     */
    using TextureId = std::size_t;

    static constexpr TextureId INVALID_TEXTURE = static_cast<TextureId>(-1);

    /**
     * Identifier for a font.
     *
     * This is an opaque type used to refer to loaded fonts. It is set on widgets when defining
     * the UI and passed back to the renderer during rendering.
     */
    using FontId = std::size_t;

    static constexpr FontId INVALID_FONT = static_cast<FontId>(-1);

    /**
     * A quad is the basis for rendering UIs.
     *
     * Every UI widget is fundamentally a collection of textured quads.
     */
    struct Quad
    {
        /// The area covered by the quad (in virtual pixels).
        Rect area{0, 0, 0, 0};

        /// The material for this quad.
        MaterialId material_id{INVALID_MATERIAL};

        /// The texture for this quad.
        TextureId texture_id{INVALID_TEXTURE};

        /// The area from the texture to use (in texels).
        Rect tex_rect{0, 0, 0, 0};
    };

    struct Text
    {
        /// The area that the text should be placed in (in virtual pixels).
        Rect area{0, 0, 0, 0};

        /// The material to use for rendering the text.
        MaterialId material_id{INVALID_MATERIAL};

        /// The font to use for rendering the text.
        FontId font_id{INVALID_FONT};

        /// The text string to render.
        std::u16string string;

        /// The text's horizontal alignment within the area.
        TextAlign text_align{TextAlign::left};
    };

    struct FilledRect
    {
        /// The area the rectangle covers (in virtual pixels). The bottom-right border is exclusive.
        Rect area{0, 0, 0, 0};

        /// The material to use for rendering the rectangle.
        MaterialId material_id{INVALID_MATERIAL};

        /// The color the rectangle will be filled with.
        ColorRGBA color{1, 1, 1, 1};
    };

    virtual ~Renderer() = default;

    /**
     * Begins rendering the UI.
     *
     * All rendering calls must be made between a call to #begin_render and #end_render.
     *
     * \param[in] canvas_size size of the canvas being rendered.
     *
     * \note it is up the implementation to decide when to do the actual rendering. Implementations
     *   can choose to render immediately when the various \a render_* methods are called, or only
     * when #end_render is called, or even some implementation-defined time.
     *
     * \throws InvalidOperationException if #begin_render was already called.
     */
    virtual void begin_render(const Size& canvas_size) = 0;

    /**
     * Ends rendering the UI.
     *
     * All rendering calls must be made between a call to #begin_render and #end_render.
     *
     * \throws InvalidOperationException if #end_render was already called.
     */
    virtual void end_render() = 0;

    /**
     * Renders filled rectangles.
     *
     * \param[in] rects rectangles to render.
     * \param[in] clip_rect the clipping area that the rendered content will be clipped to.
     */
    virtual void render_rects(gsl::span<const FilledRect> rects, const Rect& clip_rect) = 0;

    /**
     * Renders a collection of quads.
     *
     * \param[in] quads quads to render.
     * \param[in] clip_rect the clipping area that the rendered content will be clipped to.
     *
     * The coordinates in the quads are in virtual pixels relative to the canvas size specified at
     * #begin_render.
     *
     * \throws InvalidOperationException if #begin_render wasn't called.
     */
    virtual void render_quads(gsl::span<const Quad> quads, const Rect& clip_rect) = 0;

    /**
     * Renders a collection of texts.
     *
     * \param[in] texts texts to render.
     * \param[in] clip_rect the clipping area that the rendered content will be clipped to.
     *
     * The coordinates in the texts are in virtual pixels relative to the canvas size specified at
     * #begin_render.
     *
     * \throws InvalidOperationException if #begin_render wasn't called.
     */
    virtual void render_texts(gsl::span<const Text> texts, const Rect& clip_rect) = 0;
};

} // namespace khepri::ui
