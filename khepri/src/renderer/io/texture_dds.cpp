#include <khepri/io/exceptions.hpp>
#include <khepri/io/stream.hpp>
#include <khepri/renderer/io/texture.hpp>
#include <khepri/renderer/texture_desc.hpp>

#include <gsl/gsl-lite.hpp>

#include <algorithm>
#include <cassert>
#include <optional>

namespace khepri::renderer::io {
namespace {

using khepri::renderer::PixelFormat;

constexpr auto BITS_PER_BYTE = 8UL;

enum : std::uint32_t
{
    ddsf_caps        = 1,
    ddsf_height      = 2,
    ddsf_width       = 4,
    ddsf_pitch       = 8,
    ddsf_pixelformat = 0x1000,
    ddsf_mipmapcount = 0x20000,
    ddsf_linearsize  = 0x80000,
    ddsf_depth       = 0x800000,
};

enum : std::uint32_t
{
    ddscaps2_cubemap           = 0x200,
    ddscaps2_cubemap_positivex = 0x400,
    ddscaps2_cubemap_negativex = 0x800,
    ddscaps2_cubemap_positivey = 0x1000,
    ddscaps2_cubemap_negativey = 0x2000,
    ddscaps2_cubemap_positivez = 0x4000,
    ddscaps2_cubemap_negativez = 0x8000,
    ddscaps2_volume            = 0x200000,
};

enum : std::uint32_t
{
    ddpf_alphapixels     = 1,
    ddpf_alpha           = 2,
    ddpf_fourcc          = 4,
    ddpf_paletteindexed8 = 0x20,
    ddpf_rgb             = 0x40,
    ddpf_yuv             = 0x200,
    ddpf_luminance       = 0x20000,
    ddpf_bumpdudv        = 0x00080000,
};

struct DdsPixelFormat
{
    unsigned long flags;
    unsigned long fourcc;
    unsigned long rgb_bitcount;
    unsigned long r_mask;
    unsigned long g_mask;
    unsigned long b_mask;
    unsigned long a_mask;
};

constexpr unsigned long DDS_MAGIC            = 0x20534444; // "DDS " in little endian
constexpr unsigned long DDS_HEADER_SIZE      = 124;
constexpr unsigned long DDS_PIXELFORMAT_SIZE = 32;
constexpr unsigned long DDS_REQUIRED_FLAGS =
    ddsf_caps | ddsf_height | ddsf_width | ddsf_pixelformat;

void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

// Divides value by divisor, rounding the result up to the next integer
template <typename T>
constexpr T round_up(T value, T divisor)
{
    return (value + divisor - 1) / divisor;
}

constexpr std::uint32_t fourcc(char ch0, char ch1, char ch2, char ch3) noexcept
{
    return (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch0)) << (BITS_PER_BYTE * 0)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch1)) << (BITS_PER_BYTE * 1)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch2)) << (BITS_PER_BYTE * 2)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch3)) << (BITS_PER_BYTE * 3));
}

class PixelFormatHandler
{
public:
    explicit PixelFormatHandler(PixelFormat output_format) : m_output_format(output_format) {}

    PixelFormatHandler(const PixelFormatHandler&)                = delete;
    PixelFormatHandler(PixelFormatHandler&&) noexcept            = default;
    PixelFormatHandler& operator=(const PixelFormatHandler&)     = delete;
    PixelFormatHandler& operator=(PixelFormatHandler&&) noexcept = default;

    virtual ~PixelFormatHandler() = default;

    [[nodiscard]] PixelFormat output_format() const noexcept
    {
        return m_output_format;
    }

    [[nodiscard]] virtual TextureDesc::Subresource
    create_subresource(std::size_t mip_level, unsigned long mip_width,
                       unsigned long mip_height) const noexcept = 0;

    [[nodiscard]] virtual std::vector<std::uint8_t>
    read_pixel_data(khepri::io::Stream&                       stream,
                    gsl::span<const TextureDesc::Subresource> subresources) const
    {
        std::size_t data_size = 0;
        for (const auto& subresource : subresources) {
            data_size += subresource.data_size;
        }

        std::vector<std::uint8_t> data(data_size);
        if (stream.read(data.data(), data_size) != data_size) {
            throw khepri::io::InvalidFormatError();
        }
        return data;
    }

private:
    PixelFormat m_output_format;
};

