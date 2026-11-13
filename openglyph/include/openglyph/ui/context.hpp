#pragma once

#include <khepri/adapters/ui_renderer.hpp>
#include <khepri/ui/canvas.hpp>
#include <khepri/ui/frame.hpp>
#include <khepri/utility/string.hpp>

#include <openglyph/assets/asset_cache.hpp>
#include <openglyph/assets/mega_texture.hpp>
#include <openglyph/assets/string_database.hpp>
#include <openglyph/ui/style_store.hpp>

#include <map>
#include <unordered_map>

namespace openglyph::ui {

/**
 * UI context object used by dialog creation functions to obtain resources and manipulate the UI.
 */
class Context
{
public:
    explicit Context(khepri::ui::Canvas& canvas, khepri::UIRenderer& ui_renderer,
                     openglyph::AssetLoader& asset_loader, openglyph::AssetCache& asset_cache,
                     const openglyph::StringDatabase& string_database,
                     const StyleStore&                style_store);

    Context(const Context&)            = delete;
    Context(Context&&)                 = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&)      = delete;

    [[nodiscard]] khepri::ui::Canvas& canvas() noexcept
    {
        return m_canvas;
    }

    [[nodiscard]] khepri::UIRenderer& ui_renderer() noexcept
    {
        return m_ui_renderer;
    }

    [[nodiscard]] const openglyph::ui::StyleStore& style_store() const noexcept
    {
        return m_style_store;
    }

    [[nodiscard]] const openglyph::StringDatabase& string_database() const noexcept
    {
        return m_string_database;
    }

    [[nodiscard]] const khepri::ui::Frame::TextureSource&
    get_texture(std::string_view ui_texture_name) const;

    [[nodiscard]] khepri::ui::Renderer::FontId get_font(const StyleStore::FontStyle& style) const;

    [[nodiscard]] khepri::ui::Renderer::MaterialId sprite_material_id() const noexcept
    {
        return m_sprite_material_id;
    }

    [[nodiscard]] khepri::ui::Renderer::MaterialId rect_material_id() const noexcept
    {
        return m_rect_material_id;
    }

private:
    khepri::ui::Canvas&              m_canvas;
    khepri::UIRenderer&              m_ui_renderer;
    khepri::ui::Renderer::MaterialId m_sprite_material_id;
    khepri::ui::Renderer::MaterialId m_rect_material_id;
    openglyph::AssetCache&           m_asset_cache;
    const openglyph::StringDatabase& m_string_database;
    const StyleStore&                m_style_store;
    MegaTexture                      m_mega_texture;
    khepri::ui::Renderer::TextureId  m_mega_texture_id;

    mutable std::map<std::string, khepri::ui::Frame::TextureSource, khepri::CaseInsensitiveLess>
                                                                                        m_textures;
    mutable std::unordered_map<const khepri::font::Font*, khepri::ui::Renderer::FontId> m_fonts;
};

} // namespace openglyph::ui