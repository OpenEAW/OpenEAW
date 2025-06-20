#pragma once

#include <khepri/renderer/renderer.hpp>

#include <ObjectBase.hpp>
#include <Shader.h>
#include <cassert>
#include <iterator>

namespace khepri::renderer::diligent {

class ShaderStreamFactory final
    : public Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>
{
    using IObject      = Diligent::IObject;
    using INTERFACE_ID = Diligent::INTERFACE_ID;

    class MemoryStream final : public Diligent::ObjectBase<Diligent::IFileStream>
    {
    public:
        MemoryStream(Diligent::IReferenceCounters* pRefCounters, ShaderDesc shader)
            : Diligent::ObjectBase<Diligent::IFileStream>(pRefCounters), m_shader(std::move(shader))
        {
        }

        void DILIGENT_CALL_TYPE ReadBlob(Diligent::IDataBlob* data) override
        {
            assert(data != nullptr);
            assert(m_pos == 0);
            const auto& source_data = m_shader.data();
            data->Resize(source_data.size());
            std::copy(source_data.begin(), source_data.end(),
                      static_cast<uint8_t*>(data->GetDataPtr()));
        }

        bool DILIGENT_CALL_TYPE Read(void* data, size_t size) override
        {
            const auto& source_data = m_shader.data();
            auto        read        = std::min(source_data.size() - m_pos, size);
            std::copy(&source_data[m_pos], &source_data[m_pos + read], static_cast<uint8_t*>(data));
            m_pos += read;
            return read == size;
        }

        bool DILIGENT_CALL_TYPE Write(const void* /*data*/, size_t /*size*/) override
        {
            // Not supported
            return false;
        }

        size_t DILIGENT_CALL_TYPE GetSize() override
        {
            return m_shader.data().size();
        }

        bool DILIGENT_CALL_TYPE IsValid() override
        {
            return true;
        }

        IMPLEMENT_QUERY_INTERFACE_IN_PLACE(Diligent::IID_FileStream,
                                           Diligent::ObjectBase<Diligent::IFileStream>);

    private:
        ShaderDesc m_shader;
        size_t     m_pos = 0;
    };

public:
    explicit ShaderStreamFactory(Diligent::IReferenceCounters* pRefCounters,
                                 Renderer::ShaderLoader        loader)
        : Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>(pRefCounters)
        , m_loader(std::move(loader))
    {
    }

    void DILIGENT_CALL_TYPE CreateInputStream(const char*             name,
                                              Diligent::IFileStream** ppStream) final
    {
        CreateInputStream2(name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_NONE, ppStream);
    }

    void DILIGENT_CALL_TYPE CreateInputStream2(
        const char*             Name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAGS /*Flags*/,
        Diligent::IFileStream** ppStream) final
    {
        assert(ppStream != nullptr);
        auto shader = m_loader(Name);
        *ppStream = (shader) ? Diligent::MakeNewRCObj<MemoryStream>()(std::move(*shader)) : nullptr;
    }

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(
        Diligent::IID_IShaderSourceInputStreamFactory,
        Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>);

private:
    Renderer::ShaderLoader m_loader;
};

} // namespace khepri::renderer::diligent
