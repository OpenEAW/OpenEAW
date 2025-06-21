#include "font_face_state.hpp"

#include <khepri/font/exceptions.hpp>
#include <khepri/log/log.hpp>
#include <khepri/math/bits.hpp>
#include <khepri/math/color_srgb.hpp>
#include <khepri/math/math.hpp>

#include <mutex>

#include FT_STROKER_H
#include FT_GLYPH_H

namespace khepri::font::detail {
namespace {
constexpr khepri::log::Logger LOG("font");

constexpr auto FT_26_6_MULTIPLIER = 64;

template <typename T, void (*Deleter)(T)>
class FTScoped final
{
public:
    FTScoped() = default;
    ~FTScoped()
    {
        Deleter(m_object);
    }

    FTScoped(const FTScoped&)            = delete;
    FTScoped(FTScoped&&)                 = delete;
    FTScoped& operator=(const FTScoped&) = delete;
    FTScoped& operator=(FTScoped&&)      = delete;

    T get()
    {
        return m_object;
    }

    T* operator&()
    {
        return &m_object;
    }

    operator T()
    {
        return m_object;
    }

private:
    T m_object{nullptr};
};

/**
 * Blend a 8bpp grayscale bitmap into 32bpp RGBA's alpha channel
 */
void blend_bitmap_alpha(gsl::span<const std::uint8_t> src, unsigned int width, unsigned int height,
                        unsigned int src_pitch, gsl::span<std::uint8_t> dest,
                        unsigned int dest_pitch)
{
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0, d = 0; x < width; ++x, d += 4) {
            const unsigned int src_alpha = src[y * src_pitch + x];
            auto&              dst_alpha = dest[y * dest_pitch + d + 3];

            // Alphas are added together
            dst_alpha = std::min<unsigned int>(dst_alpha + src_alpha,
                                               std::numeric_limits<std::uint8_t>::max());
        }
    }
}

// Describes a gradient
struct GradientDesc
{
    // Top of the gradient, in pixels relative to the source bitmap
    int color_top_y{};

    // Color of the top of the gradient.
    khepri::ColorRGB color_top{};

    // Bottom of the gradient, in pixels relative to the source bitmap
    int color_bottom_y{};

    // Color of the top of the gradient.
    khepri::ColorRGB color_bottom;
};

using FTGlyphRef   = FTScoped<FT_Glyph, FT_Done_Glyph>;
using FTStrokerRef = FTScoped<FT_Stroker, FT_Stroker_Done>;

struct CharInfo
{
    FTGlyphRef glyph;
    FTGlyphRef stroke_glyph;
    FT_Pos     ofs_x{};
};

struct StringInfo
{
    std::vector<CharInfo> chars;
    FT_BBox               bbox{0, 0, 0, 0};
};

std::uint8_t find_darkest_above(gsl::span<const std::uint8_t> src, unsigned int src_pitch,
                                unsigned int x, unsigned int y, unsigned int distance)
{
    if (y < distance) {
        // Pretend the pixels outside of the source area are dark.
        return 0;
    }

    auto darkest = std::numeric_limits<std::uint8_t>::max();
    for (unsigned int h = 1; h <= distance; ++h) {
        darkest = std::min(darkest, src[(y - h) * src_pitch + x]);
    }
    return darkest;
}

std::uint8_t find_darkest_below(gsl::span<const std::uint8_t> src, unsigned int src_pitch,
                                unsigned int height, unsigned int x, unsigned int y,
                                unsigned int distance)
{
    if (y >= height - distance) {
        // Pretend the pixels outside of the source area are dark.
        return 0;
    }

    auto darkest = std::numeric_limits<std::uint8_t>::max();
    for (unsigned int h = 1; h <= distance; ++h) {
        darkest = std::min(darkest, src[(y + h) * src_pitch + x]);
    }
    return darkest;
}

/**
 * Blends a 8bpp grayscale bitmap into a 32bpp RGB with pre-multiplied Alpha
 */
