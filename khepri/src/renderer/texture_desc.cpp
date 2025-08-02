#include <khepri/renderer/texture_desc.hpp>

namespace khepri::renderer {
namespace {
template <typename T>
struct ColorTraits
{};

template <>
struct ColorTraits<ColorRGBA>
{
    static ColorRGBA from_r8g8b8a8(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                                   std::uint8_t a) noexcept
    {
        return {static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
                static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f};
    }

    static ColorRGBA from_float(float r, float g, float b, float a) noexcept
    {
        return {r, g, b, a};
    }
};

template <>
struct ColorTraits<ColorSRGBA>
{
    static ColorSRGBA from_r8g8b8a8(std::uint8_t r, std::uint8_t g, std::uint8_t b,
                                    std::uint8_t a) noexcept
    {
        return {r, g, b, a};
    }

    static ColorSRGBA from_float(float r, float g, float b, float a) noexcept
    {
        return {static_cast<std::uint8_t>(r * 255), static_cast<std::uint8_t>(g * 255),
                static_cast<std::uint8_t>(b * 255), static_cast<std::uint8_t>(a * 255)};
    }
};

// Converts a 16-bit (5:6:5) color to a ColorRGB type
ColorSRGB to_color(unsigned int value) noexcept
{
    const unsigned int r5 = (value >> 11) & 0x1F;
    const unsigned int g6 = (value >> 5) & 0x3F;
    const unsigned int b5 = value & 0x1F;

    return {static_cast<std::uint8_t>(r5 * 255 / 31), static_cast<std::uint8_t>(g6 * 255 / 63),
            static_cast<std::uint8_t>(b5 * 255 / 31)};
}

std::uint8_t lerp(std::uint8_t from, std::uint8_t to, unsigned int from_amount,
                  unsigned int total_amount)
{
    unsigned int to_amount = total_amount - from_amount;
    return static_cast<std::uint8_t>((static_cast<unsigned int>(from) * from_amount +
                                      static_cast<unsigned int>(to) * to_amount) /
                                     total_amount);
}

ColorSRGB lerp(const ColorSRGB& from, const ColorSRGB& to, unsigned int from_amount,
               unsigned int total_amount)
{
    return {lerp(from.r, to.r, from_amount, total_amount),
            lerp(from.g, to.g, from_amount, total_amount),
            lerp(from.b, to.b, from_amount, total_amount)};
}

// Reads 4 bytes and creates a BC (Block Compression) color palette
std::array<ColorSRGBA, 4> unpack_bc1_palette(const std::uint8_t* data)
{
    const auto c0      = (static_cast<unsigned int>(data[1]) << 8) | data[0];
    const auto c1      = (static_cast<unsigned int>(data[3]) << 8) | data[2];
    const auto color_0 = to_color(c0);
    const auto color_1 = to_color(c1);

    std::array<ColorSRGBA, 4> palette;
    palette[0] = {color_0, 255};
    palette[1] = {color_1, 255};
    if (c0 > c1) {
        // Four colors
        palette[2] = {lerp(color_0, color_1, 2, 3), 255};
        palette[3] = {lerp(color_0, color_1, 1, 3), 255};
    } else {
        // Three colors, and black transparent
        palette[2] = {lerp(color_0, color_1, 1, 2), 255};
        palette[3] = {0, 0, 0, 0};
    }
    return palette;
}

// Reads 2 bytes and creates a BC4 palette
std::array<std::uint8_t, 8> unpack_bc4_palette(const std::uint8_t* data)
{
    std::array<std::uint8_t, 8> palette;
    palette[0] = data[0];
    palette[1] = data[1];
    if (palette[0] > palette[1]) {
        palette[2] = lerp(palette[0], palette[1], 6, 7);
        palette[3] = lerp(palette[0], palette[1], 5, 7);
        palette[4] = lerp(palette[0], palette[1], 4, 7);
        palette[5] = lerp(palette[0], palette[1], 3, 7);
        palette[6] = lerp(palette[0], palette[1], 2, 7);
        palette[7] = lerp(palette[0], palette[1], 1, 7);
    } else {
        palette[2] = lerp(palette[0], palette[1], 4, 5);
        palette[3] = lerp(palette[0], palette[1], 3, 5);
        palette[4] = lerp(palette[0], palette[1], 2, 5);
        palette[5] = lerp(palette[0], palette[1], 1, 5);
        palette[6] = 0;
        palette[7] = 255;
    }
    return palette;
}

// Reads and unpacks 8 bytes of a BC1 encoded block, without external alpha
std::array<ColorSRGBA, 16> unpack_bc1_block(const std::uint8_t* data)
{
    const auto palette = unpack_bc1_palette(data);

    std::uint32_t x =
        (static_cast<std::uint32_t>(data[7]) << 24) | (static_cast<std::uint32_t>(data[6]) << 16) |
        (static_cast<std::uint32_t>(data[5]) << 8) | (static_cast<std::uint32_t>(data[4]) << 0);

    std::array<ColorSRGBA, 16> values;
    for (unsigned int i = 0; i < 16; ++i, x >>= 2) {
        values[i] = palette[x & 3];
    }
    return values;
}

// Reads and unpacks 8 bytes of a BC1 encoded block, with external alpha
std::array<ColorSRGBA, 16> unpack_bc1_block(const std::uint8_t*                 data,
                                            const std::array<std::uint8_t, 16>& alpha)
{
    const auto palette = unpack_bc1_palette(data);

    std::uint32_t x =
        (static_cast<std::uint32_t>(data[7]) << 24) | (static_cast<std::uint32_t>(data[6]) << 16) |
        (static_cast<std::uint32_t>(data[5]) << 8) | (static_cast<std::uint32_t>(data[4]) << 0);

    std::array<ColorSRGBA, 16> values;
    for (unsigned int i = 0; i < 16; ++i, x >>= 2) {
        const auto& c = palette[x & 3];
        values[i]     = {c.r, c.g, c.b, alpha[i]};
    }
    return values;
}

std::array<std::uint8_t, 16> unpack_bc2_alpha(const std::uint8_t* data)
{
    std::uint64_t x =
        (static_cast<std::uint64_t>(data[7]) << 56) | (static_cast<std::uint64_t>(data[6]) << 48) |
        (static_cast<std::uint64_t>(data[5]) << 40) | (static_cast<std::uint64_t>(data[4]) << 32) |
        (static_cast<std::uint64_t>(data[3]) << 24) | (static_cast<std::uint64_t>(data[2]) << 16) |
        (static_cast<std::uint64_t>(data[1]) << 8) | static_cast<std::uint64_t>(data[0]);

    std::array<std::uint8_t, 16> alpha;
    for (auto it = alpha.begin(); it != alpha.end(); ++it, x >>= 4) {
        *it = static_cast<std::uint8_t>((x & 0xF) * 255 / 15);
    }
    return alpha;
}

// Reads and unpacks 8 bytes of a BC4 encoded block
std::array<std::uint8_t, 16> unpack_bc4_block(const std::uint8_t* data)
{
    const auto palette = unpack_bc4_palette(data);

    std::uint64_t x =
        (static_cast<std::uint64_t>(data[7]) << 40) | (static_cast<std::uint64_t>(data[6]) << 32) |
        (static_cast<std::uint64_t>(data[5]) << 24) | (static_cast<std::uint64_t>(data[4]) << 16) |
        (static_cast<std::uint64_t>(data[3]) << 8) | static_cast<std::uint64_t>(data[2]);

    std::array<std::uint8_t, 16> values;
    for (unsigned int i = 0; i < 16; ++i, x >>= 3) {
        values[i] = palette[x & 7];
    }
    return values;
}

template <typename OutputT>
void copy_block(gsl::span<OutputT> pixels, unsigned long width, unsigned long height,
                unsigned long x, unsigned long y, const std::array<ColorSRGBA, 16>& values)
{
    auto num_rows = std::min(4LU, height - y);
    auto num_cols = std::min(4LU, width - x);
    auto dest_it  = pixels.begin() + y * width + x;
    auto src_it   = values.begin();
    for (auto row = 0LU; row < num_rows; ++row, src_it += 4, dest_it += width) {
        std::transform(src_it, src_it + num_cols, dest_it, [](const ColorSRGBA& c) {
            return ColorTraits<OutputT>::from_r8g8b8a8(c.r, c.g, c.b, c.a);
        });
    }
}

PixelFormat to_linear_space(PixelFormat format)
{
    switch (format) {
    case PixelFormat::r8g8b8a8_unorm_srgb:
        return PixelFormat::r8g8b8a8_unorm;
    case PixelFormat::b8g8r8a8_unorm_srgb:
        return PixelFormat::b8g8r8a8_unorm;
    case PixelFormat::bc1_unorm_srgb:
        return PixelFormat::bc1_unorm;
    case PixelFormat::bc2_unorm_srgb:
        return PixelFormat::bc2_unorm;
    case PixelFormat::bc3_unorm_srgb:
        return PixelFormat::bc3_unorm;
    default:
        assert(false);
        return format;
    }
}

PixelFormat to_srgb_space(PixelFormat format)
{
    switch (format) {
    case PixelFormat::r8g8b8a8_unorm:
        return PixelFormat::r8g8b8a8_unorm_srgb;
    case PixelFormat::b8g8r8a8_unorm:
        return PixelFormat::b8g8r8a8_unorm_srgb;
    case PixelFormat::bc1_unorm:
        return PixelFormat::bc1_unorm_srgb;
    case PixelFormat::bc2_unorm:
        return PixelFormat::bc2_unorm_srgb;
    case PixelFormat::bc3_unorm:
        return PixelFormat::bc3_unorm_srgb;
    default:
        assert(false);
        return format;
    }
}
} // namespace

template <typename T>
std::vector<T> TextureDesc::pixels_generic(int subresource_index) const noexcept
{
    assert(subresource_index < m_subresources.size());
    unsigned int  mip_level = subresource_index % m_mip_levels;
    unsigned long width     = std::max(1LU, m_width >> mip_level);
    unsigned long height    = std::max(1LU, m_height >> mip_level);

    std::vector<T> pixels(width * height);

    const auto& subresource = m_subresources[subresource_index];

    using CT = ColorTraits<T>;

    const std::uint8_t* src = m_data.data() + subresource.data_offset;
    switch (m_pixel_format) {
    case PixelFormat::r8g8b8a8_unorm:
    case PixelFormat::r8g8b8a8_unorm_srgb: {
        assert(subresource.data_size == pixels.size() * 4);
        T* dest = pixels.data();
        for (std::size_t i = 0; i < pixels.size(); ++i, src += 4, ++dest) {
            *dest = CT::from_r8g8b8a8(src[0], src[1], src[2], src[3]);
        }
        break;
    }

    case PixelFormat::b8g8r8a8_unorm:
    case PixelFormat::b8g8r8a8_unorm_srgb: {
        assert(subresource.data_size == pixels.size() * 4);
        T* dest = pixels.data();
        for (std::size_t i = 0; i < pixels.size(); ++i, src += 4, ++dest) {
            *dest = CT::from_r8g8b8a8(src[2], src[1], src[0], src[3]);
        }
        break;
    }

    case PixelFormat::bc1_unorm:
    case PixelFormat::bc1_unorm_srgb: {
        unsigned long block_aligned_width  = (width + 3) & -4;
        unsigned long block_aligned_height = (height + 3) & -4;
        assert(subresource.data_size == block_aligned_width * block_aligned_height / 2);

        for (unsigned long y = 0; y < block_aligned_height; y += 4) {
            for (unsigned long x = 0; x < block_aligned_width; x += 4, src += 8) {
                const auto values = unpack_bc1_block(src);
                copy_block<T>(pixels, width, height, x, y, values);
            }
        }
        break;
    }

    case PixelFormat::bc2_unorm:
    case PixelFormat::bc2_unorm_srgb: {
        unsigned long block_aligned_width  = (width + 3) & -4;
        unsigned long block_aligned_height = (height + 3) & -4;
        assert(subresource.data_size == block_aligned_width * block_aligned_height);

        for (unsigned long y = 0; y < block_aligned_height; y += 4) {
            for (unsigned long x = 0; x < block_aligned_width; x += 4, src += 16) {
                const auto alpha  = unpack_bc2_alpha(src);
                const auto values = unpack_bc1_block(src + 8, alpha);
                copy_block<T>(pixels, width, height, x, y, values);
            }
        }
        break;
    }

    case PixelFormat::bc3_unorm:
    case PixelFormat::bc3_unorm_srgb: {
        unsigned long block_aligned_width  = (width + 3) & -4;
        unsigned long block_aligned_height = (height + 3) & -4;
        assert(subresource.data_size == block_aligned_width * block_aligned_height);

        for (unsigned long y = 0; y < block_aligned_height; y += 4) {
            for (unsigned long x = 0; x < block_aligned_width; x += 4, src += 16) {
                const auto alpha  = unpack_bc4_block(src);
                const auto values = unpack_bc1_block(src + 8, alpha);
                copy_block<T>(pixels, width, height, x, y, values);
            }
        }
        break;
    }
    }
    return pixels;
}

std::vector<ColorRGBA> TextureDesc::pixels_linear(int subresource_index) const noexcept
{
    auto pixels = pixels_generic<ColorRGBA>(subresource_index);
    if (color_space(m_pixel_format) == ColorSpace::srgb) {
        // Convert to linear
        for (auto& pixel : pixels) {
            pixel = ColorRGBA(ColorSRGBA(pixel.r, pixel.g, pixel.b, pixel.a));
        }
    }
    return pixels;
}

std::vector<ColorSRGBA> TextureDesc::pixels_srgb(int subresource_index) const noexcept
{
    auto pixels = pixels_generic<ColorSRGBA>(subresource_index);
    if (color_space(m_pixel_format) == ColorSpace::linear) {
        // Convert to sRGBA
        for (auto& pixel : pixels) {
            pixel = ColorSRGBA(ColorRGBA(pixel.r, pixel.g, pixel.b, pixel.a));
        }
    }
    return pixels;
}

ColorSpace color_space(PixelFormat format)
{
    switch (format) {
    case PixelFormat::r8g8b8a8_unorm:
    case PixelFormat::b8g8r8a8_unorm:
    case PixelFormat::bc1_unorm:
    case PixelFormat::bc2_unorm:
    case PixelFormat::bc3_unorm:
        return ColorSpace::linear;

    case PixelFormat::r8g8b8a8_unorm_srgb:
    case PixelFormat::b8g8r8a8_unorm_srgb:
    case PixelFormat::bc1_unorm_srgb:
    case PixelFormat::bc2_unorm_srgb:
    case PixelFormat::bc3_unorm_srgb:
        return ColorSpace::srgb;

    default:
        assert(false);
        return ColorSpace::srgb;
    }
}

PixelFormat to_color_space(PixelFormat format, ColorSpace color_space)
{
    switch (color_space) {
    case ColorSpace::linear:
        return to_linear_space(format);
    case ColorSpace::srgb:
        return to_srgb_space(format);
    default:
        assert(false);
        return to_srgb_space(format);
    }
}

} // namespace khepri::renderer
