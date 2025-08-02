#pragma once

#include <khepri/renderer/renderer.hpp>

#include <any>
#include <memory>

namespace khepri::renderer::diligent {

/**
 * \brief Diligent-based renderer
 *
 * This renderer uses the Diligent Graphics Engine to render scenes to a surface.
 */
class Renderer : public khepri::renderer::Renderer
{
public:
    /**
     * Constructs the Diligent-based renderer.
     *
     * \param[in] window the native window to create the renderer in
     * \param[in] color_space the color space for the output buffer.
     *
     * \throws khepri::ArgumentError if \a window does not contain the expected type
     * \throws khepri::renderer::Error if the renderer could not be created
     *
     * The expected type in \a window depends on the target platform:
     * - Windows: a HWND is expected.
     * - Linux:   a std::tuple<void*, std::uint32_t> is expected where the first element is
     *            the X11 display pointer and the second element is the X11 window ID.
     *
     * The \a color_space argument matters to determine if gamma conversion is to be performed on
     * the rendered pixels. If the color space is srgb, gamma conversion will be performed. In
     * linear mode, the shader logic has to make sure the pixels are suitably converted for display.
     */
    Renderer(const std::any& window, ColorSpace color_space);
    ~Renderer() override;

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    /**
     * Set the render size for this renderer.
     */
    void render_size(const Size& size);

    /// \see #khepri::renderer::Renderer::render_size
    [[nodiscard]] Size render_size() const noexcept override;

    /// \see #khepri::renderer::Renderer::create_shader
    std::unique_ptr<Shader> create_shader(const std::filesystem::path& path,
                                          const ShaderLoader&          loader) override;

    /// \see #khepri::renderer::Renderer::create_material
    std::unique_ptr<Material> create_material(const MaterialDesc& material_desc) override;

    /// \see #khepri::renderer::Renderer::create_texture;
    std::unique_ptr<Texture> create_texture(const TextureDesc& texture_desc) override;

    /// \see #khepri::renderer::Renderer::create_mesh
    std::unique_ptr<Mesh> create_mesh(const MeshDesc& mesh_desc) override;

    /// \see #khepri::renderer::Renderer::create_render_pipeline
    std::unique_ptr<RenderPipeline>
    create_render_pipeline(const RenderPipelineDesc& render_pipeline_desc) override;

    /// \see #khepri::renderer::Renderer::set_dynamic_lights
    void set_dynamic_lights(const DynamicLightDesc& light_desc) override;

    /// \see #khepri::renderer::Renderer::clear
    void clear(ClearFlags flags) override;

    /// \see #khepri::renderer::Renderer::present
    void present() override;

    /// \see #khepri::renderer::Renderer::render_meshes
    void render_meshes(const RenderPipeline& render_pipeline, gsl::span<const MeshInstance> meshes,
                       const Camera& camera) override;

    /// \see #khepri::renderer::Renderer::render_sprites
    void render_sprites(const RenderPipeline& render_pipeline, gsl::span<const Sprite> sprites,
                        const Material& material, gsl::span<const Material::Param> params) override;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace khepri::renderer::diligent