void blend_bitmap(gsl::span<const std::uint8_t> src, unsigned int width, unsigned int height,
                  unsigned int src_pitch, gsl::span<std::uint8_t> dest, unsigned int dest_pitch,
                  const GradientDesc& gradient, const ColorRGB& dest_color, bool embossed)
{
    // How much to lighten or darken
    constexpr float        darken_strength  = 0.5F;
    constexpr float        lighten_strength = 0.25F;
    constexpr unsigned int emboss_radius    = 2;

    const auto gradient_range_y = gradient.color_bottom_y - gradient.color_top_y;

    // Alpha-blend the new color on top of the destination
    for (unsigned int y = 0, dst_ofs = 0, src_row_ofs = 0; y < height;
         ++y, dst_ofs += dest_pitch, src_row_ofs += src_pitch) {
        // Calculate the gradient color for this position
        const auto t =
            khepri::saturate((static_cast<float>(y) - static_cast<float>(gradient.color_top_y)) /
                             static_cast<float>(gradient_range_y));
        const auto gradient_color = khepri::lerp(gradient.color_top, gradient.color_bottom, t);

        const auto max_value = std::numeric_limits<std::uint8_t>::max();

        for (unsigned int x = 0, d = dst_ofs; x < width; ++x, d += 4) {
            const auto src_value = src[src_row_ofs + x];
            if (src_value != 0) {
                auto pixel = gradient_color;
                if (embossed) {
                    // Lighten/darken the pixel if they are on bottom/top edge.
                    // Edge detection works by finding the "darkest" source pixel above/below it.
                    const auto darkest_top =
                        find_darkest_above(src, src_pitch, x, y, emboss_radius);
                    pixel *= lerp(1.0F, darken_strength,
                                  static_cast<float>(max_value - darkest_top) / max_value);

                    const auto darkest_btm =
                        find_darkest_below(src, src_pitch, height, x, y, emboss_radius);
                    pixel += ColorRGB(1, 1, 1) * lighten_strength *
                             static_cast<float>(max_value - darkest_btm) / max_value;
                }

                const float src_alpha = static_cast<float>(src_value) / max_value;

                // The destination color is supposed to be pre-multiplied alpha. Rather than store
                // it in sRGB format in the texture (and have expensive conversions), we pass the
                // destination color to this function and "pre"-multiply here, before we blend with
                // the new bitmap. This only works because the background is a single
                // (alpha-blended) color.
                const float dst_alpha = static_cast<float>(dest[d + 3]) / max_value;
                const auto  dst_color = dest_color * dst_alpha;

                // Color is blended with (SrcAlpha, InvSrcAlpha)
                pixel = saturate(pixel * src_alpha + dst_color * (1 - src_alpha));

                // Store as sRGB
                ColorSRGB srgb(pixel);
                dest[d + 0] = srgb.r;
                dest[d + 1] = srgb.g;
                dest[d + 2] = srgb.b;

                // Alphas are added together
                dest[d + 3] = std::min<unsigned int>(dest[d + 3] + src_value, max_value);
            }
        }
    }
}

template <typename T, typename U>
T freetype_downcast(U&& value)
{
    // Have to use reinterpret_cast here, FreeType does not use inheritance
    // NOLINTNEXTLINE
    return reinterpret_cast<T>(value);
}

StringInfo calculate_string_info(FT_Face face, FT_Stroker stroker, std::u16string_view text)
{
    // Calculate text bounding box
    std::vector<CharInfo> chars(text.size());
    FT_BBox               text_bbox = {0, 0, 0, 0};

    FT_UInt prev_glyph_index{};
    FT_Pos  pen_x = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto glyph_index = FT_Get_Char_Index(face, text[i]);
        if (auto error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            LOG.error("cannot get glyph info: {}", error);
            throw FontError("unable to render text");
        }

        if (FT_HAS_KERNING(face) && (i > 0)) {
            FT_Vector kerning{0, 0};
            FT_Get_Kerning(face, prev_glyph_index, glyph_index, FT_KERNING_DEFAULT, &kerning);
            pen_x += kerning.x / FT_26_6_MULTIPLIER;
        }

        if (auto error = FT_Get_Glyph(face->glyph, &chars[i].glyph)) {
            LOG.error("cannot get glyph: {}", error);
            throw FontError("unable to render font");
        }

        FT_BBox glyph_bbox;
        if (stroker != nullptr) {
            // Adding a stroke makes the glyph bigger, so use the stroked glyph's bounds
            if (auto error = FT_Glyph_Copy(chars[i].glyph, &chars[i].stroke_glyph)) {
                LOG.error("cannot copy glyph: {}", error);
                throw FontError("unable to render font");
            }

            if (auto error = FT_Glyph_Stroke(&chars[i].stroke_glyph, stroker, 1)) {
                LOG.error("cannot stroke glyph: {}", error);
                throw FontError("unable to render font");
            }

            FT_Glyph_Get_CBox(chars[i].stroke_glyph, FT_GLYPH_BBOX_GRIDFIT, &glyph_bbox);
        } else {
            FT_Glyph_Get_CBox(chars[i].glyph, FT_GLYPH_BBOX_GRIDFIT, &glyph_bbox);
        }

        glyph_bbox.xMin += pen_x;
        glyph_bbox.xMax += pen_x;

        if (i == 0) {
            text_bbox = glyph_bbox;
        } else {
            text_bbox.xMin = std::min(text_bbox.xMin, glyph_bbox.xMin);
            text_bbox.xMax = std::max(text_bbox.xMax, glyph_bbox.xMax);
            text_bbox.yMin = std::min(text_bbox.yMin, glyph_bbox.yMin);
            text_bbox.yMax = std::max(text_bbox.yMax, glyph_bbox.yMax);
        }

        chars[i].ofs_x = pen_x / FT_26_6_MULTIPLIER;

        pen_x += face->glyph->advance.x;
        prev_glyph_index = glyph_index;
    }
    return {std::move(chars), text_bbox};
}

