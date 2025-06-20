#pragma once

#include "font.hpp"
#include "font_face_desc.hpp"
#include "font_options.hpp"

#include <khepri/utility/string.hpp>

#include <map>
#include <memory>
#include <string_view>

namespace khepri::font {

/**
 * Creates and caches fonts.
 *
 * Use a \a FontCache to avoid duplicate instantiations of similar fonts. Request a shared font
 * reference with \ref FontCache::get.
 */
class FontCache final
{
public:
    FontCache();
    ~FontCache();

    FontCache(const FontCache& cache)            = delete;
    FontCache(FontCache&& cache)                 = delete;
    FontCache& operator=(const FontCache& cache) = delete;
    FontCache& operator=(FontCache&& cache)      = delete;

    /**
     * Adds a font face to the cache.
     *
     * \param[in] name the font face's name
     * \param[in] font_face_desc the font face's description
     *
     * After adding, the face's name can be used in \ref FontCache::get to create fonts with.
     */
    void add_face(std::string_view name, const FontFaceDesc& font_face_desc);

    /**
     * Creates or retrieves a font for a given face with given options.
     *
     * \param[in] font_face_name the name of the font face to create the font from.
     * \param[in] options the font options for the desired font.
     *
     * If the requested font already exists in the cache, it is not created again, but the
     * previously created shared reference is returned.
     */
    std::shared_ptr<Font> get(std::string_view font_face_name, const FontOptions& options);

    /**
     * Clears the font cache.
     *
     * This removes all font faces and local references to previously created fonts.
     * Previously issued font references remain valid.
     */
    void clear();

private:
    class FaceCache;

    std::map<std::string, std::unique_ptr<FaceCache>, CaseInsensitiveLess> m_faces;
};

} // namespace khepri::font