// Handler for BC1, BC2, BC3. Also known as DXT1-DXT5.
class BlockCompressionPixelFormatHandler : public PixelFormatHandler
{
public:
    using PixelFormatHandler::PixelFormatHandler;

    [[nodiscard]] TextureDesc::Subresource
    create_subresource(std::size_t /*mip_level*/, unsigned long mip_width,
                       unsigned long mip_height) const noexcept override
    {
        // Block compression
        const std::size_t bpe =
            (output_format() == PixelFormat::bc1_unorm_srgb) ? BITS_PER_BYTE : 2 * BITS_PER_BYTE;
        const auto blocks_w = (mip_width > 0) ? std::max(1UL, round_up(mip_width, 4UL)) : 0;
        const auto blocks_h = (mip_height > 0) ? std::max(1UL, round_up(mip_height, 4UL)) : 0;

        TextureDesc::Subresource subresource{};
        subresource.stride       = blocks_w * bpe;
        subresource.depth_stride = subresource.stride * blocks_h;
        return subresource;
    }
};

// Handler for 32-bit RGBA formats
//
// \tparam Swizzle If true, the RGB pixel data is swizzled around (i.e. red and blue are flipped).
template <bool Swizzle>
class RGBA32PixelFormatHandler : public PixelFormatHandler
{
public:
    using PixelFormatHandler::PixelFormatHandler;

    [[nodiscard]] TextureDesc::Subresource
    create_subresource(std::size_t /*mip_level*/, unsigned long mip_width,
                       unsigned long mip_height) const noexcept override
    {
        constexpr auto bpp = 32;

        TextureDesc::Subresource subresource{};
        subresource.stride       = round_up(mip_width * bpp, 8UL); // Round up to nearest byte
        subresource.depth_stride = subresource.stride * mip_height;
        return subresource;
    }

    [[nodiscard]] std::vector<std::uint8_t>
    read_pixel_data(khepri::io::Stream&                       stream,
                    gsl::span<const TextureDesc::Subresource> subresources) const override
    {
        auto data = PixelFormatHandler::read_pixel_data(stream, subresources);
        if constexpr (Swizzle) {
            for (std::size_t i = 0; i < data.size(); i += 4) {
                std::swap(data[i + 0], data[i + 2]);
            }
        }
        return data;
    }
};

// Handler for 24-bit RGB formats.
// Since 24-bits RGB formats are not supported by the renderer, such textures must be converted to
// 32-bit RGBA formats.
//
// \tparam Swizzle If true, the RGB pixel data is swizzled around (i.e. red and blue are flipped).
template <bool Swizzle>
class RGB24PixelFormatHandler : public PixelFormatHandler
{
public:
    using PixelFormatHandler::PixelFormatHandler;

    [[nodiscard]] TextureDesc::Subresource
    create_subresource(std::size_t /*mip_level*/, unsigned long mip_width,
                       unsigned long mip_height) const noexcept override
    {
        constexpr auto bpp = 32;

        TextureDesc::Subresource subresource{};
        subresource.stride       = round_up(mip_width * bpp, 8UL); // Round up to nearest byte
        subresource.depth_stride = subresource.stride * mip_height;
        return subresource;
    }

    [[nodiscard]] std::vector<std::uint8_t>
    read_pixel_data(khepri::io::Stream&                       stream,
                    gsl::span<const TextureDesc::Subresource> subresources) const override
    {
        std::size_t output_data_size = 0;
        for (const auto& subresource : subresources) {
            output_data_size += subresource.data_size;
        }
        const std::size_t input_data_size = output_data_size / 4 * 3;

        std::vector<std::uint8_t> input_data(input_data_size);
        if (stream.read(input_data.data(), input_data_size) != input_data_size) {
            throw khepri::io::InvalidFormatError();
        }

        std::vector<std::uint8_t> output_data(output_data_size);
        for (std::size_t i = 0, o = 0; i < input_data_size; i += 3, o += 4) {
            if constexpr (Swizzle) {
                output_data[o + 0] = input_data[i + 2];
                output_data[o + 1] = input_data[i + 1];
                output_data[o + 2] = input_data[i + 0];
            } else {
                output_data[o + 0] = input_data[i + 0];
                output_data[o + 1] = input_data[i + 1];
                output_data[o + 2] = input_data[i + 2];
            }
            // Set the alpha channel to max since the source didn't have it
            output_data[o + 3] = std::numeric_limits<std::uint8_t>::max();
        }

        return output_data;
    }
};

