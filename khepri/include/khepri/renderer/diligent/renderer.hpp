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
     *
     * \throws khepri::ArgumentError if \a window does not contain the expected type
     * \throws khepri::renderer::Error if the renderer could not be created
     *
     * The expected type in \a window depends on the target platform:
     * - Windows: a HWND is expected.
     * - Linux:   a std::tuple<void*, std::uint32_t> is expected where the first element is
     *            the X11 display pointer and the second element is the X11 window ID.
     */
    Renderer(const std::any& window);
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

    /// \see #khepri::renderer::Renderer::clear
    void clear(ClearFlags flags) override;

    /// \see #khepri::renderer::Renderer::present
    void present() override;

    /// \see #khepri::renderer::Renderer::render_meshes
    void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera) override;

    /// \see #khepri::renderer::Renderer::render_sprites
    void render_sprites(gsl::span<const Sprite> sprites, Material& material,
                        gsl::span<const Material::Param> params) override;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace khepri::renderer::diligent
