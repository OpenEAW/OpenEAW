#include <khepri/adapters/ui_renderer.hpp>
#include <khepri/exceptions.hpp>

namespace khepri {

UIRenderer::UIRenderer(khepri::renderer::Renderer&             renderer,
                       const khepri::renderer::RenderPipeline* render_pipeline)
    : m_renderer(renderer), m_render_pipeline(render_pipeline)
{
}

void UIRenderer::begin_render(const Size& canvas_size)
{
    assert(!m_rendering);
    if (m_rendering) {
        throw khepri::InvalidOperationException();
    }
    m_rendering                = true;
    m_render_state.canvas_size = canvas_size;
}

void UIRenderer::render_rects(gsl::span<const FilledRect> rects, const Rect& clip_rect)
{
    assert(m_rendering);
    if (!m_rendering) {
        throw khepri::InvalidOperationException();
    }

    if (m_render_pipeline == nullptr) {
        // Can't render
        return;
    }

    const khepri::renderer::Material* current_material = nullptr;

    const auto screen_clip_rect = to_screen_space(clip_rect);

    std::vector<khepri::renderer::Sprite> sprites;

    const auto& render_sprites = [&] {
        if (!sprites.empty() && current_material != nullptr) {
            m_renderer.render_sprites(*m_render_pipeline, sprites, *current_material, {},
                                      screen_clip_rect);
            sprites.clear();
        }
    };

    // Map UI coordinates: (0,0) - (W,H), positive Y goes down to camera space:
    // (-1,-1) - (1,1), positive Y goes up.
    const auto& to_camera_space = [&](long x, long y) {
        return Vector2f((2.0f * x) / m_render_state.canvas_size.width - 1.0f,
                        1.0f - (2.0f * y) / m_render_state.canvas_size.height);
    };

    sprites.reserve(rects.size());
    for (const auto& rect : rects) {
        khepri::renderer::Sprite sprite;
        sprite.position_top_left = to_camera_space(rect.area.x, rect.area.y);
        sprite.position_bottom_right =
            to_camera_space(rect.area.x + rect.area.width, rect.area.y + rect.area.height);

        const auto* material = m_materials.find(rect.material_id);
        if (current_material != material) {
            // The render settings have changed, render what we've got before changing settings
            render_sprites();

            current_material = material;
        }

        sprites.push_back(sprite);
    }

    // Render any remaining sprites
    render_sprites();
}

void UIRenderer::render_quads(gsl::span<const Quad> quads, const Rect& clip_rect)
{
    assert(m_rendering);
    if (!m_rendering) {
        throw khepri::InvalidOperationException();
    }

    if (m_render_pipeline == nullptr) {
        // Can't render
        return;
    }

    const khepri::renderer::Texture*  current_texture  = nullptr;
    const khepri::renderer::Material* current_material = nullptr;

    const auto screen_clip_rect = to_screen_space(clip_rect);

    std::vector<khepri::renderer::Sprite> sprites;

    const auto& render_sprites = [&] {
        if (!sprites.empty() && current_material != nullptr) {
            khepri::renderer::Material::Param texture_param;
            texture_param.name  = "BaseTexture"; // TODO: read name from material
            texture_param.value = current_texture;
            m_renderer.render_sprites(*m_render_pipeline, sprites, *current_material,
                                      {&texture_param, 1}, screen_clip_rect);
            sprites.clear();
        }
    };

    // Map UI coordinates: (0,0) - (W,H), positive Y goes down to camera space:
    // (-1,-1) - (1,1), positive Y goes up.
    const auto& to_camera_space = [&](long x, long y) {
        return Vector2f((2.0f * x) / m_render_state.canvas_size.width - 1.0f,
                        1.0f - (2.0f * y) / m_render_state.canvas_size.height);
    };

    sprites.reserve(quads.size());
    for (const auto& quad : quads) {
        khepri::renderer::Sprite sprite;
        sprite.position_top_left = to_camera_space(quad.area.x, quad.area.y);
        sprite.position_bottom_right =
            to_camera_space(quad.area.x + quad.area.width, quad.area.y + quad.area.height);

        const auto* material = m_materials.find(quad.material_id);
        const auto* texture  = m_textures.find(quad.texture_id);
        if (texture) {
            const auto& size        = texture->size();
            const auto& to_uv_space = [&](long u, long v) {
                return Vector2f(static_cast<float>(u) / size.width,
                                static_cast<float>(v) / size.height);
            };

            sprite.uv_top_left     = to_uv_space(quad.tex_rect.x, quad.tex_rect.y);
            sprite.uv_bottom_right = to_uv_space(quad.tex_rect.x + quad.tex_rect.width,
                                                 quad.tex_rect.y + quad.tex_rect.height);
        }

        if (current_texture != texture || current_material != material) {
            // The render settings have changed, render what we've got before changing settings
            render_sprites();

            current_texture  = texture;
            current_material = material;
        }

        sprites.push_back(sprite);
    }

    // Render any remaining sprites
    render_sprites();
}

void UIRenderer::render_texts(gsl::span<const Text> texts, const Rect& clip_rect)
{
    assert(m_rendering);
    if (!m_rendering) {
        throw khepri::InvalidOperationException();
    }

    const auto screen_clip_rect = to_screen_space(clip_rect);

    // Map UI coordinates: (0,0) - (canvas_width,canvas_height), positive Y goes down to camera
    // space:
    // (-1,-1) - (1,1), positive Y goes up.
    const auto& to_camera_space = [&](long x, long y) {
        return Vector2f((2.0f * x) / m_render_state.canvas_size.width - 1.0f,
                        1.0f - (2.0f * y) / m_render_state.canvas_size.height);
    };

    khepri::renderer::Material::Param texture_param;
    texture_param.name = "BaseTexture"; // TODO: read name from material

    for (const auto& text : texts) {
        const auto font     = m_fonts.find(text.font_id);
        const auto material = m_materials.find(text.material_id);
        if (font && material) {
            const auto text_render = font->render(text.string);
            // FIXME: creating a texture for every text on every frame :(
            if (const auto texture = m_renderer.create_texture(text_render.texture_desc)) {
                const auto& size        = texture->size();
                const auto& to_uv_space = [&](long u, long v) {
                    return Vector2f(static_cast<float>(u) / size.width,
                                    static_cast<float>(v) / size.height);
                };

                const auto quad_tex_rect = text_render.rect;
                auto       quad_area =
                    khepri::Rect{0, 0, text_render.rect.width, text_render.rect.height};

                // Layout the quad to be centered
                const auto rect = text.area;
                long       dx   = 0;
                switch (text.text_align) {
                case khepri::ui::TextAlign::left:
                default:
                    dx = 0;
                    break;
                case khepri::ui::TextAlign::center:
                    dx = (static_cast<long>(rect.width) - static_cast<long>(quad_tex_rect.width)) /
                         2;
                    break;
                case khepri::ui::TextAlign::right:
                    dx = static_cast<long>(rect.width) - static_cast<long>(quad_tex_rect.width);
                    break;
                }
                // Vertically center
                long dy =
                    (static_cast<long>(rect.height) - static_cast<long>(quad_tex_rect.height)) / 2;

                quad_area.x = rect.x + dx;
                quad_area.y = rect.y + dy;

                khepri::renderer::Sprite sprite;
                sprite.position_top_left = to_camera_space(quad_area.x, quad_area.y);
                sprite.position_bottom_right =
                    to_camera_space(quad_area.x + quad_area.width, quad_area.y + quad_area.height);
                sprite.uv_top_left     = to_uv_space(quad_tex_rect.x, quad_tex_rect.y);
                sprite.uv_bottom_right = to_uv_space(quad_tex_rect.x + quad_tex_rect.width,
                                                     quad_tex_rect.y + quad_tex_rect.height);

                texture_param.value = texture.get();

                m_renderer.render_sprites(*m_render_pipeline, {&sprite, 1}, *material,
                                          {&texture_param, 1}, screen_clip_rect);
            }
        }
    }
}

void UIRenderer::end_render()
{
    assert(m_rendering);
    if (!m_rendering) {
        throw khepri::InvalidOperationException();
    }
    m_rendering = false;
}

UIRenderer::TextureId UIRenderer::register_texture(const khepri::renderer::Texture* texture)
{
    return m_textures.add(texture);
}

UIRenderer::FontId UIRenderer::register_font(const khepri::font::Font* font)
{
    return m_fonts.add(font);
}

UIRenderer::MaterialId UIRenderer::register_material(const khepri::renderer::Material* material)
{
    return m_materials.add(material);
}

const khepri::font::Font* UIRenderer::get_font(FontId font_id) const noexcept
{
    return m_fonts.find(font_id);
}

template <typename Key, typename Value>
Key UIRenderer::ObjectCache<Key, Value>::add(const Value* value)
{
    assert(value != nullptr);
    const auto key = m_next_id++;
    m_objects.emplace(key, std::move(value));
    return key;
}

template <typename Key, typename Value>
const Value* UIRenderer::ObjectCache<Key, Value>::find(const Key& key) const noexcept
{
    if (const auto it = m_objects.find(key); it != m_objects.end()) {
        return it->second;
    }
    return nullptr;
}

Rect UIRenderer::to_screen_space(const Rect& rect) const noexcept
{
    const auto& scale = [&](long x, long canvas_size, long window_size) {
        return window_size > 0
                   ? static_cast<long>((static_cast<double>(x) / canvas_size) * window_size)
                   : 0;
    };

    const auto render_size   = m_renderer.render_size();
    const auto screen_left   = scale(rect.x, m_render_state.canvas_size.width, render_size.width);
    const auto screen_top    = scale(rect.y, m_render_state.canvas_size.height, render_size.height);
    const auto screen_right  = scale(static_cast<long>(rect.x + rect.width),
                                     m_render_state.canvas_size.width, render_size.width);
    const auto screen_bottom = scale(static_cast<long>(rect.y + rect.height),
                                     m_render_state.canvas_size.height, render_size.height);

    return {screen_left, screen_top, static_cast<unsigned long>(screen_right - screen_left),
            static_cast<unsigned long>(screen_bottom - screen_top)};
}

} // namespace khepri