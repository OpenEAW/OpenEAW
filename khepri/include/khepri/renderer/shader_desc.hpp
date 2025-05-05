#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Description of a shader
 *
 * A shader is a program that be executed by a renderer to render a mesh. Shaders are created and
 * destroyed from a description by a #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_shader
 */
class ShaderDesc
{
public:
    /**
     * Constructs a shader description.
     *
     * \param data the shader source code
     */
    explicit ShaderDesc(std::vector<std::uint8_t> data) : m_data(std::move(data)) {}

    /**
     * Returns the shader's source data
     */
    [[nodiscard]] const auto& data() const noexcept
    {
        return m_data;
    }

private:
    std::vector<std::uint8_t> m_data;
};

} // namespace khepri::renderer
