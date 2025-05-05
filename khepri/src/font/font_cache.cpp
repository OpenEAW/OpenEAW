#include <khepri/exceptions.hpp>
#include <khepri/font/font_cache.hpp>
#include <khepri/font/font_face.hpp>

#include <map>
#include <tuple>

namespace khepri {

bool operator<(const ColorRGB& c1, const ColorRGB& c2) noexcept
{
    return std::tie(c1.r, c1.g, c1.b) < std::tie(c2.r, c2.g, c2.b);
}

namespace font {

bool operator<(const FontOptions& options1, const FontOptions& options2) noexcept
{
    return std::tie(options1.font_size_px, options1.color_top, options1.color_bottom,
                    options1.vert_scale, options1.stroke_size_px, options1.stroke_color,
                    options1.embossed) < std::tie(options2.font_size_px, options2.color_top,
                                                  options2.color_bottom, options2.vert_scale,
                                                  options2.stroke_size_px, options2.stroke_color,
                                                  options2.embossed);
}

class FontCache::FaceCache
{
public:
    explicit FaceCache(const FontFaceDesc& font_face_desc) : m_face(font_face_desc) {}

    std::shared_ptr<Font> get(const FontOptions& options)
    {
        auto it = m_fonts.find(options);
        if (it != m_fonts.end()) {
            return it->second;
        }
        std::shared_ptr<Font> font = m_face.create_font(options);
        m_fonts.insert({options, font});
        return font;
    }

    void clear()
    {
        m_fonts.clear();
    }

private:
    FontFace m_face;

    std::map<FontOptions, std::shared_ptr<Font>> m_fonts;
};

FontCache::FontCache()  = default;
FontCache::~FontCache() = default;

void FontCache::add_face(std::string_view name, const FontFaceDesc& font_face_desc)
{
    const auto it = m_faces.find(name);
    if (it != m_faces.end()) {
        throw ArgumentError();
    }
    m_faces.emplace(name, std::make_unique<FaceCache>(font_face_desc));
}

std::shared_ptr<Font> FontCache::get(std::string_view font_face_name, const FontOptions& options)
{
    const auto it = m_faces.find(font_face_name);
    if (it == m_faces.end()) {
        return nullptr;
    }
    return it->second->get(options);
}

void FontCache::clear()
{
    for (const auto& pair : m_faces) {
        pair.second->clear();
    }
}

} // namespace font
} // namespace khepri