std::unique_ptr<PixelFormatHandler> pixel_format_handler(const DdsPixelFormat& ddpf)
{
    if ((ddpf.flags & ddpf_rgb) != 0) {
        constexpr auto RGBA_MASK_R = 0x000000ffUL;
        constexpr auto RGBA_MASK_G = 0x0000ff00UL;
        constexpr auto RGBA_MASK_B = 0x00ff0000UL;
        constexpr auto RGBA_MASK_A = 0xff000000UL;

        constexpr auto BGRA_MASK_R = 0x00ff0000UL;
        constexpr auto BGRA_MASK_G = 0x0000ff00UL;
        constexpr auto BGRA_MASK_B = 0x000000ffUL;
        constexpr auto BGRA_MASK_A = 0xff000000UL;

        switch (ddpf.rgb_bitcount) {
        case 24: {
            if (ddpf.r_mask == RGBA_MASK_R && ddpf.g_mask == RGBA_MASK_G &&
                ddpf.b_mask == RGBA_MASK_B && ddpf.a_mask == 0) {
                return std::make_unique<RGB24PixelFormatHandler<false>>(
                    PixelFormat::r8g8b8a8_unorm_srgb);
            }
            if (ddpf.r_mask == BGRA_MASK_R && ddpf.g_mask == BGRA_MASK_G &&
                ddpf.b_mask == BGRA_MASK_B && ddpf.a_mask == 0) {
                // b8g8r8a8_unorm_srgb, but swizzle it into r8g8b8a8_unorm_srgb, which is more
                // widely supported
                return std::make_unique<RGB24PixelFormatHandler<true>>(
                    PixelFormat::r8g8b8a8_unorm_srgb);
            }
            break;
        }
        case 32: {
            if (ddpf.r_mask == RGBA_MASK_R && ddpf.g_mask == RGBA_MASK_G &&
                ddpf.b_mask == RGBA_MASK_B && ddpf.a_mask == RGBA_MASK_A) {
                return std::make_unique<RGBA32PixelFormatHandler<false>>(
                    PixelFormat::r8g8b8a8_unorm_srgb);
            }
            if (ddpf.r_mask == BGRA_MASK_R && ddpf.g_mask == BGRA_MASK_G &&
                ddpf.b_mask == BGRA_MASK_B && ddpf.a_mask == BGRA_MASK_A) {
                // b8g8r8a8_unorm_srgb, but swizzle it into r8g8b8a8_unorm_srgb, which is more
                // widely supported
                return std::make_unique<RGBA32PixelFormatHandler<true>>(
                    PixelFormat::r8g8b8a8_unorm_srgb);
            }
            break;
        }
        default:
            break;
        }
    } else if ((ddpf.flags & ddpf_fourcc) != 0) {
        // Note: there is no distinction anymore between pre-multiplied and post-multiplied alpha,
        // so DXT2/3 and DXT4/5 are treated the same.
        switch (ddpf.fourcc) {
        case fourcc('D', 'X', 'T', '1'):
            return std::make_unique<BlockCompressionPixelFormatHandler>(
                PixelFormat::bc1_unorm_srgb);
        case fourcc('D', 'X', 'T', '2'):
        case fourcc('D', 'X', 'T', '3'):
            return std::make_unique<BlockCompressionPixelFormatHandler>(
                PixelFormat::bc2_unorm_srgb);
        case fourcc('D', 'X', 'T', '4'):
        case fourcc('D', 'X', 'T', '5'):
            return std::make_unique<BlockCompressionPixelFormatHandler>(
                PixelFormat::bc3_unorm_srgb);
        default:
            break;
        }
    }
    // Unsupported or unknown format
    return {};
}

} // namespace

bool is_texture_dds(khepri::io::Stream& stream)
{
    assert(stream.readable() && stream.seekable());
    try {
        const auto magic = stream.read_uint32();
        return magic == DDS_MAGIC;
    } catch (const khepri::io::Error&) {
        return false;
    }
}

