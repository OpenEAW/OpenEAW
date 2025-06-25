#include "native_window.hpp"
#include "shader_stream_factory.hpp"

#include <khepri/exceptions.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/diligent/renderer.hpp>
#include <khepri/renderer/exceptions.hpp>
#include <khepri/utility/string.hpp>

#ifdef _MSC_VER
#include <EngineFactoryD3D11.h>
#else
#include <EngineFactoryOpenGL.h>
#endif
#include <DebugOutput.h>
#include <DeviceContext.h>
#include <HLSL2GLSLConverter.h>
#include <MapHelper.hpp>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <Sampler.h>
#include <SwapChain.h>
#include <Texture.h>
#include <iterator>
#include <unordered_map>
#include <utility>

using namespace Diligent;

namespace khepri::renderer::diligent {
namespace {
constexpr khepri::log::Logger LOG("diligent");
struct InstanceConstantBuffer
{
    Matrixf world;
    Matrixf world_inv;
};

struct ViewConstantBuffer
{
    Matrixf view_proj;
};

CULL_MODE to_cull_mode(MaterialDesc::CullMode cull_mode) noexcept
{
    switch (cull_mode) {
    case MaterialDesc::CullMode::none:
        return CULL_MODE_NONE;
    case MaterialDesc::CullMode::back:
        return CULL_MODE_BACK;
    case MaterialDesc::CullMode::front:
        return CULL_MODE_FRONT;
    default:
        break;
    }
    assert(false);
    return CULL_MODE_NONE;
}

RESOURCE_DIMENSION to_resource_dimension(TextureDimension dimension, bool is_array)
{
    switch (dimension) {
    case TextureDimension::texture_1d:
        return is_array ? RESOURCE_DIM_TEX_1D_ARRAY : RESOURCE_DIM_TEX_1D;
    case TextureDimension::texture_2d:
        return is_array ? RESOURCE_DIM_TEX_2D_ARRAY : RESOURCE_DIM_TEX_2D;
    case TextureDimension::texture_3d:
        // 3D textures cannot be arrays
        assert(!is_array);
        return RESOURCE_DIM_TEX_3D;
    case TextureDimension::texture_cubemap:
        return is_array ? RESOURCE_DIM_TEX_CUBE_ARRAY : RESOURCE_DIM_TEX_CUBE;
    }
    assert(false);
    return RESOURCE_DIM_UNDEFINED;
}

TEXTURE_FORMAT to_texture_format(PixelFormat format)
{
    switch (format) {
    case PixelFormat::r8g8b8a8_unorm_srgb:
        return TEX_FORMAT_RGBA8_UNORM_SRGB;
    case PixelFormat::b8g8r8a8_unorm_srgb:
        return TEX_FORMAT_BGRA8_UNORM_SRGB;
    case PixelFormat::bc1_unorm_srgb:
        return TEX_FORMAT_BC1_UNORM_SRGB;
    case PixelFormat::bc2_unorm_srgb:
        return TEX_FORMAT_BC2_UNORM_SRGB;
    case PixelFormat::bc3_unorm_srgb:
        return TEX_FORMAT_BC3_UNORM_SRGB;
    }
    assert(false);
    return TEX_FORMAT_UNKNOWN;
}

COMPARISON_FUNCTION to_comparison_func(MaterialDesc::ComparisonFunc func)
{
    switch (func) {
    case MaterialDesc::ComparisonFunc::never:
        return COMPARISON_FUNC_NEVER;
    case MaterialDesc::ComparisonFunc::less:
        return COMPARISON_FUNC_LESS;
    case MaterialDesc::ComparisonFunc::equal:
        return COMPARISON_FUNC_EQUAL;
    case MaterialDesc::ComparisonFunc::less_equal:
        return COMPARISON_FUNC_LESS_EQUAL;
    case MaterialDesc::ComparisonFunc::greater:
        return COMPARISON_FUNC_GREATER;
    case MaterialDesc::ComparisonFunc::not_equal:
        return COMPARISON_FUNC_NOT_EQUAL;
    case MaterialDesc::ComparisonFunc::greater_equal:
        return COMPARISON_FUNC_GREATER_EQUAL;
    case MaterialDesc::ComparisonFunc::always:
        return COMPARISON_FUNC_ALWAYS;
    }
    assert(false);
    return COMPARISON_FUNC_UNKNOWN;
}

// helper type for a variant visitor
template <class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

void diligent_debug_message_callback(DEBUG_MESSAGE_SEVERITY severity, const char* message,
                                     const char* /*function*/, const char* /*file*/, int /*line*/)
{
    switch (severity) {
    case DEBUG_MESSAGE_SEVERITY_INFO:
        LOG.info("{}", message);
        break;
    case DEBUG_MESSAGE_SEVERITY_WARNING:
        LOG.warning("{}", message);
        break;
    case DEBUG_MESSAGE_SEVERITY_ERROR:
    case DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
        LOG.error("{}", message);
        break;
    }
}

constexpr bool using_shader_conversion()
{
#ifdef _MSC_VER
    return false;
#else
    // OpenGL uses shader conversion
    return true;
#endif
}

} // namespace

class Renderer::Impl
{
    struct Shader : public khepri::renderer::Shader
    {
        RefCntAutoPtr<IShader> vertex_shader;
        RefCntAutoPtr<IShader> pixel_shader;
    };

