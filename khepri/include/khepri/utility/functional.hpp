#pragma once

#include <gsl/gsl-lite.hpp>

#include <vector>

namespace khepri {

/**
 * @brief Returns a vector of members from a collection of objects.
 *
 * Use this function to copy a particular member from a collection of objects.
 *
 * @tparam ClassT       The class type to copy the member from.
 * @tparam ValueT       The value type of the member.
 * @param collection    The collection of objects to copy the member from.
 * @param member_ptr    The pointer-to-member of the class member to copy.
 */
template <class ClassT, class ValueT>
auto pluck(gsl::span<const ClassT> collection, ValueT ClassT::* member_ptr)
{
    std::vector<ValueT> v;
    v.reserve(collection.size());
    for (const auto& item : collection) {
        v.push_back(item.*member_ptr);
    }
    return v;
}

} // namespace khepri
