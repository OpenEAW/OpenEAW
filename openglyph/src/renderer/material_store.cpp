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

void MaterialStore::register_materials(
    gsl::span<const openglyph::renderer::MaterialDesc> material_descs)
{
    for (const auto& desc : material_descs) {
        khepri::renderer::MaterialDesc info;
        info.cull_mode        = khepri::renderer::MaterialDesc::CullMode::front;
        info.alpha_blend_mode = desc.alpha_blend_mode;
        info.depth_buffer     = desc.depth_buffer;
        info.shader           = m_shader_loader(desc.shader);
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
