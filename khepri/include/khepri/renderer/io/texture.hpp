#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/texture_desc.hpp>

namespace khepri ::renderer ::io {

/**
 * Loads a texture description from a stream.
 *
 * Only the DDS and TARGA formats are supported by this function.
 *
 * @throw khepri::argument_error the stream is not readable and seekable.
 * @throw khepri::io::invalid_format the stream is not a valid texture.
 */
TextureDesc load_texture(khepri::io::Stream& stream);

/**
 * Possible texture formats for #save_texture
 */
enum TextureFormat
{
    /**
     * TrueVision TARGA.
     *
     * This format can only store 2D non-array textures up to 65535x65535 texels with pixel format
     * #PixelFormat::r8g8b8a8_unorm_srgb or #PixelFormat::b8g8r8a8_unorm_srgb.
     * Only the first mip level is stored.
     */
    targa,
};

/**
 * Options for saving a texture
 */
struct TextureSaveOptions
{
    /// The format the texture should be stored in
    TextureFormat format;
};

/**
 * Saves a texture description to a stream.
 *
 * @throw khepri::ArgumentError the stream is not writable.
 * @throw khepri::ArgumentError the texture description is invalid.
 * @throw khepri::ArgumentError the options are invalid.
 * @throw khepri::io::Error an error occured writing to the stream.
 *
 * Note that not all formats can write all kinds of texture descriptions. See #texture_format for
 * details. If a description is unsupported by a format, #khepri::argument_error is thrown.
 */
void save_texture(khepri::io::Stream& stream, const TextureDesc& texture_desc,
                  const TextureSaveOptions& options);

} // namespace khepri::renderer::io
