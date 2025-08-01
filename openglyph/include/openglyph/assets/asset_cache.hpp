#pragma once

#include "asset_loader.hpp"

#include <khepri/renderer/renderer.hpp>
#include <khepri/utility/cache.hpp>

#include <openglyph/renderer/material_store.hpp>
#include <openglyph/renderer/model_creator.hpp>
#include <openglyph/renderer/render_pipeline_store.hpp>

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

    AssetCache(const AssetCache&)            = delete;
    AssetCache(AssetCache&&)                 = delete;
    AssetCache& operator=(const AssetCache&) = delete;
    AssetCache& operator=(AssetCache&&)      = delete;
    ~AssetCache();

    const khepri::renderer::RenderPipeline* get_render_pipeline(std::string_view name);

    const khepri::renderer::Material* get_material(std::string_view name);

    const khepri::renderer::Texture* get_texture(std::string_view name);

    const openglyph::renderer::RenderModel* get_render_model(std::string_view name);

private:
    khepri::OwningCache<const khepri::renderer::Shader>         m_shader_cache;
    khepri::OwningCache<const khepri::renderer::Texture>        m_texture_cache;
    openglyph::renderer::RenderPipelineStore                    m_render_pipelines;
    openglyph::renderer::MaterialStore                          m_materials;
    openglyph::renderer::ModelCreator                           m_model_creator;
    khepri::OwningCache<const openglyph::renderer::RenderModel> m_render_model_cache;
};

} // namespace openglyph
