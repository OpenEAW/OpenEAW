#include <openglyph/renderer/render_pipeline_store.hpp>

namespace openglyph::renderer {

RenderPipelineStore::RenderPipelineStore(khepri::renderer::Renderer& renderer)
    : m_renderer(renderer)
{
}

void RenderPipelineStore::register_render_pipelines(
    gsl::span<const khepri::renderer::RenderPipelineDesc> pipeline_descs)
{
    for (const auto& desc : pipeline_descs) {
        m_render_pipelines.emplace(desc.name, m_renderer.create_render_pipeline(desc));
    }
}

khepri::renderer::RenderPipeline* RenderPipelineStore::get(std::string_view name) const noexcept
{
    const auto it = m_render_pipelines.find(name);
    return (it != m_render_pipelines.end()) ? it->second.get() : nullptr;
}

} // namespace openglyph::renderer
