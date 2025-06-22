#pragma once

#include <cctype>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace khepri {

/**
 * Returns the base name of a path-like string.
 * The base name is the path's filename, without extension.
 */
std::string basename(std::string_view str);

/**
 * Returns the uppercase version of a string.
 */
std::string uppercase(std::string_view str);

/**
 * Returns the trimmed view of a string.
 *
 * Trimmed strings have spaces at the start and end removed.
 */
std::string_view trim(std::string_view str);

/**
 * Checks if two strings are equal, ignoring case.
 *
 * @note the case-insensitive comparison is locale-independent
 */
bool case_insensitive_equals(std::string_view s1, std::string_view s2);

/**
 * Less-than comparator for case-insensitive comparisons on string-like objects.
 *
 * This comparator is a transparent comparator; it can be used instead of @c std::less<> in e.g.
 * @c std::map to avoid the requirement that the key and lookup types are the same.
 *
 * @note This class cannot be used with character pointers or literals. Use a \a string or \a
 * string_view, instead.
 * @note the case-insensitive comparison is locale-independent
 */
class CaseInsensitiveLess
{
public:
    /// Checks if the first string-like argument is lexicographically less-than the second
    /// string-like argument
    template <typename T, typename U>
    bool operator()(T&& t, U&& u) const noexcept
    {
        // Do not allow T or U to be character strings or literals.
        // Character pointers do not have std::begin/std::end and character literals's length
        // includes their null terminators, which std::end will include.
        static_assert(!std::is_pointer_v<std::decay_t<T>>,
                      "T may not be a pointer or decay-to-pointer type");
        static_assert(!std::is_pointer_v<std::decay_t<U>>,
                      "U may not be a pointer or decay-to-pointer type");

        using CharType = std::decay_t<decltype(*std::begin(t))>;
        auto const& ct = std::use_facet<std::ctype<CharType>>(std::locale::classic());

        const auto& nocase_compare = [&](CharType c1, CharType c2) {
            return ct.tolower(c1) < ct.tolower(c2);
        };

        return std::lexicographical_compare(std::begin(t), std::end(t), std::begin(u), std::end(u),
                                            nocase_compare);
    }

    /// Marks the comparator as a transparent comparator
    using is_transparent = std::bool_constant<true>;
};

/**
 * Tokenizes a string
 */
class Tokenizer
{
public:
    /**
     * Constructs the tokenizer.
     *
     * Calling #next repeatedly on a tokenizer returns subsequent substrings such that the
     * substrings are delimited by one or more of the specified delimiters.
     *
     * \param input the input string to tokenize
     * \param delimiters the delimiters to tokenize on (default = all whitespace characters)
     * \param keep_empty if true, empty tokens are included in the result
     *
     * \note the input strings are not copied, the caller must ensure they remain valid.
     */
    Tokenizer(std::string_view input, std::string_view delimiters = " \t\r\n\v\f",
              bool keep_empty = false);

    /**
     * Returns the next token from the input string, or std::none if there are no more tokens.
     */
    std::optional<std::string_view> next() noexcept;

private:
    std::string_view            m_input;
    std::string_view            m_delimiters;
    bool                        m_keep_empty;
    std::string_view::size_type m_next;
};

/**
 * @brief Splits a string into substrings
 *
 * @param str the string to split
 * @param delimiters the delimiters to split the string on
 * @param keep_empty if true, empty substrings are included in the result
 */
std::vector<std::string_view> split(std::string_view str, std::string_view delimiters,
                                    bool keep_empty = false);

template <typename ContainerT>
std::string join(const ContainerT& container, std::string_view separator)
{
    std::stringstream ss;
    if (!container.empty()) {
        auto it = container.begin();
        ss << *it;
        for (++it; it != container.end(); ++it) {
            ss << separator << *it;
        }
    }
    return ss.str();
}

} // namespace khepri