#include <khepri/utility/string.hpp>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>

namespace khepri {

std::string basename(std::string_view str)
{
    return std::filesystem::path(str).filename().replace_extension("").string();
}

std::string uppercase(std::string_view str)
{
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(),
                   [](unsigned char ch) { return toupper(ch); });
    return result;
}

std::string_view trim(std::string_view str)
{
    static constexpr auto spaces = "\t\n\v\f\r ";

    const auto start = str.find_first_not_of(spaces);
    if (start == std::string_view::npos) {
        return "";
    }
    auto end = str.find_last_not_of(spaces);
    assert(end != std::string_view::npos);
    return str.substr(start, end + 1 - start);
}

bool case_insensitive_equals(std::string_view s1, std::string_view s2)
{
    using CharType = std::string_view::value_type;
    auto const& ct = std::use_facet<std::ctype<CharType>>(std::locale::classic());
    return std::equal(std::begin(s1), std::end(s1), std::begin(s2), std::end(s2),
                      [&](CharType c1, CharType c2) { return ct.tolower(c1) == ct.tolower(c2); });
}

Tokenizer::Tokenizer(std::string_view input, std::string_view delimiters)
    : m_input(input), m_delimiters(delimiters), m_next(m_input.find_first_not_of(m_delimiters))
{}

std::optional<std::string_view> Tokenizer::next() noexcept
{
    if (m_next == std::string_view::npos) {
        return {};
    }

    const auto start = m_next;
    const auto end   = m_input.find_first_of(m_delimiters, start);
    const auto count = ((end != std::string_view::npos) ? end : m_input.size()) - start;

    m_next = m_input.find_first_not_of(m_delimiters, end);
    return m_input.substr(start, count);
}

std::vector<std::string_view> split(std::string_view str, std::string_view delimiters)
{
    std::vector<std::string_view> result;

    Tokenizer tokenizer(str, delimiters);
    while (auto token = tokenizer.next()) {
        result.push_back(*token);
    }
    return result;
}
} // namespace khepri