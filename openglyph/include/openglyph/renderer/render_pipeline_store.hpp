#pragma once

#include <khepri/renderer/render_pipeline.hpp>
#include <khepri/renderer/render_pipeline_desc.hpp>
#include <khepri/renderer/renderer.hpp>
#include <khepri/utility/string.hpp>

#include <gsl/gsl-lite.hpp>

#include <map>
#include <string>
#include <string_view>

namespace openglyph::renderer {

class RenderPipelineStore final
{
public:
    explicit RenderPipelineStore(khepri::renderer::Renderer& renderer);
    ~RenderPipelineStore() = default;

    RenderPipelineStore(const RenderPipelineStore&)                = delete;
    RenderPipelineStore(RenderPipelineStore&&) noexcept            = delete;
    RenderPipelineStore& operator=(const RenderPipelineStore&)     = delete;
    RenderPipelineStore& operator=(RenderPipelineStore&&) noexcept = delete;

    void
    register_render_pipelines(gsl::span<const khepri::renderer::RenderPipelineDesc> pipeline_descs);

    [[nodiscard]] khepri::renderer::RenderPipeline* get(std::string_view name) const noexcept;

    auto as_loader()
    {
        return [this](std::string_view id) { return this->get(id); };
    }

private:
    using RenderPipelineMap =
        std::map<std::string, std::unique_ptr<khepri::renderer::RenderPipeline>,
                 khepri::CaseInsensitiveLess>;

    khepri::renderer::Renderer& m_renderer;

    RenderPipelineMap m_render_pipelines;
};

} // namespace openglyph::renderer