auto create_subresources(unsigned long width, unsigned long height, unsigned long depth,
                         unsigned long mip_levels, const PixelFormatHandler& pixel_format_handler)
{
    size_t data_offset = 0;
    auto   mip_width   = width;
    auto   mip_height  = height;
    auto   mip_depth   = depth;

    std::vector<TextureDesc::Subresource> subresources(mip_levels);
    for (size_t mip = 0; mip < mip_levels; ++mip) {
        auto subresource = pixel_format_handler.create_subresource(mip, mip_width, mip_height);
        subresource.data_offset = data_offset;
        subresource.data_size   = subresource.depth_stride * mip_depth;

        subresources[mip] = subresource;

        data_offset += subresource.data_size;
        mip_width  = std::max(1UL, mip_width / 2UL);
        mip_height = std::max(1UL, mip_height / 2UL);
        mip_depth  = std::max(1UL, mip_depth / 2UL);
    }
    return subresources;
}

TextureDesc load_texture_dds(khepri::io::Stream& stream, const TextureLoadOptions& options)
{
    assert(stream.readable() && stream.seekable());
    TextureDimension dimension = TextureDimension::texture_2d;

    auto magic = stream.read_uint32();
    verify(magic == DDS_MAGIC);

    auto size   = stream.read_uint32();
    auto flags  = stream.read_uint32();
    auto height = stream.read_uint32();
    auto width  = stream.read_uint32();

    // Ignore pitch/linear size, it's unreliable. We must calculate it ourselves.
    stream.read_uint32();

    auto depth = stream.read_uint32();
    if ((flags & ddsf_depth) != 0) {
        // 3D (volume) texture
        depth     = std::max((uint32_t)1, depth);
        dimension = TextureDimension::texture_3d;
    } else {
        depth = 1;
    }

    auto mip_levels = stream.read_uint32();
    if ((flags & ddsf_mipmapcount) != 0) {
        mip_levels = std::max((uint32_t)1, mip_levels);
    } else {
        mip_levels = 1;
    }

    // Reserved data
    constexpr auto num_reserved_uints = 11;
    for (int i = 0; i < num_reserved_uints; ++i) {
        stream.read_uint32();
    }

    // Pixel format
    auto           pf_size = stream.read_uint32();
    DdsPixelFormat ddpf{};
    ddpf.flags        = stream.read_uint32();
    ddpf.fourcc       = stream.read_uint32();
    ddpf.rgb_bitcount = stream.read_uint32();
    ddpf.r_mask       = stream.read_uint32();
    ddpf.g_mask       = stream.read_uint32();
    ddpf.b_mask       = stream.read_uint32();
    ddpf.a_mask       = stream.read_uint32();

    stream.read_uint32(); // Caps. Ignored, contains no relevant data.
    auto caps2 = stream.read_uint32();
    stream.read_uint32(); // Caps3. Unused
    stream.read_uint32(); // Caps4. Unused.
    stream.read_uint32(); // Reserved

    verify(size == DDS_HEADER_SIZE);
    verify((flags & DDS_REQUIRED_FLAGS) == DDS_REQUIRED_FLAGS);
    verify(pf_size == DDS_PIXELFORMAT_SIZE);

    const bool dx10_header =
        (((ddpf.flags & ddpf_fourcc) != 0) && (ddpf.fourcc == fourcc('D', 'X', '1', '0')));
    if (dx10_header) {
        // DX10 extensions, unsupported as of yet
        throw khepri::io::InvalidFormatError();
    }

    if ((caps2 & ddscaps2_cubemap) != 0) {
        // Cubemap texture, unsupported as of yet
        throw khepri::io::InvalidFormatError();
    }

    auto format_handler = pixel_format_handler(ddpf);
    if (!format_handler) {
        // Unsupported or unknown format
        throw khepri::io::InvalidFormatError();
    }

    if (width == 0 || height == 0) {
        // Invalid format
        throw khepri::io::InvalidFormatError();
    }

    auto subresources = create_subresources(width, height, depth, mip_levels, *format_handler);

    auto data = format_handler->read_pixel_data(stream, subresources);

    const unsigned long depth_array_size = (dimension == TextureDimension::texture_3d) ? depth : 0;

    auto pixel_format = format_handler->output_format();
    if (!dx10_header) {
        // Without a DX10 header, we don't know the color space, so we use the default from the
        // provided options.
        pixel_format = to_color_space(pixel_format, options.default_color_space);
    }

    return {dimension,
            width,
            height,
            depth_array_size,
            mip_levels,
            pixel_format,
            std::move(subresources),
            std::move(data)};
}

} // namespace khepri::renderer::io
