#include <openglyph/renderer/material_store.hpp>

namespace openglyph::renderer {
namespace {
// helper type for a variant visitor
template <class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
} // namespace

MaterialStore::MaterialStore(khepri::renderer::Renderer&             renderer,
                             Loader<const khepri::renderer::Shader>  shader_loader,
                             Loader<const khepri::renderer::Texture> texture_loader)
    : m_renderer(renderer)
    , m_shader_loader(std::move(shader_loader))
    , m_texture_loader(std::move(texture_loader))
{
}

void MaterialStore::register_materials(
    gsl::span<const openglyph::renderer::MaterialDesc> material_descs)
{
    for (const auto& desc : material_descs) {
        khepri::renderer::MaterialDesc info;
        info.type                      = desc.type;
        info.shader                    = m_shader_loader(desc.shader);
        info.num_directional_lights    = desc.num_directional_lights;
        info.num_point_lights          = desc.num_point_lights;
        info.graphics_pipeline_options = desc.graphics_pipeline_options;
        for (const auto& property : desc.properties) {
            khepri::renderer::MaterialDesc::Property prop;
            prop.name = property.name;
            std::visit(Overloaded{[&](const std::string& str) {
                                      prop.default_value = m_texture_loader(str);
                                  },
                                  [&](const auto& val) { prop.default_value = val; }},
                       property.default_value);
            info.properties.push_back(std::move(prop));
        }

        m_materials.emplace(desc.name, m_renderer.create_material(info));
    }
}

khepri::renderer::Material* MaterialStore::get(std::string_view name) const noexcept
{
    const auto it = m_materials.find(name);
    return (it != m_materials.end()) ? it->second.get() : nullptr;
}

} // namespace openglyph::renderer
