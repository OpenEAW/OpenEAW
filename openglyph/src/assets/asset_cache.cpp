#include <khepri/log/log.hpp>
#include <khepri/renderer/io/shader.hpp>
#include <khepri/renderer/io/texture.hpp>

#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/renderer/io/material.hpp>
#include <openglyph/renderer/io/model.hpp>
#include <openglyph/renderer/io/render_pipeline.hpp>

#include <mutex>
#include <set>

namespace openglyph {
namespace {
constexpr khepri::log::Logger LOG("assets");

auto create_shader_loader(AssetLoader& asset_loader, khepri::renderer::Renderer& renderer)
{
    return [&](std::string_view name) -> std::unique_ptr<khepri::renderer::Shader> {
        const auto& shader_desc_loader =
            [&](const std::filesystem::path& path) -> std::optional<khepri::renderer::ShaderDesc> {
            if (auto stream = asset_loader.open_shader(path.string())) {
                return khepri::renderer::io::load_shader(*stream);
            }
            return {};
        };
        return renderer.create_shader(name, shader_desc_loader);
    };
}

auto create_texture_loader(AssetLoader& asset_loader, khepri::renderer::Renderer& renderer)
{
    return [&](std::string_view name) -> std::unique_ptr<khepri::renderer::Texture> {
        if (auto stream = asset_loader.open_texture(name)) {
            auto texture_desc = khepri::renderer::io::load_texture(*stream);
            return renderer.create_texture(texture_desc);
        }
        return {};
    };
}

auto create_render_model_loader(AssetLoader&                       asset_loader,
                                openglyph::renderer::ModelCreator& model_creator)
{
    return [&](std::string_view name) -> std::unique_ptr<openglyph::renderer::RenderModel> {
        if (auto stream = asset_loader.open_model(name)) {
            const auto model = openglyph::io::read_model(*stream);
            return model_creator.create_model(model);
        }
        return {};
    };
}

} // namespace

AssetCache::AssetCache(AssetLoader& asset_loader, khepri::renderer::Renderer& renderer)
    : m_shader_cache(create_shader_loader(asset_loader, renderer))
    , m_texture_cache(create_texture_loader(asset_loader, renderer))
    , m_render_pipelines(renderer)
    , m_materials(renderer, m_shader_cache.as_loader(), m_texture_cache.as_loader())
    , m_model_creator(
          renderer, [this](auto name) { return get_material(name); }, m_texture_cache.as_loader())
    , m_render_model_cache(create_render_model_loader(asset_loader, m_model_creator))
{
    if (auto stream = asset_loader.open_config("RenderPipelines")) {
        m_render_pipelines.register_render_pipelines(
            openglyph::renderer::io::load_render_pipelines(*stream));
    }

    if (auto stream = asset_loader.open_config("Materials")) {
        m_materials.register_materials(openglyph::renderer::io::load_materials(*stream));
    }
}

AssetCache::~AssetCache() = default;

const khepri::renderer::RenderPipeline* AssetCache::get_render_pipeline(std::string_view name)
{
    if (const auto* pipeline = m_render_pipelines.get(name)) {
        return pipeline;
    }

    // Log only once to avoid spamming
    static std::set<std::string, std::less<>> logged_names;
    static std::mutex                         log_mutex;
    {
        std::lock_guard lock(log_mutex);
        if (logged_names.find(name) == logged_names.end()) {
            logged_names.insert(std::string(name));
            LOG.error("cannot find render pipeline \"{}\"", name);
        }
    }

    return nullptr;
}

const khepri::renderer::Material* AssetCache::get_material(std::string_view name)
{
    if (const auto* material = m_materials.get(name)) {
        return material;
    }

    // Log only once to avoid spamming
    static std::set<std::string, std::less<>> logged_names;
    static std::mutex                         log_mutex;
    {
        std::lock_guard lock(log_mutex);
        if (logged_names.find(name) == logged_names.end()) {
            logged_names.insert(std::string(name));
            LOG.error("cannot find material \"{}\"", name);
        }
    }

    return nullptr;
}

const khepri::renderer::Texture* AssetCache::get_texture(std::string_view name)
{
    // Unfindable textures are logged from the AssetLoader
    return m_texture_cache.get(name);
}

const openglyph::renderer::RenderModel* AssetCache::get_render_model(std::string_view name)
{
    // Unfindable models are logged from the AssetLoader
    return m_render_model_cache.get(name);
}

} // namespace openglyph
