/* This code is based on Malte Skarupke's fantastic flat hash map (https://github.com/skarupke/flat_hash_map).
 * The original code is licensed under the Boost Software License, Version 1.0. A copy of the entire
 * license can be found in external/licenses/LICENSE_1_0.txt.
 *
 * Our version mostly fixes some small quirks and adapts the code to match our style guide.
*/
#ifndef QUBUS_UTIL_DENSE_HASH_SET_HPP
#define QUBUS_UTIL_DENSE_HASH_SET_HPP

#include <qubus/util/detail/dense_hash_table.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace qubus
{
namespace util
{

template <typename T, typename Hasher = std::hash<T>, typename Equal = std::equal_to<T>,
          typename Allocator = std::allocator<T>>
class dense_hash_set
    : public detail::dense_hash_table<T, T, Hasher, detail::functor_storage<size_t, Hasher>, Equal,
                                      detail::functor_storage<bool, Equal>, Allocator,
                                      typename std::allocator_traits<Allocator>::
                                          template rebind_alloc<detail::dense_hash_table_entry<T>>>
{
    using base =
        detail::dense_hash_table<T, T, Hasher, detail::functor_storage<size_t, Hasher>, Equal,
                                 detail::functor_storage<bool, Equal>, Allocator,
                                 typename std::allocator_traits<Allocator>::template rebind_alloc<
                                     detail::dense_hash_table_entry<T>>>;

public:
    using key_type = T;

    using base::base;

    dense_hash_set() = default;

    template <typename... Args>
    std::pair<typename base::iterator, bool> emplace(Args&&... args)
    {
        return base::emplace(T(std::forward<Args>(args)...));
    }

    std::pair<typename base::iterator, bool> emplace(const key_type& arg)
    {
        return base::emplace(arg);
    }

    std::pair<typename base::iterator, bool> emplace(key_type& arg)
    {
        return base::emplace(arg);
    }

    std::pair<typename base::iterator, bool> emplace(const key_type&& arg)
    {
        return base::emplace(std::move(arg));
    }

    std::pair<typename base::iterator, bool> emplace(key_type&& arg)
    {
        return base::emplace(std::move(arg));
    }

    friend bool operator==(const dense_hash_set& lhs, const dense_hash_set& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;

        for (const T& value : lhs)
        {
            if (rhs.find(value) == rhs.end())
                return false;
        }

        return true;
    }

    friend bool operator!=(const dense_hash_set& lhs, const dense_hash_set& rhs)
    {
        return !(lhs == rhs);
    }
};
}
}

#endif