// Reference-counted handle to the FT_Library handle
class LibraryState
{
public:
    FT_Library& acquire()
    {
        std::lock_guard lock(m_lock);
        if (m_refcount == 0) {
            if (auto error = FT_Init_FreeType(&m_handle)) {
                LOG.error("unable to initialize FreeType: {}", error);
                throw FontError("unable to initialize FreeType");
            }
        }
        m_refcount++;
        return m_handle;
    }

    void release() noexcept
    {
        std::lock_guard lock(m_lock);
        if (--m_refcount == 0) {
            FT_Done_FreeType(m_handle);
        }
    }

    static LibraryState& get()
    {
        static LibraryState library;
        return library;
    }

private:
    std::mutex   m_lock;
    FT_Library   m_handle{};
    unsigned int m_refcount{0};
};

} // namespace

FontFaceState::FontFaceState(const FontFaceDesc& font_face_desc)
    : m_library(LibraryState::get().acquire()), m_data(font_face_desc.data())
{
    if (auto error = FT_New_Memory_Face(m_library, m_data.data(),
                                        static_cast<FT_Long>(m_data.size()), 0, &m_face)) {
        LOG.error("unable to create font: {}", error);
        throw FontError("unable to create font");
    }

    if (!FT_IS_SCALABLE(m_face)) {
        // Only scalable fonts are supported (for now)
        throw FontError("font is not scalable");
    }

    LOG.info("created font \"{}-{}\"", m_face->family_name, m_face->style_name);
}

FontFaceState::~FontFaceState()
{
    FT_Done_Face(m_face);

    LibraryState::get().release();
}

