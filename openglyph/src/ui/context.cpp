#include <khepri/log/log.hpp>

#include <openglyph/assets/io/mega_texture.hpp>
#include <openglyph/ui/context.hpp>

namespace openglyph::ui {
namespace {
constexpr khepri::log::Logger LOG("ui");

// Default fontname, used when the font specified in the UI configuration cannot be loaded.
// Note: Glyph uses "Arial Unicode MS", but that doesn't exist on Linux. So we use the open-source
// Noto (sans-serif) font instead, which looks very similar.
constexpr auto* DEFAULT_FONTNAME = "NotoSansDisplay-Regular";

static constexpr khepri::Size CANVAS_SIZE{1024u, 768u};

// Converts a point size to a pixel size, assuming a 800x600 resolution at 96 dpi
constexpr unsigned int point_to_px(unsigned int pt)
{
    constexpr unsigned int SCREEN_DPI     = 96;
    constexpr unsigned int SCREEN_HEIGHT  = 600;
    constexpr double       INCH_PER_POINT = 1.0 / 72;

    // Scale the assumed DPI for the font definition to the canvas DPI.
    // Note: this assumes a fixed canvas size. But the renderer text is scaled up or down, across a
    // screen which may be larger. To get pixel-perfect fonts, we should use the real window height
    // instead of the virtual canvas size. But then a single resize invalidates all fonts.
    const auto canvas_dpi = static_cast<double>(SCREEN_DPI) / SCREEN_HEIGHT * CANVAS_SIZE.height;

    // A point is 1/72th of an inch
    return static_cast<unsigned int>(pt * INCH_PER_POINT * canvas_dpi + 0.5);
}
} // namespace

Context::Context(khepri::ui::Canvas& canvas, khepri::UIRenderer& ui_renderer,
                 openglyph::AssetLoader& asset_loader, openglyph::AssetCache& asset_cache,
                 const openglyph::StringDatabase& string_database, const StyleStore& style_store)
    : m_canvas(canvas)
    , m_ui_renderer(ui_renderer)
    , m_asset_cache(asset_cache)
    , m_string_database(string_database)
    , m_style_store(style_store)
{
    try {
        const auto mega_texture_name = std::string(m_style_store.mega_texture_filename());
        if (auto stream = asset_loader.open_mega_texture(mega_texture_name + ".mtd")) {
            m_mega_texture = io::read_mega_texture(*stream);
        }

        if (auto texture = m_asset_cache.get_texture(mega_texture_name + ".dds")) {
            m_mega_texture_id = m_ui_renderer.register_texture(texture);
        }
    } catch (const std::exception& e) {
        LOG.error("cannot load UI texture: {}", e.what());

        m_mega_texture    = {};
        m_mega_texture_id = khepri::ui::Renderer::INVALID_TEXTURE;
    }

    if (const auto* sprite_material = m_asset_cache.get_material("GUI_Sprite")) {
        m_sprite_material_id = m_ui_renderer.register_material(sprite_material);
    }

    if (const auto* rect_material = m_asset_cache.get_material("GUI_Rect")) {
        m_rect_material_id = m_ui_renderer.register_material(rect_material);
    }
}

const khepri::ui::Frame::TextureSource& Context::get_texture(std::string_view ui_texture_name) const
{
    if (khepri::case_insensitive_equals(ui_texture_name, "none")) {
        // Special case for no texture
        static const khepri::ui::Frame::TextureSource null_texture{};
        return null_texture;
    }

    auto [it, inserted] = m_textures.emplace(ui_texture_name, khepri::ui::Frame::TextureSource{});
    if (inserted) {
        // The entry didn't exist yet, load the texture
        if (const auto rect = m_mega_texture.texture_area(ui_texture_name)) {
            // Found it in the mega texture
            it->second = {m_mega_texture_id, *rect};
        } else if (auto texture = m_asset_cache.get_texture(ui_texture_name)) {
            // Found it as a regular texture
            it->second = {m_ui_renderer.register_texture(texture),
                          {0, 0, texture->size().width, texture->size().height}};
        } else {
            // Not found
            it->second = {khepri::ui::Renderer::INVALID_TEXTURE, {}};
        }
    }
    return it->second;
}

khepri::ui::Renderer::FontId Context::get_font(const StyleStore::FontStyle& style) const
{
    khepri::font::FontOptions options;
    options.font_size_px = point_to_px(style.point_size);
    // TODO: support the alpha component?
    options.color_top      = khepri::ColorRGB(style.color_top);
    options.color_bottom   = khepri::ColorRGB(style.color_bottom);
    options.vert_scale     = style.stretch_factor;
    options.stroke_size_px = style.has_stroke ? 1 : 0;
    options.embossed       = style.has_emboss;
    // TODO: style.char_spacing;

    const auto* font = m_asset_cache.get_font(style.name, options);
    if (font == nullptr) {
        // Requested font doesn't exist, try to load the default font
        font = m_asset_cache.get_font(DEFAULT_FONTNAME, options);
        if (font == nullptr) {
            return khepri::ui::Renderer::INVALID_FONT;
        }
    }

    if (const auto it = m_fonts.find(font); it != m_fonts.end()) {
        return it->second;
    }
    const auto font_id = m_ui_renderer.register_font(font);
    m_fonts.emplace(font, font_id);
    return font_id;
}

} // namespace openglyph::ui