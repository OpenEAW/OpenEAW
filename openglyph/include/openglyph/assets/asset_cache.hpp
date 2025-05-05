#pragma once

#include "asset_loader.hpp"

#include <khepri/renderer/renderer.hpp>
#include <khepri/utility/cache.hpp>
#include <openglyph/renderer/material_store.hpp>
#include <openglyph/renderer/model_creator.hpp>

namespace openglyph {

/**
 * @brief Cache of the various assets
 *
 * This class loads, instantiates and subsequently own requested assets.
 *
 * It hands out non-owning references to users of the class that are valid during the lifetime of
 * the object.
 */
class AssetCache final
{
public:
    AssetCache(AssetLoader& asset_loader, khepri::renderer::Renderer& renderer);

    AssetCache(const AssetCache&) = delete;
    AssetCache& operator=(const AssetCache&) = delete;
    ~AssetCache();

    khepri::renderer::Material* get_material(std::string_view name);

    khepri::renderer::Texture* get_texture(std::string_view name);

    openglyph::renderer::RenderModel* get_render_model(std::string_view name);

private:
    khepri::OwningCache<khepri::renderer::Shader>         m_shader_cache;
    khepri::OwningCache<khepri::renderer::Texture>        m_texture_cache;
    openglyph::renderer::MaterialStore                    m_materials;
    openglyph::renderer::ModelCreator                     m_model_creator;
    khepri::OwningCache<openglyph::renderer::RenderModel> m_render_model_cache;
};

} // namespace openglyph