    struct Mesh : public khepri::renderer::Mesh
    {
        using Index = khepri::renderer::MeshDesc::Index;

        Index                  index_count{0};
        RefCntAutoPtr<IBuffer> vertex_buffer;
        RefCntAutoPtr<IBuffer> index_buffer;
    };

    struct Material : public khepri::renderer::Material
    {
        struct Param
        {
            std::string                                   name;
            khepri::renderer::MaterialDesc::PropertyValue default_value;
            size_t                                        buffer_offset;
        };

        RefCntAutoPtr<IPipelineState>         pipeline;
        RefCntAutoPtr<IShaderResourceBinding> shader_resource_binding;
        RefCntAutoPtr<IBuffer>                param_buffer;
        std::vector<Param>                    params;
    };

    struct Texture : public khepri::renderer::Texture
    {
        using khepri::renderer::Texture::Texture;

        RefCntAutoPtr<ITexture> texture;
        ITextureView*           shader_view{};
    };

public:
    Impl(const std::any& window)
    {
        SetDebugMessageCallback(diligent_debug_message_callback);
        const auto native_window = get_native_window(window);

#ifdef _MSC_VER
        auto* factory = GetEngineFactoryD3D11();

        EngineD3D11CreateInfo engine_ci;
#ifndef NDEBUG
        engine_ci.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
        factory->CreateDeviceAndContextsD3D11(engine_ci, &m_device, &m_context);

        SwapChainDesc      swapchain_desc;
        FullScreenModeDesc fullscreenmode_desc;
        factory->CreateSwapChainD3D11(m_device, m_context, swapchain_desc, fullscreenmode_desc,
                                      native_window, &m_swapchain);
        static_assert(!using_shader_conversion());
#else
        auto* factory = GetEngineFactoryOpenGL();

        EngineGLCreateInfo engine_ci{};
#ifndef NDEBUG
        engine_ci.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
        // Enable separate programs to support querying shader resources
        engine_ci.Features.SeparablePrograms = DEVICE_FEATURE_STATE_ENABLED;

        engine_ci.Window = native_window;
        const SwapChainDesc swapchain_desc;
        factory->CreateDeviceAndSwapChainGL(engine_ci, &m_device, &m_context, swapchain_desc,
                                            &m_swapchain);
        static_assert(using_shader_conversion());
#endif
        if (m_device == nullptr || m_context == nullptr || m_swapchain == nullptr) {
            throw khepri::renderer::Error("Failed to create renderer");
        }

        // Create constants buffers for vertex shader
        {
            BufferDesc desc;
            desc.Name           = "VS Instance Constants";
            desc.Size           = sizeof(InstanceConstantBuffer);
            desc.Usage          = USAGE_DYNAMIC;
            desc.BindFlags      = BIND_UNIFORM_BUFFER;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, nullptr, &m_constants_instance);
        }

        {
            BufferDesc desc;
            desc.Name           = "VS View Constants";
            desc.Size           = sizeof(ViewConstantBuffer);
            desc.Usage          = USAGE_DYNAMIC;
            desc.BindFlags      = BIND_UNIFORM_BUFFER;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, nullptr, &m_constants_view);
        }

