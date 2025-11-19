#pragma once

#include <khepri/renderer/renderer.hpp>
#include <khepri/ui/renderer.hpp>

#include <unordered_map>

namespace khepri {

/**
 * \brief UI renderer adapter that uses a khepri::renderer::Renderer to render UIs.
 */
class UIRenderer : public ui::Renderer
{
public:
    explicit UIRenderer(khepri::renderer::Renderer&             renderer,
                        const khepri::renderer::RenderPipeline& render_pipeline,
                        const khepri::renderer::Material&       sprite_material);

    void begin_render() override;
    void end_render() override;

    void render_quads(gsl::span<const Quad> quads, const Size& canvas_size) override;
    void render_texts(gsl::span<const Text> texts, const Size& canvas_size) override;

    TextureId create_texture(const khepri::renderer::TextureDesc& texture_desc);

private:
    khepri::renderer::Texture* find_texture(TextureId texture_id) const;

    khepri::renderer::Renderer&             m_renderer;
    const khepri::renderer::RenderPipeline& m_render_pipeline;
    const khepri::renderer::Material&       m_sprite_material;

    std::unordered_map<TextureId, std::unique_ptr<khepri::renderer::Texture>> m_textures;
    TextureId                                                                 m_next_texture_id{0};
};

} // namespace khepri