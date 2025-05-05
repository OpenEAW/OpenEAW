#pragma once

#include <khepri/math/vector2.hpp>

namespace khepri::renderer {

/**
 * \brief A camera-aligned rectangle
 *
 * A sprite is a rectangle that always faces the camera. It has a 2D position
 * and material information.
 *
 * \see #khepri::renderer::Renderer::render_sprites
 */
struct Sprite
{
    /// The top-left position of the sprite
    Vector2f position_top_left;

    /// The bottom-right position of the sprite
    Vector2f position_bottom_right;

    /// The top-left UV coordinates of the sprite
    Vector2f uv_top_left;

    /// The bottom-right UV coordinates of the sprite
    Vector2f uv_bottom_right;
};

} // namespace khepri::renderer