        // Create dynamic buffers for sprite rendering
        {
            const BufferData buffer_data{};
            BufferDesc       desc;
            desc.Size           = static_cast<Uint32>(SPRITE_BUFFER_COUNT * VERTICES_PER_SPRITE *
                                                      sizeof(SpriteVertex));
            desc.BindFlags      = BIND_VERTEX_BUFFER;
            desc.Usage          = USAGE_DYNAMIC;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, &buffer_data, &m_sprite_vertex_buffer);
        }

        {
            std::vector<std::uint16_t> indices(SPRITE_BUFFER_COUNT * TRIANGLES_PER_SPRITE *
                                               VERTICES_PER_TRIANGLE);
            for (std::uint16_t i = 0, j = 0; i < SPRITE_BUFFER_COUNT;
                 i += VERTICES_PER_SPRITE, j += TRIANGLES_PER_SPRITE * VERTICES_PER_TRIANGLE) {
                const auto triangle0   = j;
                indices[triangle0 + 0] = i + 0;
                indices[triangle0 + 1] = i + 2;
                indices[triangle0 + 2] = i + 1;

                const auto triangle1   = j + VERTICES_PER_TRIANGLE;
                indices[triangle1 + 0] = i + 0;
                indices[triangle1 + 1] = i + 3;
                indices[triangle1 + 2] = i + 2;
            }

            const BufferData bufdata{indices.data(),
                                     static_cast<Uint32>(indices.size() * sizeof(std::uint16_t))};
            BufferDesc       desc;
            desc.Size      = bufdata.DataSize;
            desc.BindFlags = BIND_INDEX_BUFFER;
            desc.Usage     = USAGE_IMMUTABLE;
            m_device->CreateBuffer(desc, &bufdata, &m_sprite_index_buffer);
        }
    }

    Impl(const Impl&)            = delete;
    Impl(Impl&&)                 = delete;
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&)      = delete;

    ~Impl() = default;

    void render_size(const Size& size)
    {
        m_swapchain->Resize(size.width, size.height);
    }

    [[nodiscard]] Size render_size() const noexcept
    {
        const auto& desc = m_swapchain->GetDesc();
        return {desc.Width, desc.Height};
    }

    [[nodiscard]] std::unique_ptr<Shader> create_shader(const std::filesystem::path& path,
                                                        const ShaderLoader&          loader)
    {
        RefCntAutoPtr<ShaderStreamFactory> factory(MakeNewRCObj<ShaderStreamFactory>()(loader));

        auto shader = std::make_unique<Shader>();
        shader->vertex_shader =
            create_shader_object(path.string(), *factory, SHADER_TYPE_VERTEX, "vs_main");
        if (!shader->vertex_shader) {
            throw khepri::renderer::Error("Failed to create vertex shader from file: " +
                                          path.string());
        }

        shader->pixel_shader =
            create_shader_object(path.string(), *factory, SHADER_TYPE_PIXEL, "ps_main");
        if (!shader->pixel_shader) {
            throw khepri::renderer::Error("Failed to create pixel shader from file: " +
                                          path.string());
        }
        return shader;
    }

    [[nodiscard]] std::unique_ptr<Material>
    create_material(const khepri::renderer::MaterialDesc& material_desc)
    {
        auto* const shader = dynamic_cast<Shader*>(material_desc.shader);
        if (shader == nullptr) {
            throw ArgumentError();
        }

        assert(shader->vertex_shader != nullptr);
        assert(shader->pixel_shader != nullptr);

        const auto dynamic_variables =
            determine_dynamic_material_variables(*shader, material_desc.properties);

        GraphicsPipelineStateCreateInfo ci;
        ci.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        switch (material_desc.alpha_blend_mode) {
        case MaterialDesc::AlphaBlendMode::additive:
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend    = BLEND_FACTOR_ONE;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend   = BLEND_FACTOR_ONE;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp     = BLEND_OPERATION_ADD;
            break;
        case MaterialDesc::AlphaBlendMode::blend_src:
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend    = BLEND_FACTOR_SRC_ALPHA;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend   = BLEND_FACTOR_INV_SRC_ALPHA;
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp     = BLEND_OPERATION_ADD;
            break;
        case MaterialDesc::AlphaBlendMode::none:
            ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = false;
            break;
        }
        ci.GraphicsPipeline.NumRenderTargets = 1;
        ci.GraphicsPipeline.RTVFormats[0]    = m_swapchain->GetDesc().ColorBufferFormat;
        ci.GraphicsPipeline.DSVFormat        = m_swapchain->GetDesc().DepthBufferFormat;
        if (material_desc.depth_buffer) {
            ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
            ci.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable =
                material_desc.depth_buffer->write_enable;
            ci.GraphicsPipeline.DepthStencilDesc.DepthFunc =
                to_comparison_func(material_desc.depth_buffer->comparison_func);
        } else {
            ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
        }
        ci.GraphicsPipeline.RasterizerDesc.CullMode = to_cull_mode(material_desc.cull_mode);

        static_assert(sizeof(MeshDesc::Vertex) < std::numeric_limits<Uint32>::max(),
                      "Vertex is too large");

        constexpr auto                                 num_layout_elements = 6;
        std::array<LayoutElement, num_layout_elements> layout{
            LayoutElement{0, 0, 3, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, position)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))},
            LayoutElement{1, 0, 3, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, normal)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))},
            LayoutElement{2, 0, 3, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, tangent)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))},
            LayoutElement{3, 0, 3, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, binormal)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))},
            LayoutElement{4, 0, 2, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, uv)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))},
            LayoutElement{5, 0, 4, VT_FLOAT32, false,
                          static_cast<Uint32>(offsetof(MeshDesc::Vertex, color)),
                          static_cast<Uint32>(sizeof(MeshDesc::Vertex))}};
        ci.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
        ci.GraphicsPipeline.InputLayout.NumElements    = static_cast<Uint32>(layout.size());

        ci.pVS = shader->vertex_shader;
        ci.pPS = shader->pixel_shader;

        // Mark all material properties as dynamic (the rest is static by default)
        std::vector<ShaderResourceVariableDesc> variables;
        for (const auto& var : dynamic_variables) {
            variables.emplace_back(SHADER_TYPE_VERTEX, var.c_str(),
                                   SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
            variables.emplace_back(SHADER_TYPE_PIXEL, var.c_str(),
                                   SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
        }

        ci.PSODesc.ResourceLayout.Variables    = variables.data();
        ci.PSODesc.ResourceLayout.NumVariables = static_cast<Uint32>(variables.size());

        auto material = std::make_unique<Material>();
        m_device->CreateGraphicsPipelineState(ci, &material->pipeline);

        if (auto* var = material->pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX,
                                                                    "InstanceConstants")) {
            var->Set(m_constants_instance);
        }
        if (auto* var =
                material->pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX, "ViewConstants")) {
            var->Set(m_constants_view);
        }
        material->pipeline->CreateShaderResourceBinding(&material->shader_resource_binding, true);

        const auto& property_size = [](const MaterialDesc::PropertyValue& value) -> Uint32 {
            // Every type has its own size, except for textures, which don't take up space.
            return std::visit(
                Overloaded{[&](khepri::renderer::Texture* /*texture*/) -> Uint32 { return 0; },
                           [&](const auto& value) -> Uint32 { return sizeof(value); }},
                value);
        };

        Uint32 buffer_size = 0;
        material->params.reserve(material_desc.properties.size());
        for (const auto& p : material_desc.properties) {
            material->params.push_back({p.name, p.default_value, buffer_size});
            buffer_size += property_size(p.default_value);
            // Align next parameter to 16 bytes
            constexpr auto param_alignment = 16;
            buffer_size = (buffer_size + param_alignment - 1) / param_alignment * param_alignment;
        }

        // Create the material properties buffer
        if (buffer_size > 0) {
            BufferDesc desc;
            desc.Name           = "Material Constants";
            desc.Size           = buffer_size;
            desc.Usage          = USAGE_DYNAMIC;
            desc.BindFlags      = BIND_UNIFORM_BUFFER;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, nullptr, &material->param_buffer);
        }
        return material;
    }

    [[nodiscard]] std::unique_ptr<Texture> create_texture(const TextureDesc& texture_desc)
    {
        Diligent::TextureDesc desc;
        desc.Type  = to_resource_dimension(texture_desc.dimension(), texture_desc.array_size() > 0);
        desc.Width = static_cast<Uint32>(texture_desc.width());
        desc.Height    = static_cast<Uint32>(texture_desc.height());
        desc.Format    = to_texture_format(texture_desc.pixel_format());
        desc.MipLevels = static_cast<Uint32>(texture_desc.mip_levels());
        desc.Usage     = USAGE_IMMUTABLE;
        desc.BindFlags = BIND_SHADER_RESOURCE;

        const std::size_t array_size        = std::max<std::size_t>(1, texture_desc.array_size());
        const std::size_t subresource_count = array_size * texture_desc.mip_levels();

        if (texture_desc.dimension() == TextureDimension::texture_3d) {
            desc.Depth = static_cast<Uint32>(texture_desc.depth()); // NOLINT - union access
        } else {
            desc.ArraySize = static_cast<Uint32>(array_size); // NOLINT - union access
        }

        std::vector<TextureSubResData> subresources(subresource_count);

        auto subresource = subresources.begin();
        for (std::size_t index = 0; index < array_size; ++index) {
            for (std::size_t mip = 0; mip < texture_desc.mip_levels(); ++mip, ++subresource) {
                assert(subresource != subresources.end());
                const auto& src =
                    texture_desc.subresource(texture_desc.subresource_index(mip, index));
                subresource->pData       = texture_desc.data().data() + src.data_offset;
                subresource->Stride      = static_cast<Uint32>(src.stride);
                subresource->DepthStride = static_cast<Uint32>(src.depth_stride);
            }
        }

        Diligent::TextureData texdata;
        texdata.pSubResources   = subresources.data();
        texdata.NumSubresources = static_cast<Uint32>(subresources.size());

        auto texture = std::make_unique<Texture>(Size{texture_desc.width(), texture_desc.height()});
        m_device->CreateTexture(desc, &texdata, &texture->texture);
        if (!texture->texture) {
            throw khepri::renderer::Error("Failed to create texture");
        }
        texture->shader_view = texture->texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        // Create the associated sampler. We use combined samplers to simplify OpenGL development.
        SamplerDesc sampler_desc;
        sampler_desc.MinFilter = FILTER_TYPE_LINEAR;
        sampler_desc.MagFilter = FILTER_TYPE_LINEAR;
        sampler_desc.MipFilter = FILTER_TYPE_LINEAR;
        sampler_desc.AddressU  = TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV  = TEXTURE_ADDRESS_WRAP;
        Diligent::RefCntAutoPtr<Diligent::ISampler> sampler;
        m_device->CreateSampler(sampler_desc, &sampler);
        if (!sampler) {
            throw khepri::renderer::Error("Failed to create texture sampler");
        }
        texture->shader_view->SetSampler(sampler);

        return texture;
    }

    [[nodiscard]] std::unique_ptr<Mesh> create_mesh(const MeshDesc& mesh_desc)
    {
        auto mesh         = std::make_unique<Mesh>();
        mesh->index_count = static_cast<Mesh::Index>(mesh_desc.indices.size());

        {
            using Vertex = khepri::renderer::MeshDesc::Vertex;
            const BufferData bufdata{
                mesh_desc.vertices.data(),
                static_cast<Uint32>(mesh_desc.vertices.size() * sizeof(Vertex))};
            BufferDesc desc;
            desc.Size      = bufdata.DataSize;
            desc.BindFlags = BIND_VERTEX_BUFFER;
            desc.Usage     = USAGE_IMMUTABLE;
            m_device->CreateBuffer(desc, &bufdata, &mesh->vertex_buffer);
        }

        {
            const BufferData bufdata{
                mesh_desc.indices.data(),
                static_cast<Uint32>(mesh_desc.indices.size() * sizeof(MeshDesc::Index))};
            BufferDesc desc;
            desc.Size      = bufdata.DataSize;
            desc.BindFlags = BIND_INDEX_BUFFER;
            desc.Usage     = USAGE_IMMUTABLE;
            m_device->CreateBuffer(desc, &bufdata, &mesh->index_buffer);
        }

        return mesh;
    }

    void clear(ClearFlags flags)
    {
        auto* rtv = m_swapchain->GetCurrentBackBufferRTV();
        auto* dsv = m_swapchain->GetDepthBufferDSV();

        m_context->SetRenderTargets(1, &rtv, dsv, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        if ((flags & clear_rendertarget) != 0) {
            std::array<float, 4> color = {0, 0, 0, 1};
            m_context->ClearRenderTarget(rtv, color.data(),
                                         RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        if ((flags & (clear_depth | clear_stencil)) != 0) {
            CLEAR_DEPTH_STENCIL_FLAGS ctx_flags = CLEAR_DEPTH_FLAG_NONE;
            if ((flags & clear_depth) != 0) {
                ctx_flags |= CLEAR_DEPTH_FLAG;
            }
            if ((flags & clear_stencil) != 0) {
                ctx_flags |= CLEAR_STENCIL_FLAG;
            }
            m_context->ClearDepthStencil(dsv, ctx_flags, 1.0F, 0,
                                         RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    void present()
    {
        m_swapchain->Present();
    }

    void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera)
    {
        // Set the view-specific constants
        {
            MapHelper<ViewConstantBuffer> constants(m_context, m_constants_view, MAP_WRITE,
                                                    MAP_FLAG_DISCARD);
            constants->view_proj = camera.matrices().view_proj;
        }

        for (const auto& mesh_info : meshes) {
            auto* const material = dynamic_cast<Material*>(mesh_info.material);
            auto* const mesh     = dynamic_cast<Mesh*>(mesh_info.mesh);
            if (material == nullptr || mesh == nullptr) {
                throw ArgumentError();
            }

            m_context->SetPipelineState(material->pipeline);

            apply_material_params(*material, mesh_info.material_params);

            m_context->CommitShaderResources(material->shader_resource_binding,
                                             RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            IBuffer* vertex_buffer = mesh->vertex_buffer.RawPtr();
            m_context->SetVertexBuffers(0, 1, &vertex_buffer, nullptr,
                                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                        SET_VERTEX_BUFFERS_FLAG_RESET);
            m_context->SetIndexBuffer(mesh->index_buffer, 0,
                                      RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            // Set instance-specific constants
            {
                MapHelper<InstanceConstantBuffer> constants(m_context, m_constants_instance,
                                                            MAP_WRITE, MAP_FLAG_DISCARD);
                constants->world     = mesh_info.transform;
                constants->world_inv = inverse(mesh_info.transform);
            }

            static_assert(sizeof(Mesh::Index) == sizeof(std::uint16_t));
            DrawIndexedAttribs draw_attribs;
            draw_attribs.NumIndices = mesh->index_count;
            draw_attribs.IndexType  = VT_UINT16;
#ifndef NDEBUG
            draw_attribs.Flags = DRAW_FLAG_VERIFY_ALL;
#endif
            m_context->DrawIndexed(draw_attribs);
        }
    }

    void render_sprites(gsl::span<const Sprite> sprites, khepri::renderer::Material& material,
                        gsl::span<const khepri::renderer::Material::Param> params)
    {
        auto* mat = dynamic_cast<Material*>(&material);
        if (mat == nullptr) {
            throw ArgumentError();
        }

        m_context->SetPipelineState(mat->pipeline);
        apply_material_params(*mat, params);

        m_context->CommitShaderResources(mat->shader_resource_binding,
                                         RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        std::size_t sprite_index = 0;
        while (sprite_index < sprites.size()) {
            const std::size_t sprites_left = sprites.size() - sprite_index;
            const std::size_t sprite_count = std::min(sprites_left, SPRITE_BUFFER_COUNT);

            {
                // Copy the vertex data
                MapHelper<SpriteVertex> vertices_map(m_context, m_sprite_vertex_buffer, MAP_WRITE,
                                                     MAP_FLAG_DISCARD);

                const auto vertices = gsl::span<SpriteVertex>(
                    vertices_map, m_sprite_vertex_buffer->GetDesc().Size / sizeof(SpriteVertex));

                for (std::size_t i = 0; i < sprite_count * VERTICES_PER_SPRITE;
                     i += VERTICES_PER_SPRITE, ++sprite_index) {
                    const auto& sprite = sprites[sprite_index];
                    vertices[i + 0].position =
                        Vector3f(sprite.position_top_left.x, sprite.position_top_left.y, 0);
                    vertices[i + 1].position =
                        Vector3f(sprite.position_bottom_right.x, sprite.position_top_left.y, 0);
                    vertices[i + 2].position =
                        Vector3f(sprite.position_bottom_right.x, sprite.position_bottom_right.y, 0);
                    vertices[i + 3].position =
                        Vector3f(sprite.position_top_left.x, sprite.position_bottom_right.y, 0);
                    vertices[i + 0].uv = Vector2f(sprite.uv_top_left.x, sprite.uv_top_left.y);
                    vertices[i + 1].uv = Vector2f(sprite.uv_bottom_right.x, sprite.uv_top_left.y);
                    vertices[i + 2].uv =
                        Vector2f(sprite.uv_bottom_right.x, sprite.uv_bottom_right.y);
                    vertices[i + 3].uv = Vector2f(sprite.uv_top_left.x, sprite.uv_bottom_right.y);
                }
            }

            m_context->SetVertexBuffers(0, 1, &m_sprite_vertex_buffer, nullptr,
                                        RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                        SET_VERTEX_BUFFERS_FLAG_RESET);

            m_context->SetIndexBuffer(m_sprite_index_buffer, 0,
                                      RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            static_assert(sizeof(Mesh::Index) == sizeof(std::uint16_t));
            DrawIndexedAttribs draw_attribs;
            draw_attribs.NumIndices =
                static_cast<Uint32>(sprite_count * TRIANGLES_PER_SPRITE * VERTICES_PER_TRIANGLE);
            draw_attribs.IndexType = VT_UINT16;
#ifndef NDEBUG
            draw_attribs.Flags = DRAW_FLAG_VERIFY_ALL;
#endif
            m_context->DrawIndexed(draw_attribs);
        }
    }

private:
    static constexpr unsigned int TRIANGLES_PER_SPRITE  = 2;
    static constexpr unsigned int VERTICES_PER_TRIANGLE = 3;

    // Number of vertices to render one sprite
    static constexpr std::size_t VERTICES_PER_SPRITE = 4;

    // Number of sprites that fit in the sprite vertex/index buffers
    static constexpr std::size_t SPRITE_BUFFER_COUNT = 1024;

    using SpriteVertex = MeshDesc::Vertex;

    RefCntAutoPtr<IShader> create_shader_object(const std::string&   path,
                                                ShaderStreamFactory& factory,
                                                SHADER_TYPE          shader_type,
                                                const std::string&   entrypoint)
    {
        RefCntAutoPtr<IDataBlob>                  compiler_output;
        RefCntAutoPtr<IHLSL2GLSLConversionStream> conversion_stream;

        ShaderCreateInfo ci;
        ci.Desc.Name                  = path.c_str();
        ci.Desc.ShaderType            = shader_type;
        ci.FilePath                   = path.c_str();
        ci.pShaderSourceStreamFactory = &factory;
        ci.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;
        ci.EntryPoint                 = entrypoint.c_str();
        ci.ppCompilerOutput           = &compiler_output;
        ci.ppConversionStream         = &conversion_stream;

        // Note: to support OpenGL, we need to use combined texture samplers.
        // This means HLSL shaders need to define a "Foo" texture and associated "FooSampler"
        // sampler, which are treated as one by Diligent. This does increase the number of
        // samplers.
        ci.Desc.UseCombinedTextureSamplers = true;
        ci.Desc.CombinedSamplerSuffix      = "Sampler";

        RefCntAutoPtr<IShader> shader;
        m_device->CreateShader(ci, &shader);
        if (shader == nullptr) {
            LOG.error("Failed to create shader from file: {}, error: {}", path,
                      compiler_output != nullptr
                          ? static_cast<const char*>(compiler_output->GetConstDataPtr())
                          : "<unknown>");
            if constexpr (using_shader_conversion()) {
                print_converted_glsl_source(std::move(conversion_stream), path, factory,
                                            shader_type, entrypoint);
            }
        }
        return shader;
    }

    static void
    print_converted_glsl_source(RefCntAutoPtr<IHLSL2GLSLConversionStream> conversion_stream,
                                const std::string& path, ShaderStreamFactory& factory,
                                SHADER_TYPE shader_type, const std::string& entrypoint)
    {
        if (conversion_stream == nullptr) {
            // If we didn't get a conversion stream, try to create one ourselves
            RefCntAutoPtr<IHLSL2GLSLConverter> converter;
            Diligent::CreateHLSL2GLSLConverter(&converter);
            converter->CreateStream(path.c_str(), &factory, nullptr, 0, &conversion_stream);
        }

        RefCntAutoPtr<IDataBlob> glsl_source;
        if (conversion_stream != nullptr) {
            conversion_stream->Convert(entrypoint.c_str(), shader_type, true, "Sampler", false,
                                       &glsl_source);
        }

        if (glsl_source != nullptr) {
            const char* glsl = static_cast<const char*>(glsl_source->GetConstDataPtr());
            LOG.info("Converted GLSL source:");
            int line_number = 1;
            for (const auto& line : khepri::split(glsl, "\n", true)) {
                LOG.info("{}: {}", line_number++, line);
            }
            LOG.info("---End of GLSL source---");
        } else {
            LOG.info("Couldn't convert shader to GLSL for diagnostics.");
        }
    }

    void apply_material_params(Material&                                          material,
                               gsl::span<const khepri::renderer::Material::Param> params)
    {
        const auto& set_variable = [&](const char* name, IDeviceObject* object) {
            auto& srb = *material.shader_resource_binding;
            if (auto* var = srb.GetVariableByName(SHADER_TYPE_VERTEX, name)) {
                var->Set(object);
            }
            if (auto* var = srb.GetVariableByName(SHADER_TYPE_PIXEL, name)) {
                var->Set(object);
            }
        };

        std::optional<MapHelper<std::uint8_t>> map_helper;
        if (material.param_buffer != nullptr) {
            map_helper = MapHelper<std::uint8_t>(m_context, material.param_buffer, MAP_WRITE,
                                                 MAP_FLAG_DISCARD);
        }

        for (const auto& param : material.params) {
            // Use the value from the provided params if it exists, otherwise the material's default
            const auto* const it = std::find_if(
                params.begin(), params.end(), [&](const auto& p) { return p.name == param.name; });
            const auto& value = (it != params.end()) ? it->value : param.default_value;

            std::visit(Overloaded{[&](khepri::renderer::Texture* value) {
                                      auto* texture = dynamic_cast<Texture*>(value);
                                      if (texture != nullptr) {
                                          set_variable(param.name.c_str(), texture->shader_view);
                                      }
                                  },
                                  [&](const auto& value) {
                                      if (map_helper) {
                                          // NOLINTBEGIN - pointer arithmetic and reinterpret_cast
                                          auto* param_data =
                                              static_cast<std::uint8_t*>(*map_helper) +
                                              param.buffer_offset;
                                          *reinterpret_cast<std::decay_t<decltype(value)>*>(
                                              param_data) = value;
                                          // NOLINTEND
                                      }
                                  }},
                       value);
        }

        if (material.param_buffer != nullptr) {
            set_variable("Material", material.param_buffer);
        }
    }

    static std::vector<std::string>
    determine_dynamic_material_variables(const Shader&                              shader,
                                         const std::vector<MaterialDesc::Property>& properties)
    {
        // Any top-level material property with the same name as these is an error
        const std::unordered_map<std::string, SHADER_RESOURCE_TYPE> predefined_variables{
            {"InstanceConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
            {"ViewConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
            {"Material", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
        };

        // Collect all top-level shader resources, they must be matched by top-level material
        // properties
        std::unordered_map<std::string, SHADER_RESOURCE_TYPE> shader_variables;
        for (const auto& shader : {shader.vertex_shader, shader.pixel_shader}) {
            const Uint32 count = shader->GetResourceCount();
            for (Uint32 i = 0; i < count; ++i) {
                ShaderResourceDesc desc;
                shader->GetResourceDesc(i, desc);
                if (desc.Type == SHADER_RESOURCE_TYPE_SAMPLER) {
                    // We're using combined samplers, so we don't need to track these
                    continue;
                }

                const auto it = predefined_variables.find(desc.Name);
                if (it == predefined_variables.end()) {
                    // No predefined variable, remember it as a top-level shader variable
                    shader_variables[desc.Name] = desc.Type;
                } else if (it->second != desc.Type) {
                    LOG.error(
                        "type of property \"{}\" in material does not match shader variable type",
                        desc.Name);
                    throw ArgumentError();
                }
            }
        }

        // Validate properties and collect all dynamic top-level materials
        std::vector<std::string> dynamic_variables;
        for (const auto& p : properties) {
            if (!std::holds_alternative<khepri::renderer::Texture*>(p.default_value)) {
                // Non-texture properties are fine, they are in a cbuffer and not top-level
                // variables
                continue;
            }

            if (predefined_variables.find(p.name) != predefined_variables.end()) {
                // Invalid variable name
                throw ArgumentError();
            }

            // See if the property has a matching shader variable
            const auto it = shader_variables.find(p.name);
            if (it == shader_variables.end()) {
                LOG.error("missing shader variable for property \"{}\"", p.name);
                throw ArgumentError();
            }
            if (it->second != SHADER_RESOURCE_TYPE_TEXTURE_SRV) {
                LOG.error("mismatch for shader variable type for property \"{}\"", p.name);
                throw ArgumentError();
            }
            // We've seen this one, remove it
            shader_variables.erase(it);

            dynamic_variables.push_back(p.name);
        }
        dynamic_variables.emplace_back("Material");

        if (!shader_variables.empty()) {
            // Not all shader variables have been accounted for
            LOG.error("missing material property for shader variable \"{}\"",
                      shader_variables.begin()->first);
            throw ArgumentError();
        }
        return dynamic_variables;
    }

    bool                                              m_using_shader_conversion{false};
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_device;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_context;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_swapchain;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_instance;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_constants_view;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_sprite_vertex_buffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>        m_sprite_index_buffer;
};

Renderer::Renderer(const std::any& window) : m_impl(std::make_unique<Impl>(window)) {}

Renderer::~Renderer() = default;

void Renderer::render_size(const Size& size)
{
    m_impl->render_size(size);
}

Size Renderer::render_size() const noexcept
{
    return m_impl->render_size();
}

std::unique_ptr<Shader> Renderer::create_shader(const std::filesystem::path& path,
                                                const ShaderLoader&          loader)
{
    return m_impl->create_shader(path, loader);
}

std::unique_ptr<Material> Renderer::create_material(const MaterialDesc& material_desc)
{
    return m_impl->create_material(material_desc);
}

std::unique_ptr<Texture> Renderer::create_texture(const TextureDesc& texture_desc)
{
    return m_impl->create_texture(texture_desc);
}

std::unique_ptr<Mesh> Renderer::create_mesh(const MeshDesc& mesh_desc)
{
    return m_impl->create_mesh(mesh_desc);
}

void Renderer::clear(ClearFlags flags)
{
    m_impl->clear(flags);
}

void Renderer::present()
{
    m_impl->present();
}

void Renderer::render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera)
{
    m_impl->render_meshes(meshes, camera);
}

void Renderer::render_sprites(gsl::span<const Sprite> sprites, Material& material,
                              gsl::span<const Material::Param> params)
{
    m_impl->render_sprites(sprites, material, params);
}

} // namespace khepri::renderer::diligent
