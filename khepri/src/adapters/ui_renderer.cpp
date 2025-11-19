#pragma once

#include <khepri/adapters/ui_renderer.hpp>

namespace khepri {

UIRenderer::UIRenderer(khepri::renderer::Renderer&             renderer,
                       const khepri::renderer::RenderPipeline& render_pipeline,
                       const khepri::renderer::Material&       sprite_material)
    : m_renderer(renderer), m_render_pipeline(render_pipeline), m_sprite_material(sprite_material)
{
}

void UIRenderer::begin_render() {}
void UIRenderer::end_render() {}

void UIRenderer::render_quads(gsl::span<const Quad> quads, const Size& canvas_size)
{
    std::vector<khepri::renderer::Sprite> sprites;

    // Assign a null texture so the variant has the right type
    khepri::renderer::Material::Param texture_param;
    texture_param.name = "BaseTexture"; // TODO: read name from material
    texture_param.value.emplace<const khepri::renderer::Texture*>(nullptr);

    // Scissor rect for the current sprites
    Rect scissor_rect{0, 0, 0, 0};

    const auto& render_sprites = [&] {
        if (!sprites.empty()) {
            // TODO: use scissor_rect
            m_renderer.render_sprites(m_render_pipeline, sprites, m_sprite_material,
                                      {&texture_param, 1});
            sprites.clear();
        }
    };

    // Map UI coordinates: (0,0) - (W,H), positive Y goes down to camera space:
    // (-1,-1) - (1,1), positive Y goes up.
    const auto& to_camera_space = [&](long x, long y) {
        return Vector2f((2.0f * x) / canvas_size.width - 1.0f,
                        1.0f - (2.0f * y) / canvas_size.height);
    };

    sprites.reserve(quads.size());
    for (const auto& quad : quads) {
        khepri::renderer::Sprite sprite;
        sprite.position_top_left = to_camera_space(quad.area.x, quad.area.y);
        sprite.position_bottom_right =
            to_camera_space(quad.area.x + quad.area.width, quad.area.y + quad.area.height);

        const auto* texture = find_texture(quad.texture_id);
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
        sprites.push_back(sprite);

        if (std::get<const khepri::renderer::Texture*>(texture_param.value) != texture ||
            scissor_rect != quad.clip_rect) {
            // The render settings have changed, render what we've got before changing settings
            render_sprites();

            texture_param.value = texture;
            scissor_rect        = quad.clip_rect;
        }
    }

    // Render any remaining sprites
    render_sprites();
}

void UIRenderer::render_texts(gsl::span<const Text> texts, const Size& canvas_size)
{
    // TODO
}

UIRenderer::TextureId UIRenderer::create_texture(const khepri::renderer::TextureDesc& texture_desc)
{
    auto texture    = m_renderer.create_texture(texture_desc);
    auto texture_id = m_next_texture_id++;
    m_textures.emplace(texture_id, std::move(texture));
    return texture_id;
}

khepri::renderer::Texture* UIRenderer::find_texture(TextureId texture_id) const
{
    if (const auto it = m_textures.find(texture_id); it != m_textures.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace khepri