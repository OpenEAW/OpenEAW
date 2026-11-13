#pragma once

#include <khepri/font/font.hpp>
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
    /**
     * Constructs a UI renderer.
     *
     * \param renderer the renderer to use to render the UI.
     * \param render_pipeline the render pipeline to use to render the UI.
     * \param sprite_material the material to use the render the UI sprites.
     *
     * If \a pipeline or \a material are null, this object effectively becomes a null object. Any
     * render methods become no-ops. This is done so the object can still be used in UI logic
     * without requiring the user to check for the case the pipeline and/or material cannot be
     * created.
     */
    UIRenderer(khepri::renderer::Renderer&             renderer,
               const khepri::renderer::RenderPipeline* render_pipeline);

    /**
     * Returns the size of the rendering area
     */
    [[nodiscard]] khepri::Size render_size() const noexcept
    {
        return m_renderer.render_size();
    }

    void begin_render(const Size& canvas_size) override;
    void end_render() override;

    void render_rects(gsl::span<const FilledRect> rects, const Rect& clip_rect) override;
    void render_quads(gsl::span<const Quad> quads, const Rect& clip_rect) override;
    void render_texts(gsl::span<const Text> texts, const Rect& clip_rect) override;

    TextureId  register_texture(const khepri::renderer::Texture* texture);
    FontId     register_font(const khepri::font::Font* font);
    MaterialId register_material(const khepri::renderer::Material* material);

    const khepri::font::Font* get_font(FontId font_id) const noexcept;

private:
    void do_render();

    // Map UI coordinates: (0,0) - (canvas_width,canvas_height), positive Y goes down to viewport
    // space: (0,0) - (screen_width,screen_height), positive Y goes down.
    Rect to_screen_space(const Rect& rect) const noexcept;

    struct RenderState
    {
        Size canvas_size;
    };

    template <typename Key, typename Value>
    class ObjectCache final
    {
    public:
        Key add(const Value* value);

        const Value* find(const Key& key) const noexcept;

    private:
        std::unordered_map<Key, const Value*> m_objects;
        Key                                   m_next_id{0};
    };

    khepri::renderer::Renderer&             m_renderer;
    const khepri::renderer::RenderPipeline* m_render_pipeline;

    ObjectCache<TextureId, const khepri::renderer::Texture>   m_textures;
    ObjectCache<FontId, const khepri::font::Font>             m_fonts;
    ObjectCache<MaterialId, const khepri::renderer::Material> m_materials;

    bool        m_rendering{false};
    RenderState m_render_state;
};

} // namespace khepri