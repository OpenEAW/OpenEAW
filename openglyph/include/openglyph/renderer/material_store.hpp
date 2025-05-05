#pragma once

#include "material_desc.hpp"

#include <gsl/gsl-lite.hpp>
#include <khepri/renderer/renderer.hpp>
#include <khepri/renderer/shader.hpp>
#include <khepri/renderer/texture.hpp>
#include <khepri/utility/string.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace openglyph::renderer {

class MaterialStore
{
public:
    template <typename T>
    using Loader = std::function<T*(std::string_view)>;

    MaterialStore(khepri::renderer::Renderer&       renderer,
                  Loader<khepri::renderer::Shader>  shader_loader,
                  Loader<khepri::renderer::Texture> texture_loader)
        : m_renderer(renderer)
        , m_shader_loader(std::move(shader_loader))
        , m_texture_loader(std::move(texture_loader))
    {}

    void register_materials(gsl::span<const MaterialDesc> material_descs);

    khepri::renderer::Material* get(std::string_view name) const noexcept;

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