TextRender FontFaceState::render(std::u16string_view text, const FontOptions& options) const
{
    const auto font_width_px = static_cast<FT_UInt>(options.font_size_px);
    const auto font_height_px =
        static_cast<FT_UInt>(static_cast<float>(options.font_size_px) * options.vert_scale);

    FTStrokerRef stroker;
    if (options.stroke_size_px > 0) {
        FT_Stroker_New(m_library, &stroker);
        FT_Stroker_Set(stroker, static_cast<FT_Fixed>(options.stroke_size_px * FT_26_6_MULTIPLIER),
                       FT_STROKER_LINECAP_BUTT, FT_STROKER_LINEJOIN_ROUND, 0);
    }

    // Setting the font size modifies the face, so we can only render one string at a time
    std::lock_guard lock(m_mutex);

    if (auto error = FT_Set_Pixel_Sizes(m_face, font_width_px, font_height_px)) {
        LOG.error("cannot set character size: {}", error);
        throw FontError("unable to create font");
    }

    // Calculate text bounding box and character info
    auto info = calculate_string_info(m_face, stroker, text);

    // Area of the texture that will be used by the rendered text
    const Rect text_rect{1, 1,
                         static_cast<unsigned long>(info.bbox.xMax / FT_26_6_MULTIPLIER -
                                                    info.bbox.xMin / FT_26_6_MULTIPLIER),
                         static_cast<unsigned long>(info.bbox.yMax / FT_26_6_MULTIPLIER -
                                                    info.bbox.yMin / FT_26_6_MULTIPLIER)};

    // Add one transparent pixel around the text in the texture.
    // This pixel acts as a buffer for rounding errors during font rendering and avoids border
    // issues during texture sampling when rendering the texture.
    const auto tex_width  = ceil_power_of_two(text_rect.width + 2);
    const auto tex_height = ceil_power_of_two(text_rect.height + 2);
    const auto tex_pitch  = tex_width * std::size_t{4};

    // Create empty, transparent texture
    std::vector<std::uint8_t> data(tex_pitch * tex_height, 0);

    // Calculate the bitmap positions of the ascender/descender lines.
    // This is where the color gradient starts/finishes. Note that these positions may lie outside
    // of the text bitmap, if the string does not use the full ascender/descender, or that the
    // bitmap may extend beyond these points if the font has glyphs that extend beyond the
    // ascender/descender lines.
    const auto ascender_px = static_cast<FT_Long>(m_face->size->metrics.y_ppem) * m_face->ascender /
                             m_face->units_per_EM;
    const auto descender_px = static_cast<FT_Long>(m_face->size->metrics.y_ppem) *
                              m_face->descender / m_face->units_per_EM;
    const auto y_color_top    = info.bbox.yMax / FT_26_6_MULTIPLIER - ascender_px;
    const auto y_color_bottom = info.bbox.yMax / FT_26_6_MULTIPLIER - descender_px;

    // Render stroke first
    if (stroker != nullptr) {
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (auto error = FT_Glyph_To_Bitmap(&info.chars[i].stroke_glyph, FT_RENDER_MODE_NORMAL,
                                                nullptr, 1)) {
                LOG.error("cannot rasterize glyph: {}", error);
                throw FontError("unable to render font");
            }

            auto* bitmapGlyph = freetype_downcast<FT_BitmapGlyph>(info.chars[i].stroke_glyph.get());

            const auto x =
                info.chars[i].ofs_x + bitmapGlyph->left - info.bbox.xMin / FT_26_6_MULTIPLIER;
            const auto  y      = info.bbox.yMax / FT_26_6_MULTIPLIER - bitmapGlyph->top;
            const auto& bitmap = bitmapGlyph->bitmap;
            if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
                throw FontError("unsupported pixel format while rendering font");
            }

            // Store the stroked glyph's luminescence in the alpha channel.
            // We read this back during blending for the main glyph and modulate with the stroke
            // color when calculating the final color. This avoids intermediate RGB -> sRGB ->
            // RGB conversions for the color channel and avoids the need to store the
            // intermediate texture at a higher resolution.
            const auto src_buffer = gsl::span<const std::uint8_t>(
                bitmap.buffer, bitmap.rows * static_cast<std::size_t>(bitmap.pitch));
            const auto dst_buffer = gsl::span<std::uint8_t>(data).subspan((y + 1) * tex_pitch +
                                                                          (x + 1) * std::size_t{4});
            blend_bitmap_alpha(src_buffer, bitmap.width, bitmap.rows, bitmap.pitch, dst_buffer,
                               tex_pitch);
        }

        // Convert the texture after stroke glyph to sRGB with pre-multiplied alpha
        // We will likely overwrite some parts of this when writing the main glyph, below.
        for (unsigned int y = 0; y < tex_height; ++y) {
            const auto dst_row = gsl::span<std::uint8_t>(data).subspan(y * tex_pitch);
            for (unsigned int x = 0, d = 0; x < tex_width; ++x, d += 4) {
                if (dst_row[d + 3] != 0) {
                    const float alpha = static_cast<float>(dst_row[d + 3]) /
                                        std::numeric_limits<std::uint8_t>::max();
                    ColorSRGB srgb(options.stroke_color * alpha);
                    dst_row[d + 0] = srgb.r;
                    dst_row[d + 1] = srgb.g;
                    dst_row[d + 2] = srgb.b;
                }
            }
        }
    }

    // Render main glyphs
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (auto error =
                FT_Glyph_To_Bitmap(&info.chars[i].glyph, FT_RENDER_MODE_NORMAL, nullptr, 1)) {
            LOG.error("cannot rasterize glyph: {}", error);
            throw FontError("unable to render font");
        }

        auto* bitmapGlyph = freetype_downcast<FT_BitmapGlyph>(info.chars[i].glyph.get());

        const long x =
            info.chars[i].ofs_x + bitmapGlyph->left - info.bbox.xMin / FT_26_6_MULTIPLIER;
        const long  y      = info.bbox.yMax / FT_26_6_MULTIPLIER - bitmapGlyph->top;
        const auto& bitmap = bitmapGlyph->bitmap;
        if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
            throw FontError("unsupported pixel format while rendering font");
        }

        GradientDesc gradient{};
        gradient.color_top      = options.color_top;
        gradient.color_top_y    = y_color_top - y;
        gradient.color_bottom   = options.color_bottom;
        gradient.color_bottom_y = y_color_bottom - y;

        const auto src_buffer = gsl::span<const std::uint8_t>(
            bitmap.buffer, bitmap.rows * static_cast<size_t>(bitmap.pitch));
        const auto dst_buffer =
            gsl::span<std::uint8_t>(data).subspan((y + 1) * tex_pitch + (x + 1) * std::size_t{4});
        blend_bitmap(src_buffer, bitmap.width, bitmap.rows, bitmap.pitch, dst_buffer, tex_pitch,
                     gradient, options.stroke_color, options.embossed);
    }

    return TextRender{khepri::renderer::TextureDesc(
                          khepri::renderer::TextureDimension::texture_2d, tex_width, tex_height, 0,
                          1, khepri::renderer::PixelFormat::r8g8b8a8_unorm_srgb,
                          {{0, tex_pitch * tex_height, tex_pitch, tex_pitch * tex_height}},
                          std::move(data)),
                      text_rect, static_cast<int>(info.bbox.yMax / FT_26_6_MULTIPLIER)};
}

} // namespace khepri::font::detail
