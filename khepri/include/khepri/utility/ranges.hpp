#pragma once

#include <memory>
#include <ranges>

namespace khepri::views {

namespace detail {
struct PointersFn : std::ranges::range_adaptor_closure<PointersFn>
{
    template <std::ranges::viewable_range R>
        requires std::is_lvalue_reference_v<std::ranges::range_reference_t<R>>
    constexpr auto operator()(R&& range) const noexcept
    {
        return std::forward<R>(range) |
               std::views::transform([](auto& value) { return std::addressof(value); });
    }
};
} // namespace detail

/**
 * A range adaptor that returns a range of pointers to the elements of the input range.
 */
inline constexpr auto to_pointers = detail::PointersFn{};

namespace detail {
template <auto Member>
struct PluckFn : std::ranges::range_adaptor_closure<PluckFn<Member>>
{
    template <std::ranges::viewable_range R>
        requires std::is_lvalue_reference_v<std::ranges::range_reference_t<R>>
    constexpr auto operator()(R&& range) const
    {
        return std::forward<R>(range) | std::views::transform([](auto&& value) {
                   if constexpr (std::is_pointer_v<std::decay_t<decltype(value)>>) {
                       return std::forward<decltype(value)>(value)->*Member;
                   } else {
                       return std::forward<decltype(value)>(value).*Member;
                   }
               });
    }
};
} // namespace detail

/**
 * @brief A range adapter that returns a range of references to a member of the objects of the input
 * range.
 *
 * @tparam MemberPtr   The pointer-to-member of the class member to return a reference to.
 */
template <auto MemberPtr>
inline constexpr auto pluck = detail::PluckFn<MemberPtr>{};

} // namespace khepri::views

namespace khepri::ranges {
namespace details {
template <template <class...> class Container>
struct ToFn : public std::ranges::range_adaptor_closure<ToFn<Container>>
{
    template <std::ranges::viewable_range R>
    auto operator()(R&& range) const
    {
        return Container<std::ranges::range_value_t<R>>(range.begin(), range.end());
    }
};

} // namespace details
/**
 * Construct a new non-view object from a source range.
 *
 * This is intended as a simplistic replacement for std::ranges::to, which is not fully supported by
 * all of our targeted compilers.
 */
template <template <class...> class Container>
constexpr auto to()
{
    return details::ToFn<Container>{};
}
} // namespace khepri::ranges
