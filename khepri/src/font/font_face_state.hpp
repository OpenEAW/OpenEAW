#include <khepri/font/font.hpp>
#include <khepri/font/font_face_desc.hpp>
#include <khepri/font/font_options.hpp>
#include <khepri/math/size.hpp>

#include <mutex>
#include <string_view>

// Freetype
#include <ft2build.h>
#include FT_FREETYPE_H

namespace khepri::font::detail {

class FontFaceState final
{
public:
    explicit FontFaceState(const FontFaceDesc& font_face_desc);
    ~FontFaceState();

    FontFaceState(const FontFaceState&)            = delete;
    FontFaceState(FontFaceState&&)                 = delete;
    FontFaceState& operator=(const FontFaceState&) = delete;
    FontFaceState& operator=(FontFaceState&&)      = delete;

    TextRender render(std::u16string_view text, const FontOptions& options) const;

private:
    FT_Library m_library;

    mutable std::mutex        m_mutex;
    std::vector<std::uint8_t> m_data;
    FT_Face                   m_face{};
};

} // namespace khepri::font::detail
