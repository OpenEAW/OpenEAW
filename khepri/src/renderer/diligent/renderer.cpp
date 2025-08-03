#include "native_window.hpp"
#include "refcnt_ptr.hpp"
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
#include <RenderDevice.h>
#include <Sampler.h>
#include <SwapChain.h>
#include <Texture.h>
#include <functional>
#include <iterator>
#include <stack>
#include <unordered_map>
#include <utility>

using namespace Diligent;

namespace khepri::renderer::diligent {
namespace {
constexpr khepri::log::Logger LOG("diligent");

// Index of a render pass in a render pipeline
using LocalRenderPassIndex = std::size_t;

// Index of a render pass in a global render pass collection
using GlobalRenderPassIndex = std::size_t;

#pragma pack(push, 4)
struct InstanceConstantBuffer
{
    Matrixf world;
    Matrixf world_inv;
};
static_assert(sizeof(InstanceConstantBuffer) == 8 * 16); // Validate packing

struct ViewConstantBuffer
{
    Matrixf view;
    Matrixf view_proj;
    Matrixf view_proj_inv;
};
static_assert(sizeof(ViewConstantBuffer) == 12 * 16); // Validate packing

struct DirectionalLight
{
    Vector3f direction;
    float    intensity;
    Vector3f diffuse_color;
    long : 32;
    Vector3f specular_color;
    long : 32;
};
static_assert(sizeof(DirectionalLight) == 3 * 16); // Validate packing

struct PointLight
{
    Vector3f direction;
    float    intensity;
    Vector3f diffuse_color;
    long : 32;
    Vector3f specular_color;
    float    max_distance;
};
static_assert(sizeof(PointLight) == 3 * 16); // Validate packing
#pragma pack(pop)

// Combine the default and override options into a final set of options.
// All optional members in the result will be set.
GraphicsPipelineOptions combine_options(const GraphicsPipelineOptions& default_options,
                                        const GraphicsPipelineOptions& override_options)
{
    // Description of the fields in GraphicsPipelineOptions, and their defaults
    using GPO = GraphicsPipelineOptions;
    const std::tuple fields{std::tuple(&GPO::cull_mode, GPO::CullMode::back),
                            std::tuple(&GPO::front_ccw, false),
                            std::tuple(&GPO::alpha_blend_mode, GPO::AlphaBlendMode::none),
                            std::tuple(&GPO::depth_enable, true),
                            std::tuple(&GPO::depth_comparison_func, GPO::ComparisonFunc::less),
                            std::tuple(&GPO::depth_write_enable, true)};

    GraphicsPipelineOptions combined_options;

    const auto& combine_field = [&](auto& field) {
        const auto& field_ptr     = std::get<0>(field);
        const auto& default_value = std::get<1>(field);
        combined_options.*field_ptr =
            (override_options.*field_ptr)
                .value_or((default_options.*field_ptr).value_or(default_value));
    };

    std::apply([&](const auto&... f) { (combine_field(f), ...); }, fields);
    return combined_options;
}

CULL_MODE to_cull_mode(GraphicsPipelineOptions::CullMode cull_mode) noexcept
{
    switch (cull_mode) {
    case GraphicsPipelineOptions::CullMode::none:
        return CULL_MODE_NONE;
    case GraphicsPipelineOptions::CullMode::back:
        return CULL_MODE_BACK;
    case GraphicsPipelineOptions::CullMode::front:
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
    case PixelFormat::r8g8b8a8_unorm:
        return TEX_FORMAT_RGBA8_UNORM;
    case PixelFormat::r8g8b8a8_unorm_srgb:
        return TEX_FORMAT_RGBA8_UNORM_SRGB;
    case PixelFormat::b8g8r8a8_unorm:
        return TEX_FORMAT_BGRA8_UNORM;
    case PixelFormat::b8g8r8a8_unorm_srgb:
        return TEX_FORMAT_BGRA8_UNORM_SRGB;
    case PixelFormat::bc1_unorm:
        return TEX_FORMAT_BC1_UNORM;
    case PixelFormat::bc1_unorm_srgb:
        return TEX_FORMAT_BC1_UNORM_SRGB;
    case PixelFormat::bc2_unorm:
        return TEX_FORMAT_BC2_UNORM;
    case PixelFormat::bc2_unorm_srgb:
        return TEX_FORMAT_BC2_UNORM_SRGB;
    case PixelFormat::bc3_unorm:
        return TEX_FORMAT_BC3_UNORM;
    case PixelFormat::bc3_unorm_srgb:
        return TEX_FORMAT_BC3_UNORM_SRGB;
    }
    assert(false);
    return TEX_FORMAT_UNKNOWN;
}

COMPARISON_FUNCTION to_comparison_func(GraphicsPipelineOptions::ComparisonFunc func)
{
    switch (func) {
    case GraphicsPipelineOptions::ComparisonFunc::never:
        return COMPARISON_FUNC_NEVER;
    case GraphicsPipelineOptions::ComparisonFunc::less:
        return COMPARISON_FUNC_LESS;
    case GraphicsPipelineOptions::ComparisonFunc::equal:
        return COMPARISON_FUNC_EQUAL;
    case GraphicsPipelineOptions::ComparisonFunc::less_equal:
        return COMPARISON_FUNC_LESS_EQUAL;
    case GraphicsPipelineOptions::ComparisonFunc::greater:
        return COMPARISON_FUNC_GREATER;
    case GraphicsPipelineOptions::ComparisonFunc::not_equal:
        return COMPARISON_FUNC_NOT_EQUAL;
    case GraphicsPipelineOptions::ComparisonFunc::greater_equal:
        return COMPARISON_FUNC_GREATER_EQUAL;
    case GraphicsPipelineOptions::ComparisonFunc::always:
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
    struct ConstantsBuffers
    {
        RefCntPtr<IBuffer> instance;
        RefCntPtr<IBuffer> view;
        RefCntPtr<IBuffer> directional_lights;
        RefCntPtr<IBuffer> point_lights;
        RefCntPtr<IBuffer> environment;
    };

    struct Shader : public khepri::renderer::Shader
    {
        RefCntPtr<IShader> vertex_shader;
        RefCntPtr<IShader> pixel_shader;
    };

    struct Mesh : public khepri::renderer::Mesh
    {
        using Index = khepri::renderer::MeshDesc::Index;

        Index              index_count{0};
        RefCntPtr<IBuffer> vertex_buffer;
        RefCntPtr<IBuffer> index_buffer;
    };

    class Material : public khepri::renderer::Material
    {
        static Shader copy_shader(const khepri::renderer::Shader* generic_shader)
        {
            auto* const shader = dynamic_cast<const Shader*>(generic_shader);
            if (shader == nullptr) {
                throw ArgumentError();
            }
            assert(shader->vertex_shader != nullptr);
            assert(shader->pixel_shader != nullptr);
            return *shader;
        }

    public:
        Material(IRenderDevice& device, ISwapChain& swapchain, const MaterialDesc& desc,
                 std::function<void(Material&)> destroy_callback)
            : m_device(device)
            , m_swapchain(swapchain)
            , m_destroy_callback(destroy_callback)
            , m_type(desc.type)
            , m_num_directional_lights(desc.num_directional_lights)
            , m_num_point_lights(desc.num_point_lights)
            , m_graphics_pipeline_options(desc.graphics_pipeline_options)
            , m_shader(copy_shader(desc.shader))
            , m_dynamic_variables(determine_dynamic_material_variables(m_shader, desc.properties))
        {
            if (desc.num_directional_lights < 0 || desc.num_point_lights < 0) {
                throw ArgumentError();
            }

            Uint32 buffer_size = 0;
            m_params.reserve(desc.properties.size());
            for (const auto& p : desc.properties) {
                // Every type has its own size, except for textures, which don't take up space.
                const auto property_size = std::visit(
                    Overloaded{[&](const khepri::renderer::Texture*) -> Uint32 { return 0; },
                               [&](const auto& value) -> Uint32 { return sizeof(value); }},
                    p.default_value);

                constexpr auto param_alignment = 16;
                const auto     remaining_size  = param_alignment - (buffer_size % param_alignment);
                if (property_size > remaining_size) {
                    // The next property doesn't fit in the remaining space in this alignment
                    // 'block'. Align parameter to multiple of 16 bytes.
                    buffer_size =
                        (buffer_size + param_alignment - 1) / param_alignment * param_alignment;
                }

                m_params.push_back({p.name, p.default_value, buffer_size});
                buffer_size += property_size;
            }

            // Create the material properties buffer
            if (buffer_size > 0) {
                BufferDesc desc;
                desc.Name           = "Material Constants";
                desc.Size           = buffer_size;
                desc.Usage          = USAGE_DYNAMIC;
                desc.BindFlags      = BIND_UNIFORM_BUFFER;
                desc.CPUAccessFlags = CPU_ACCESS_WRITE;
                m_device.CreateBuffer(desc, nullptr, &m_param_buffer);
            }
        }

        ~Material() override
        {
            m_destroy_callback(*this);
        }

        int num_directional_lights() const noexcept
        {
            return m_num_directional_lights;
        }

        int num_point_lights() const noexcept
        {
            return m_num_point_lights;
        }

        void set_render_pass(GlobalRenderPassIndex render_pass_index, const RenderPassDesc& desc,
                             ConstantsBuffers& constants)
        {
            // Check if this material is rendered in the render pass
            if (!khepri::case_insensitive_equals(desc.material_type, m_type)) {
                // Nope, nothing to do
                return;
            }

            if (render_pass_index >= m_render_pass_data.size()) {
                m_render_pass_data.resize(render_pass_index + 1);
            }

            GraphicsPipelineStateCreateInfo ci;
            ci.PSODesc.PipelineType              = PIPELINE_TYPE_GRAPHICS;
            ci.GraphicsPipeline.NumRenderTargets = 1;
            ci.GraphicsPipeline.RTVFormats[0]    = m_swapchain.GetDesc().ColorBufferFormat;
            ci.GraphicsPipeline.DSVFormat        = m_swapchain.GetDesc().DepthBufferFormat;

            const auto combined_options = combine_options(desc.default_graphics_pipeline_options,
                                                          m_graphics_pipeline_options);

            switch (*combined_options.alpha_blend_mode) {
            case GraphicsPipelineOptions::AlphaBlendMode::additive:
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend    = BLEND_FACTOR_ONE;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend   = BLEND_FACTOR_ONE;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp     = BLEND_OPERATION_ADD;
                break;
            case GraphicsPipelineOptions::AlphaBlendMode::blend_src:
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend    = BLEND_FACTOR_SRC_ALPHA;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend =
                    BLEND_FACTOR_INV_SRC_ALPHA;
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = BLEND_OPERATION_ADD;
                break;
            case GraphicsPipelineOptions::AlphaBlendMode::none:
                ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = false;
                break;
            }

            ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = *combined_options.depth_enable;
            ci.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable =
                *combined_options.depth_write_enable;
            ci.GraphicsPipeline.DepthStencilDesc.DepthFunc =
                to_comparison_func(*combined_options.depth_comparison_func);
            ci.GraphicsPipeline.RasterizerDesc.CullMode = to_cull_mode(*combined_options.cull_mode);
            ci.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;

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

            ci.pPS = m_shader.pixel_shader;
            ci.pVS = m_shader.vertex_shader;

            // Mark all material properties as dynamic (the rest is static by default)
            std::vector<ShaderResourceVariableDesc> variables;
            for (const auto& var : m_dynamic_variables) {
                variables.emplace_back(SHADER_TYPE_VERTEX, var.c_str(),
                                       SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
                variables.emplace_back(SHADER_TYPE_PIXEL, var.c_str(),
                                       SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
            }

            ci.PSODesc.ResourceLayout.Variables    = variables.data();
            ci.PSODesc.ResourceLayout.NumVariables = static_cast<Uint32>(variables.size());

            RenderPassData data;
            m_device.CreateGraphicsPipelineState(ci, &data.pipeline);

            const auto& set_variable = [&](const char* name, IDeviceObject* object) {
                if (auto* var = data.pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX, name)) {
                    var->Set(object);
                }
                if (auto* var = data.pipeline->GetStaticVariableByName(SHADER_TYPE_PIXEL, name)) {
                    var->Set(object);
                }
            };

            set_variable("InstanceConstants", constants.instance);
            set_variable("ViewConstants", constants.view);

            data.pipeline->CreateShaderResourceBinding(&data.shader_resource_binding, true);

            m_render_pass_data[render_pass_index] = std::move(data);
        }

        void clear_render_pass(GlobalRenderPassIndex render_pass_index)
        {
            if (render_pass_index < m_render_pass_data.size()) {
                m_render_pass_data[render_pass_index] = {};
            }
        }

        // Checks if this material is used during this render pass
        bool is_used(GlobalRenderPassIndex render_pass_index) const noexcept
        {
            return render_pass_index < m_render_pass_data.size() &&
                   m_render_pass_data[render_pass_index].pipeline;
        }

        // Activates the material for the given render pass on the context with specified
        // parameters.
        void set_active(GlobalRenderPassIndex render_pass_index, IDeviceContext& context,
                        gsl::span<const khepri::renderer::Material::Param> params,
                        IBuffer& directional_lights_buffer) const
        {
            if (render_pass_index < m_render_pass_data.size()) {
                if (auto& data = m_render_pass_data[render_pass_index]; data.pipeline) {
                    context.SetPipelineState(data.pipeline);

                    apply_material_params(data, context, params);

                    set_variable(data, "DirectionalLightConstants", &directional_lights_buffer);

                    context.CommitShaderResources(data.shader_resource_binding,
                                                  RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
            }
        }

    private:
        struct Param
        {
            std::string                                   name;
            khepri::renderer::MaterialDesc::PropertyValue default_value;
            size_t                                        buffer_offset;
        };

        // Data for this material for a particular render pass.
        // The indices for render passes are shared by all materials and managed by the renderer,
        // not here.
        struct RenderPassData
        {
            // The graphics pipeline for this render pass/material combo.
            // Not set if this material is not renderer in this render pass.
            RefCntPtr<IPipelineState> pipeline;

            RefCntPtr<IShaderResourceBinding> shader_resource_binding;
        };

        void apply_material_params(const RenderPassData& data, IDeviceContext& context,
                                   gsl::span<const khepri::renderer::Material::Param> params) const
        {
            std::optional<MapHelper<std::uint8_t>> map_helper;
            if (m_param_buffer != nullptr) {
                map_helper =
                    MapHelper<std::uint8_t>(&context, m_param_buffer, MAP_WRITE, MAP_FLAG_DISCARD);
            }

            for (const auto& param : m_params) {
                // Use the value from the provided params if it exists, otherwise the material's
                // default
                const auto* const it =
                    std::find_if(params.begin(), params.end(),
                                 [&](const auto& p) { return p.name == param.name; });
                const auto& value = (it != params.end()) ? it->value : param.default_value;

                std::visit(
                    Overloaded{[&](const khepri::renderer::Texture* value) {
                                   auto* texture = dynamic_cast<const Texture*>(value);
                                   if (texture != nullptr) {
                                       set_variable(data, param.name.c_str(), texture->shader_view);
                                   }
                               },
                               [&](const auto& value) {
                                   if (map_helper) {
                                       // NOLINTBEGIN - pointer arithmetic and reinterpret_cast
                                       auto* param_data = static_cast<std::uint8_t*>(*map_helper) +
                                                          param.buffer_offset;
                                       *reinterpret_cast<std::decay_t<decltype(value)>*>(
                                           param_data) = value;
                                       // NOLINTEND
                                   }
                               }},
                    value);
            }

            if (m_param_buffer != nullptr) {
                set_variable(data, "Material", m_param_buffer);
            }
        }

        void set_variable(const RenderPassData& data, const char* name, IDeviceObject* object) const
        {
            auto& srb = *data.shader_resource_binding;
            if (auto* var = srb.GetVariableByName(SHADER_TYPE_VERTEX, name)) {
                var->Set(object);
            }
            if (auto* var = srb.GetVariableByName(SHADER_TYPE_PIXEL, name)) {
                var->Set(object);
            }
        }

        IRenderDevice&                 m_device;
        ISwapChain&                    m_swapchain;
        std::function<void(Material&)> m_destroy_callback;

        //
        // The material's original data (as specified in the material description)
        //

        // The material type (for matching with the RenderPass material filter)
        std::string m_type;

        // The material light count
        int m_num_directional_lights;
        int m_num_point_lights;

        // The material's graphics pipeline options
        GraphicsPipelineOptions m_graphics_pipeline_options;

        // The material's original shader
        Shader m_shader;
        // Names of variables in the shaders that are dynamic (can change on every render)
        std::vector<std::string> m_dynamic_variables;

        // The graphics pipelines for each render pass in each render pipeline
        std::vector<RenderPassData> m_render_pass_data;

        // Buffer for the material's parameters
        RefCntPtr<IBuffer> m_param_buffer;
        std::vector<Param> m_params;
    };

    struct Texture : public khepri::renderer::Texture
    {
        using khepri::renderer::Texture::Texture;

        RefCntPtr<ITexture> texture;
        ITextureView*       shader_view{};
    };

    class RenderPipeline : public khepri::renderer::RenderPipeline
    {
    public:
        RenderPipeline(const std::vector<GlobalRenderPassIndex>& render_pass_indices,
                       std::function<void(RenderPipeline&)>      destroy_callback)
            : m_render_pass_indices(render_pass_indices)
            , m_destroy_callback(std::move(destroy_callback))
        {
        }

        ~RenderPipeline()
        {
            m_destroy_callback(*this);
        }

        const auto& render_pass_indices() const noexcept
        {
            return m_render_pass_indices;
        }

    private:
        std::vector<GlobalRenderPassIndex>   m_render_pass_indices;
        std::function<void(RenderPipeline&)> m_destroy_callback;
    };

public:
    Impl(const std::any& window, ColorSpace color_space)
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

        SwapChainDesc swapchain_desc;
        swapchain_desc.ColorBufferFormat =
            to_texture_format(to_color_space(PixelFormat::r8g8b8a8_unorm_srgb, color_space));
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
            m_device->CreateBuffer(desc, nullptr, &m_constants.instance);
        }

        {
            BufferDesc desc;
            desc.Name           = "VS View Constants";
            desc.Size           = sizeof(ViewConstantBuffer);
            desc.Usage          = USAGE_DYNAMIC;
            desc.BindFlags      = BIND_UNIFORM_BUFFER;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, nullptr, &m_constants.view);
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
        RefCntPtr<ShaderStreamFactory> factory(MakeNewRCObj<ShaderStreamFactory>()(loader));

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

    [[nodiscard]] std::unique_ptr<khepri::renderer::Material>
    create_material(const khepri::renderer::MaterialDesc& material_desc)
    {
        const auto& update_max_light_count = [this](const Material& mat) {
            m_max_directional_light_count =
                std::max(m_max_directional_light_count,
                         static_cast<unsigned int>(mat.num_directional_lights()));
            m_max_point_light_count = std::max(m_max_point_light_count,
                                               static_cast<unsigned int>(mat.num_point_lights()));
        };

        auto material =
            std::make_unique<Material>(*m_device, *m_swapchain, material_desc, [=](auto& mat) {
                // Remove the destroyed material from the alive list, and update the max light count
                // in the process.
                m_max_directional_light_count = 0;
                m_max_point_light_count       = 0;
                for (std::size_t i = 0; i < m_alive_materials.size(); ++i) {
                    if (m_alive_materials[i] == &mat) {
                        std::swap(m_alive_materials[i], m_alive_materials.back());
                        m_alive_materials.resize(m_alive_materials.size() - 1);
                    }
                    update_max_light_count(*m_alive_materials[i]);
                }
            });

        // Set all existing render passes on the material
        for (GlobalRenderPassIndex i = 0; i < m_render_passes.size(); ++i) {
            if (m_render_passes[i]) {
                material->set_render_pass(i, *m_render_passes[i], m_constants);
            }
        }

        m_alive_materials.push_back(material.get());
        update_max_light_count(*material);

        // The max number of lights might have grown, check if we should re-allocate the buffer.
        if (m_max_directional_light_count > 0 &&
            (!m_constants.directional_lights ||
             m_max_directional_light_count >
                 m_constants.directional_lights->GetDesc().Size / sizeof(DirectionalLight))) {
            // Re-allocate the directional light buffer
            BufferDesc desc;
            desc.Name           = "Directional Lights";
            desc.Size           = sizeof(DirectionalLight) * m_max_directional_light_count;
            desc.Usage          = USAGE_DYNAMIC;
            desc.BindFlags      = BIND_UNIFORM_BUFFER;
            desc.CPUAccessFlags = CPU_ACCESS_WRITE;
            m_device->CreateBuffer(desc, nullptr, &m_constants.directional_lights);
            assert(m_constants.directional_lights != nullptr);

            fill_directional_light_buffer(*m_constants.directional_lights,
                                          m_dynamic_light_desc.directional_lights);
        }

        return std::move(material);
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

        TextureData texdata;
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
        RefCntPtr<ISampler> sampler;
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

    std::unique_ptr<RenderPipeline>
    create_render_pipeline(const RenderPipelineDesc& render_pipeline_desc)
    {
        // Store the render passes and get their global IDs
        const auto render_pass_indices = store_render_passes(render_pipeline_desc.render_passes);

        const auto on_pipeline_destroyed = [this](RenderPipeline& pipeline) {
            // Update all alive materials to clear the render passes.
            // Note: this is safe to do even if the render pass wasn't set yet (it's just ignored).
            const auto& render_pass_indices = pipeline.render_pass_indices();
            for (auto* const material : m_alive_materials) {
                for (const auto render_pass_index : render_pass_indices) {
                    material->clear_render_pass(render_pass_index);
                }
            }
            // Free the pipeline's render passes
            remove_render_passes(render_pass_indices);
        };

        // Create the pipeline and its resources
        auto pipeline = [&] {
            try {
                return std::make_unique<RenderPipeline>(render_pass_indices, on_pipeline_destroyed);
            } catch (...) {
                // Something went wrong, free the allocated indices
                remove_render_passes(render_pass_indices);
                throw;
            }
        }();

        // Update all alive materials with the new render passes.
        // This will create new graphics pipelines for those render pass/material combinations.
        for (auto* const material : m_alive_materials) {
            for (std::size_t i = 0; i < render_pass_indices.size(); ++i) {
                material->set_render_pass(render_pass_indices[i],
                                          render_pipeline_desc.render_passes[i], m_constants);
            }
        }

        return std::move(pipeline);
    }

    void set_dynamic_lights(const DynamicLightDesc& light_desc)
    {
        m_dynamic_light_desc = light_desc;

        if (m_constants.directional_lights) {
            // Update the light buffer, if we have any
            fill_directional_light_buffer(*m_constants.directional_lights,
                                          m_dynamic_light_desc.directional_lights);
        }
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

    void render_meshes(const khepri::renderer::RenderPipeline& render_pipeline,
                       gsl::span<const MeshInstance> meshes, const Camera& camera)
    {
        // Validate the input first
        const auto* const pipeline = dynamic_cast<const RenderPipeline*>(&render_pipeline);
        if (pipeline == nullptr) {
            throw ArgumentError();
        }

        for (const auto& mesh_info : meshes) {
            const auto* const material = dynamic_cast<const Material*>(mesh_info.material);
            const auto* const mesh     = dynamic_cast<const Mesh*>(mesh_info.mesh);
            if (material == nullptr || mesh == nullptr) {
                throw ArgumentError();
            }
        }

        // Set the view-specific constants
        const auto& camera_matrices = camera.matrices();
        {
            MapHelper<ViewConstantBuffer> constants(m_context, m_constants.view, MAP_WRITE,
                                                    MAP_FLAG_DISCARD);
            constants->view          = camera_matrices.view;
            constants->view_proj     = camera_matrices.view_proj;
            constants->view_proj_inv = camera_matrices.view_proj_inv;
        }

        // Returns the mesh's distance 'in front of' the camera
        const auto& get_view_distance = [&](const MeshInstance& mesh_info) {
            // Note that view space's Z axis (pointing from the camera into the scene) is the
            // *negative* Z axis. So we invert Z to get the the view distance, where larger is
            // further away.
            return -(mesh_info.transform.get_translation() * camera.matrices().view_proj).z;
        };

        // Space to filter and sort meshes per render pass. Reserved once and reused.
        std::vector<const MeshInstance*> render_pass_meshes;
        render_pass_meshes.reserve(meshes.size());

        // Execute all render passes, in order
        for (const auto render_pass_index : pipeline->render_pass_indices()) {
            // Collect the meshes for this render pass
            render_pass_meshes.clear();
            for (const auto& mesh_info : meshes) {
                const auto* const material = static_cast<const Material*>(mesh_info.material);
                if (material->is_used(render_pass_index)) {
                    render_pass_meshes.push_back(&mesh_info);
                }
            }

            // Depth-sort the meshes if needed.
            switch (m_render_passes[render_pass_index]->depth_sorting) {
            default:
                assert(false);
                // Fall-through
            case RenderPassDesc::DepthSorting::none:
                // Nothing to do
                break;
            case RenderPassDesc::DepthSorting::front_to_back:
                // Smaller distance gets rendered first
                std::sort(render_pass_meshes.begin(), render_pass_meshes.end(),
                          [&](const MeshInstance* lhs, const MeshInstance* rhs) {
                              return get_view_distance(*lhs) < get_view_distance(*rhs);
                          });
                break;
            case RenderPassDesc::DepthSorting::back_to_front:
                // Larger distance gets rendered first
                std::sort(render_pass_meshes.begin(), render_pass_meshes.end(),
                          [&](const MeshInstance* lhs, const MeshInstance* rhs) {
                              return get_view_distance(*lhs) > get_view_distance(*rhs);
                          });
                break;
            }

            // Now render the meshes in order
            for (const auto* mesh_info : render_pass_meshes) {
                auto* const material = static_cast<const Material*>(mesh_info->material);
                auto* const mesh     = static_cast<const Mesh*>(mesh_info->mesh);

                assert(material->is_used(render_pass_index));
                material->set_active(render_pass_index, *m_context, mesh_info->material_params,
                                     *m_constants.directional_lights);

                std::array<IBuffer*, 1> vertex_buffers{mesh->vertex_buffer};
                m_context->SetVertexBuffers(
                    0, static_cast<Uint32>(vertex_buffers.size()), vertex_buffers.data(), nullptr,
                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
                m_context->SetIndexBuffer(mesh->index_buffer, 0,
                                          RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                // Set instance-specific constants
                {
                    MapHelper<InstanceConstantBuffer> constants(m_context, m_constants.instance,
                                                                MAP_WRITE, MAP_FLAG_DISCARD);
                    constants->world     = mesh_info->transform;
                    constants->world_inv = inverse(mesh_info->transform);
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
    }

    void render_sprites(const khepri::renderer::RenderPipeline& render_pipeline,
                        gsl::span<const Sprite> sprites, const khepri::renderer::Material& material,
                        gsl::span<const khepri::renderer::Material::Param> params)
    {
        const auto* const pipeline = dynamic_cast<const RenderPipeline*>(&render_pipeline);
        if (!pipeline) {
            throw ArgumentError();
        }

        auto* mat = dynamic_cast<const Material*>(&material);
        if (mat == nullptr) {
            throw ArgumentError();
        }

        // Execute all render passes, in order
        for (const auto render_pass_index : pipeline->render_pass_indices()) {
            if (!mat->is_used(render_pass_index)) {
                // Nothing to do for this material in this render pass
                continue;
            }

            mat->set_active(render_pass_index, *m_context, params, *m_constants.directional_lights);

            std::size_t sprite_index = 0;
            while (sprite_index < sprites.size()) {
                const std::size_t sprites_left = sprites.size() - sprite_index;
                const std::size_t sprite_count = std::min(sprites_left, SPRITE_BUFFER_COUNT);

                {
                    // Copy the vertex data
                    MapHelper<SpriteVertex> vertices_map(m_context, m_sprite_vertex_buffer,
                                                         MAP_WRITE, MAP_FLAG_DISCARD);

                    const auto vertices = gsl::span<SpriteVertex>(
                        vertices_map,
                        m_sprite_vertex_buffer->GetDesc().Size / sizeof(SpriteVertex));

                    for (std::size_t i = 0; i < sprite_count * VERTICES_PER_SPRITE;
                         i += VERTICES_PER_SPRITE, ++sprite_index) {
                        const auto& sprite = sprites[sprite_index];
                        vertices[i + 0].position =
                            Vector3f(sprite.position_top_left.x, sprite.position_top_left.y, 0);
                        vertices[i + 1].position =
                            Vector3f(sprite.position_bottom_right.x, sprite.position_top_left.y, 0);
                        vertices[i + 2].position = Vector3f(sprite.position_bottom_right.x,
                                                            sprite.position_bottom_right.y, 0);
                        vertices[i + 3].position =
                            Vector3f(sprite.position_top_left.x, sprite.position_bottom_right.y, 0);
                        vertices[i + 0].uv = Vector2f(sprite.uv_top_left.x, sprite.uv_top_left.y);
                        vertices[i + 1].uv =
                            Vector2f(sprite.uv_bottom_right.x, sprite.uv_top_left.y);
                        vertices[i + 2].uv =
                            Vector2f(sprite.uv_bottom_right.x, sprite.uv_bottom_right.y);
                        vertices[i + 3].uv =
                            Vector2f(sprite.uv_top_left.x, sprite.uv_bottom_right.y);
                    }
                }

                m_context->SetVertexBuffers(0, 1, &m_sprite_vertex_buffer, nullptr,
                                            RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                            SET_VERTEX_BUFFERS_FLAG_RESET);

                m_context->SetIndexBuffer(m_sprite_index_buffer, 0,
                                          RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                static_assert(sizeof(Mesh::Index) == sizeof(std::uint16_t));
                DrawIndexedAttribs draw_attribs;
                draw_attribs.NumIndices = static_cast<Uint32>(sprite_count * TRIANGLES_PER_SPRITE *
                                                              VERTICES_PER_TRIANGLE);
                draw_attribs.IndexType  = VT_UINT16;
#ifndef NDEBUG
                draw_attribs.Flags = DRAW_FLAG_VERIFY_ALL;
#endif
                m_context->DrawIndexed(draw_attribs);
            }
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

    RefCntPtr<IShader> create_shader_object(const std::string& path, ShaderStreamFactory& factory,
                                            SHADER_TYPE shader_type, const std::string& entrypoint)
    {
        RefCntPtr<IDataBlob>                  compiler_output;
        RefCntPtr<IHLSL2GLSLConversionStream> conversion_stream;

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

        RefCntPtr<IShader> shader;
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

    static void print_converted_glsl_source(RefCntPtr<IHLSL2GLSLConversionStream> conversion_stream,
                                            const std::string& path, ShaderStreamFactory& factory,
                                            SHADER_TYPE shader_type, const std::string& entrypoint)
    {
        if (conversion_stream == nullptr) {
            // If we didn't get a conversion stream, try to create one ourselves
            RefCntPtr<IHLSL2GLSLConverter> converter;
            CreateHLSL2GLSLConverter(&converter);
            converter->CreateStream(path.c_str(), &factory, nullptr, 0, &conversion_stream);
        }

        RefCntPtr<IDataBlob> glsl_source;
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

    static std::vector<std::string>
    determine_dynamic_material_variables(const Shader&                              shader,
                                         const std::vector<MaterialDesc::Property>& properties)
    {
        // Any top-level material property with the same name as these is an error
        const std::unordered_map<std::string, SHADER_RESOURCE_TYPE> predefined_variables{
            {"InstanceConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
            {"ViewConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
            {"Material", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
            {"DirectionalLightConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
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
            if (!std::holds_alternative<const khepri::renderer::Texture*>(p.default_value)) {
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
                // Don't make this a fatal error (i.e. don't throw), because this can happen during
                // normal development flow when shader code is temporarily commented out during
                // development and the variable is optimized away.
                LOG.error("missing shader variable for property \"{}\"", p.name);
                continue;
            }
            if (it->second != SHADER_RESOURCE_TYPE_TEXTURE_SRV) {
                LOG.error("mismatch for shader variable type for property \"{}\"", p.name);
                throw ArgumentError();
            }
            // We've seen this one, remove it
            shader_variables.erase(it);

            dynamic_variables.push_back(p.name);
        }

        // These predefined variables can change
        dynamic_variables.emplace_back("Material");
        dynamic_variables.emplace_back("DirectionalLightConstants");

        if (!shader_variables.empty()) {
            // Not all shader variables have been accounted for
            LOG.error("missing material property for shader variable \"{}\"",
                      shader_variables.begin()->first);
            throw ArgumentError();
        }
        return dynamic_variables;
    }

    std::vector<GlobalRenderPassIndex>
    store_render_passes(gsl::span<const RenderPassDesc> render_passes)
    {
        std::vector<GlobalRenderPassIndex> indices;
        indices.reserve(render_passes.size());
        for (const auto& render_pass : render_passes) {
            std::size_t index;
            if (!m_unused_render_pass_indices.empty()) {
                index = m_unused_render_pass_indices.top();
                m_unused_render_pass_indices.pop();
            } else {
                index = m_next_render_pass_index++;
                m_render_passes.push_back({});
            }
            m_render_passes[index] = render_pass;
            indices.push_back(index);
        }
        return indices;
    }

    void remove_render_passes(gsl::span<const GlobalRenderPassIndex> indices)
    {
        for (const auto& index : indices) {
            m_render_passes[index] = {};
            m_unused_render_pass_indices.push(index);
        }
    }

    void fill_directional_light_buffer(IBuffer&                              buffer,
                                       gsl::span<const DirectionalLightDesc> lights) const
    {
        const std::size_t buffer_count   = buffer.GetDesc().Size / sizeof(DirectionalLight);
        const std::size_t lights_to_copy = std::min(lights.size(), buffer_count);

        MapHelper<DirectionalLight> constants(m_context, &buffer, MAP_WRITE, MAP_FLAG_DISCARD);

        // Fill the buffer with as many lights as we have
        std::size_t i = 0;
        for (; i < lights_to_copy; ++i) {
            const auto& light          = lights[i];
            constants[i].direction     = Vector3f(normalize(light.direction));
            constants[i].intensity     = static_cast<float>(light.intensity);
            constants[i].diffuse_color = Vector3f(
                Vector3(light.diffuse_color.r, light.diffuse_color.g, light.diffuse_color.b));
            constants[i].specular_color = Vector3f(
                Vector3(light.specular_color.r, light.specular_color.g, light.specular_color.b));
        }

        // Clear the rest of the buffer (if we have fewer lights)
        for (; i < buffer_count; ++i) {
            constants[i] = {{0, 0, -1}, 0, {0, 0, 0}, {0, 0, 0}};
        }
    }

    RefCntPtr<IRenderDevice>  m_device;
    RefCntPtr<IDeviceContext> m_context;
    RefCntPtr<ISwapChain>     m_swapchain;

    ConstantsBuffers m_constants;

    RefCntPtr<IBuffer> m_sprite_vertex_buffer;
    RefCntPtr<IBuffer> m_sprite_index_buffer;

    // This is a non-owning set of all alive materials.
    // This is necessary for when new render pipelines are created. When that happens, all alive
    // materials need to be updated with newly construct graphics pipelines for the new render
    // pipeline's render passes.
    std::vector<Material*> m_alive_materials;

    // Maximum count of lights in the alive materials
    unsigned int m_max_directional_light_count{0};
    unsigned int m_max_point_light_count{0};

    // Freed and reusable global render pass indices
    std::stack<GlobalRenderPassIndex> m_unused_render_pass_indices;
    // Next available render pass index (after reusing the unused ones)
    GlobalRenderPassIndex m_next_render_pass_index{0};
    // Global list of render passes. Indexed by GlobalRenderPassIndex
    std::vector<std::optional<RenderPassDesc>> m_render_passes;

    // Currently active dynamic lighting
    DynamicLightDesc m_dynamic_light_desc{};
};

Renderer::Renderer(const std::any& window, ColorSpace color_space)
    : m_impl(std::make_unique<Impl>(window, color_space))
{
}

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

std::unique_ptr<khepri::renderer::Material>
Renderer::create_material(const MaterialDesc& material_desc)
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

std::unique_ptr<RenderPipeline>
Renderer::create_render_pipeline(const RenderPipelineDesc& render_pipeline_desc)
{
    return m_impl->create_render_pipeline(render_pipeline_desc);
}

void Renderer::set_dynamic_lights(const DynamicLightDesc& light_desc)
{
    m_impl->set_dynamic_lights(light_desc);
}

void Renderer::clear(ClearFlags flags)
{
    m_impl->clear(flags);
}

void Renderer::present()
{
    m_impl->present();
}

void Renderer::render_meshes(const khepri::renderer::RenderPipeline& render_pipeline,
                             gsl::span<const MeshInstance> meshes, const Camera& camera)
{
    m_impl->render_meshes(render_pipeline, meshes, camera);
}

void Renderer::render_sprites(const khepri::renderer::RenderPipeline& render_pipeline,
                              gsl::span<const Sprite> sprites, const Material& material,
                              gsl::span<const Material::Param> params)
{
    m_impl->render_sprites(render_pipeline, sprites, material, params);
}

} // namespace khepri::renderer::diligent
