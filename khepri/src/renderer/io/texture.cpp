#include <khepri/io/exceptions.hpp>
#include <khepri/renderer/io/texture.hpp>

namespace khepri::renderer::io {

// DirectDraw surface
extern bool        is_texture_dds(khepri::io::Stream& stream);
extern TextureDesc load_texture_dds(khepri::io::Stream& stream);

// TrueVision TGA
extern bool        is_texture_tga(khepri::io::Stream& stream);
extern TextureDesc load_texture_tga(khepri::io::Stream& stream);
extern void        save_texture_tga(khepri::io::Stream& stream, const TextureDesc& texture_desc,
                                    const TextureSaveOptions& options);

namespace {

struct FormatFuncs
{
    // Checks if the stream contains this format
    bool (*check_func)(khepri::io::Stream&);

    // Loads a texture in this format from stream
    TextureDesc (*load_func)(khepri::io::Stream&);
};

} // namespace

TextureDesc load_texture(khepri::io::Stream& stream)
{
    if (!stream.readable() || !stream.seekable()) {
        throw ArgumentError();
    }

    static const std::array<FormatFuncs, 2> formats = {
        {{is_texture_dds, load_texture_dds}, {is_texture_tga, load_texture_tga}}};
    // Check each supported format
    for (const auto& format : formats) {
        stream.seek(0, khepri::io::SeekOrigin::begin);
        if (format.check_func(stream)) {
            stream.seek(0, khepri::io::SeekOrigin::begin);
            return format.load_func(stream);
        }
    }
    throw khepri::io::InvalidFormatError();
}

void save_texture(khepri::io::Stream& stream, const TextureDesc& texture_desc,
                  const TextureSaveOptions& options)
{
    if (!stream.writable()) {
        throw ArgumentError();
    }

    switch (options.format) {
    case TextureFormat::targa:
        save_texture_tga(stream, texture_desc, options);
        break;
    default:
        throw ArgumentError();
    }
}

} // namespace khepri::renderer::io
