#pragma once

#include <gsl/gsl-lite.hpp>

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace khepri::renderer {

/**
 * Dimensionality of a texture
 */
enum class TextureDimension
{
    /**
     * One-dimensional texture.
     * Its height and depth are 1. Can be an array.
     */
    texture_1d,

    /**
     * Two-dimensional texture.
     * Its depth is 1. Can be an array.
     */
    texture_2d,

    /**
     * Three-dimensional texture.
     * Can NOT be an array.
     */
    texture_3d,

    /**
     * Two-dimensional cubemap.
     * Its depth is 1. Must be an array with an array size a multiple of 6.
     */
    texture_cubemap,
};

/**
 * The format of pixel data in a texture
 */
enum class PixelFormat
{
    /**
     * Four-component unsigned-normalized-integer format with 8 bits for R, G, B and A. Color data
     * is in sRGB space.
     */
    r8g8b8a8_unorm_srgb,
    b8g8r8a8_unorm_srgb,

    /**
     * Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits
     * for G, 5 bits for B, and 0 or 1 bit for A channel. The pixel data is encoded using 8 bytes
     * per 4x4 block (4 bits per pixel) providing 1:8 compression ratio against RGBA8 format. Color
     * data is in sRGB space.
     */
    bc1_unorm_srgb,

    /**
     * Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits
     * for G, 5 bits for B, and 4 bits for low-coherent separate A channel. The pixel data is
     * encoded using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio
     * against RGBA8 format. Color data is in sRGB space.
     */
    bc2_unorm_srgb,

    /**
     * Four-component unsigned-normalized-integer block-compression format with 5 bits for R, 6 bits
     * for G, 5 bits for B, and 8 bits for highly-coherent A channel. The pixel data is encoded
     * using 16 bytes per 4x4 block (8 bits per pixel) providing 1:4 compression ratio against RGBA8
     * format. Color data is in sRGB space.
     */
    bc3_unorm_srgb,
};

/**
 * Description of a texture
 */
class TextureDesc
{
public:
    /**
     * Identifies a subresource in this texture.
     *
     * A subresource is a 1D, 2D or 3D texture for a single MIP level and/or array index.
     */
    struct Subresource
    {
        /// Offset of this subresource in the texture data, in bytes.
        std::size_t data_offset;

        /// Size of this subresource in the texture data, in bytes
        std::size_t data_size;

        /// For 2D and 3D textures, row stride in bytes.
        std::size_t stride;

        /// For 3D textures, depth slice stride in bytes.
        /// This must be a mutliple of #stride.
        std::size_t depth_stride;
    };

    /**
     * Constructs the texture
     */
    TextureDesc(TextureDimension dimension, unsigned long width, unsigned long height,
                unsigned long depth_array_size, unsigned long mip_levels, PixelFormat pixel_format,
                std::vector<Subresource> subresources, std::vector<std::uint8_t> data)
        : m_dimension(dimension)
        , m_width(width)
        , m_height(height)
        , m_depth_array_size(depth_array_size)
        , m_mip_levels(mip_levels)
        , m_pixel_format(pixel_format)
        , m_subresources(std::move(subresources))
        , m_data(std::move(data))
    {
        assert(m_width >= 1);
        assert(m_height >= 1);
        assert(dimension != TextureDimension::texture_3d || m_depth_array_size >= 1);
        assert(m_mip_levels >= 1);
        assert(!m_subresources.empty());
        assert(!m_data.empty());
    }

    /**
     * Returns the dimension of the texture.
     */
    [[nodiscard]] TextureDimension dimension() const noexcept
    {
        return m_dimension;
    }

    /**
     * Returns the width of the texture.
     *
     * The width will be at least 1.
     */
    [[nodiscard]] unsigned long width() const noexcept
    {
        return m_width;
    }

    /**
     * Returns the height of the texture.
     *
     * The height will be at least 1. If the texture dimension is #TextureDimension::texture_1d,
     * this will be exactly 1.
     */
    [[nodiscard]] unsigned long height() const noexcept
    {
        return m_height;
    }

    /**
     * Returns the depth of the texture.
     *
     * The depth will be at least 1. Unless the texture dimension is #TextureDimension::texture_3d,
     * this will be exactly 1.
     */
    [[nodiscard]] unsigned long depth() const noexcept
    {
        return (m_dimension == TextureDimension::texture_3d) ? m_depth_array_size : 1;
    }

    /**
     * Number of mip levels in the texture.
     *
     * The number of mip levels will be at least 1.
     */
    [[nodiscard]] unsigned long mip_levels() const noexcept
    {
        return m_mip_levels;
    }

    /**
     * Returns the size of the texture array, if any.
     *
     * If the array size is greater than 0, the texture is a texture array. 3D textures can never
     * be texture arrays. If the texture is a cubemap, the returned array size is a multiple of 6,
     * so to get the "true" array size, divide the returned value by 6.
     */
    [[nodiscard]] unsigned long array_size() const noexcept
    {
        return (m_dimension != TextureDimension::texture_3d) ? m_depth_array_size : 0;
    }

    /**
     * The pixel format of the data in the texture.
     */
    [[nodiscard]] PixelFormat pixel_format() const noexcept
    {
        return m_pixel_format;
    }

    /**
     * Returns the specified subresource.
     *
     * Subresources are mip-levels and array slices.
     * Use the #subresource_index() method to find the index of a subresource.
     */
    [[nodiscard]] const Subresource& subresource(std::size_t index) const noexcept
    {
        assert(index < m_subresources.size());
        return m_subresources[index];
    }

    /**
     * Returns the raw texture data.
     *
     * This texture data is indexed by the various subresources and is stored in the format
     * specified by #pixel_format()
     */
    [[nodiscard]] gsl::span<const std::uint8_t> data() const noexcept
    {
        return m_data;
    }

    /**
     * Returns the index of a specified MIP and array subresource.
     * The returned index can be passed to #subresource() to retrieve the data for that subresource.
     */
    [[nodiscard]] std::size_t subresource_index(std::size_t mip_level,
                                                std::size_t array_index) const noexcept
    {
        return mip_level + (array_index * m_mip_levels);
    }

private:
    TextureDimension          m_dimension;
    unsigned long             m_width;
    unsigned long             m_height;
    unsigned long             m_depth_array_size;
    unsigned long             m_mip_levels;
    PixelFormat               m_pixel_format;
    std::vector<Subresource>  m_subresources;
    std::vector<std::uint8_t> m_data;
};

} // namespace khepri::renderer