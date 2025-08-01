#pragma once

#include "model.hpp"
#include "render_model.hpp"

#include <khepri/io/exceptions.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/renderer.hpp>

namespace openglyph::renderer {

class ModelCreator
{
public:
    template <typename T>
    using Loader = std::function<T*(std::string_view)>;

    ModelCreator(khepri::renderer::Renderer&              renderer,
                 Loader<const khepri::renderer::Material> material_loader,
                 Loader<const khepri::renderer::Texture>  texture_loader);
    ~ModelCreator() = default;

    ModelCreator(const ModelCreator&)                = delete;
    ModelCreator(ModelCreator&&) noexcept            = delete;
    ModelCreator& operator=(const ModelCreator&)     = delete;
    ModelCreator& operator=(ModelCreator&&) noexcept = delete;

    std::unique_ptr<RenderModel> create_model(const Model& model);

private:
    khepri::renderer::Renderer&              m_renderer;
    Loader<const khepri::renderer::Material> m_material_loader;
    Loader<const khepri::renderer::Texture>  m_texture_loader;
};

} // namespace openglyph::renderer
