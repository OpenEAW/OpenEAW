#pragma once

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
        Rect area;

        /// The texture for this quad.
        TextureId texture_id;

        /// The area from the texture to use (in texels).
        Rect tex_rect;

        /// Clipping rectangle for this quad (in virtual pixels).
        Rect clip_rect;
    };

    struct Text
    {
        /// The start position of the baseline of the text (in virtual pixels).
        Pointi position;

        /// The font to use for rendering the text.
        FontId font_id;

        /// The text string to render.
        std::u16string string;

        /// Clipping rectangle for the text area (in virtual pixels).
        Rect clip_rect;
    };

    virtual ~Renderer() = default;

    /**
     * Begins rendering the UI.
     *
     * All rendering calls must be made between a call to #begin_render and #end_render.
     */
    virtual void begin_render() = 0;

    /**
     * Ends rendering the UI.
     *
     * All rendering calls must be made between a call to #begin_render and #end_render.
     */
    virtual void end_render() = 0;

    /**
     * Renders a collection of quads.
     *
     * \param[in] quads quads to render.
     * \param[in] canvas_size size of the canvas being rendered.
     *
     * The coordinates in the quads are in virtual pixels relative to the canvas size.
     */
    virtual void render_quads(gsl::span<const Quad> quads, const Size& canvas_size) = 0;

    /**
     * Renders a collection of texts.
     *
     * \param[in] texts texts to render.
     * \param[in] canvas_size size of the canvas being rendered.
     *
     * The coordinates in the texts are in virtual pixels relative to the canvas size.
     */
    virtual void render_texts(gsl::span<const Text> texts, const Size& canvas_size) = 0;
};

} // namespace khepri::ui
