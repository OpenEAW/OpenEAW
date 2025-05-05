#include <cassert>
#include <string_view>

namespace openglyph::detail {

bool split(std::string_view str, const char* separators, std::string_view* result,
           std::size_t count) noexcept
{
    assert(separators != nullptr);
    assert(result != nullptr);
    const char* const space_chars = " \t\r\n\v\f";

    std::size_t i;
    auto        start = str.find_first_not_of(space_chars, 0);

    for (i = 0; (i < count) && (start != std::string_view::npos); ++i) {
        auto sep       = str.find_first_of(separators, start);
        auto end       = (sep == std::string_view::npos) ? str.size() - 1 : sep - 1;
        auto last_char = str.find_last_not_of(space_chars, end);
        if (last_char != std::string_view::npos) {
            end = last_char + 1;
            assert(end >= start);
        }

        result[i] = str.substr(start, end - start);

        start = (sep != std::string_view::npos) ? str.find_first_not_of(space_chars, sep + 1) : sep;
    }

    return ((i == count) && (start == std::string_view::npos));
}

} // namespace openglyph::detail
