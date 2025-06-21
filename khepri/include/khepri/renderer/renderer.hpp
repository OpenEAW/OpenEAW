#pragma once

#include "camera.hpp"
#include "material.hpp"
#include "material_desc.hpp"
#include "mesh.hpp"
#include "mesh_desc.hpp"
#include "mesh_instance.hpp"
#include "shader.hpp"
#include "shader_desc.hpp"
#include "sprite.hpp"
#include "texture.hpp"
#include "texture_desc.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/size.hpp>
#include <khepri/utility/enum.hpp>

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Interface for renderers
 *
 * This interface provides a technology-independent interface to various renderers.
 */
class Renderer
{
public:
    /// Flags for #clear()
    enum ClearFlags : int
    {
        clear_rendertarget = 1,
        clear_depth        = 2,
        clear_stencil      = 4,
        clear_all          = clear_rendertarget | clear_depth | clear_stencil,
    };

    /**
     * Callback used to load shaders on demand.
     * \param the path of the shader.
     * \return the shaders's content or std::none if the shader cannot be found or read.
     */
    using ShaderLoader = std::function<std::optional<ShaderDesc>(const std::filesystem::path&)>;

    Renderer()          = default;
    virtual ~Renderer() = default;

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    /**
     * Returns the size of the rendering area
     */
    [[nodiscard]] virtual Size render_size() const noexcept = 0;

    /**
     * \brief Creates a shader by compiling a shader source file.
     *
     * A shader is a pair of vertex and pixel shaders loaded from the same file.
     * The vertex shader's entry point is called @c vs_main and the pixel shader's entry point is
     * called @c ps_main.
     *
     * \param path the path to the source file of the shader
     * \param loader the loader used to load shader files
     *
     * In case the shader source files contain includes of other files, @a loader is invoked
     * multiple times to load the needed files.
     */
    virtual std::unique_ptr<Shader> create_shader(const std::filesystem::path& path,
                                                  const ShaderLoader&          loader) = 0;

    /**
     * \brief Creates a material to be used when rendering meshes.
     *
     * After creating a material, specify it in a #khepri::renderer::MeshInstance to
     * render that mesh with the created material.
     *
     * \param material_desc the material to create.
     */
    virtual std::unique_ptr<Material> create_material(const MaterialDesc& material_desc) = 0;

    /**
     * \brief Creates a texture from a texture description.
     *
     * Textures can be referenced by meshes for binding to shader arguments during rendering.
     *
     * \param[in] texture_desc the texture data.
     *
     * \return the newly created texture.
     */
    virtual std::unique_ptr<Texture> create_texture(const TextureDesc& texture_desc) = 0;

    /**
     * \brief Creates a mesh from a mesh descrpition.
     *
     * The returned mesh can be specified in a #khepri::renderer::MeshInstance to be rendered.
     *
     * \param[in] mesh_desc the mesh description to create a mesh from.
     *
     * \return the created mesh.
     */
    virtual std::unique_ptr<Mesh> create_mesh(const MeshDesc& mesh_desc) = 0;

    /**
     * Clears the render target and/or depth/stencil buffer
     *
     * \param[in] flags one or more OR'd flags from #ClearFlags
     */
    virtual void clear(ClearFlags flags) = 0;

    /**
     * Presents all the rendered objects.
     */
    virtual void present() = 0;

    /**
     * Renders a collection of meshe instances.
     *
     * \param[in] meshes a collection of mesh instances to render.
     * \param[in] camera the camera to render them with.
     */
    virtual void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera) = 0;

    /**
     * Renders a collection of sprites in camera-space
     *
     * \param[in] sprites a collection of sprites to render.
     * \param[in] material the material to render the sprites with.
     * \param[in] params the material parameters to render the sprites with.
     */
    virtual void render_sprites(gsl::span<const Sprite> sprites, Material& material,
                                gsl::span<const Material::Param> params) = 0;
};

} // namespace khepri::renderer