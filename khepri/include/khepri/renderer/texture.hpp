#pragma once

#include <khepri/math/size.hpp>

namespace khepri::renderer {

/**
 * \brief A texture
 *
 * A texture can be specified as material parameter and used in shaders to render meshes. Textures
 * are created by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_texture
 */
class Texture
{
public:
    /**
     * Constructs a texture.
     * \param[in] size the size of the texture (texels)
     */
    explicit Texture(const Size& size) : m_size(size) {}
    virtual ~Texture() = default;

    /// Retrieves the size of the texture
    [[nodiscard]] const Size& size() const noexcept
    {
        return m_size;
    }

protected:
    Texture(const Texture&)            = default;
    Texture(Texture&&)                 = default;
    Texture& operator=(const Texture&) = default;
    Texture& operator=(Texture&&)      = default;

private:
    Size m_size;
};

} // namespace khepri::renderer
