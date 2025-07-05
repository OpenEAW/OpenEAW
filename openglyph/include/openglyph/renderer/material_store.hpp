#pragma once

#include "material_desc.hpp"

#include <khepri/renderer/renderer.hpp>
#include <khepri/renderer/shader.hpp>
#include <khepri/renderer/texture.hpp>
#include <khepri/utility/string.hpp>

#include <gsl/gsl-lite.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace openglyph::renderer {

class MaterialStore final
{
public:
    template <typename T>
    using Loader = std::function<T*(std::string_view)>;

    MaterialStore(khepri::renderer::Renderer&       renderer,
                  Loader<khepri::renderer::Shader>  shader_loader,
                  Loader<khepri::renderer::Texture> texture_loader);
    ~MaterialStore() = default;

    MaterialStore(const MaterialStore&)                = delete;
    MaterialStore(MaterialStore&&) noexcept            = delete;
    MaterialStore& operator=(const MaterialStore&)     = delete;
    MaterialStore& operator=(MaterialStore&&) noexcept = delete;

    void register_materials(gsl::span<const MaterialDesc> material_descs);

    [[nodiscard]] khepri::renderer::Material* get(std::string_view name) const noexcept;

    auto as_loader()
    {
        return [this](std::string_view id) { return this->get(id); };
    }

private:
    using MaterialMap = std::map<std::string, std::unique_ptr<khepri::renderer::Material>,
                                 khepri::CaseInsensitiveLess>;

    khepri::renderer::Renderer&       m_renderer;
    Loader<khepri::renderer::Shader>  m_shader_loader;
    Loader<khepri::renderer::Texture> m_texture_loader;

    MaterialMap m_materials;
};

} // namespace openglyph::renderer
