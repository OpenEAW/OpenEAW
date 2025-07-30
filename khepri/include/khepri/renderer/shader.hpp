#pragma once

namespace khepri::renderer {

/**
 * \brief A shader.
 *
 * A shader is a program that be executed by a renderer to render a mesh. Shaders are created by a
 * #khepri::renderer::Renderer.
 *
 * \see #khepri::renderer::Renderer::create_shader
 */
class Shader
{
public:
    Shader()          = default;
    virtual ~Shader() = default;

protected:
    Shader(const Shader&)            = default;
    Shader(Shader&&)                 = default;
    Shader& operator=(const Shader&) = default;
    Shader& operator=(Shader&&)      = default;
};

} // namespace khepri::renderer
