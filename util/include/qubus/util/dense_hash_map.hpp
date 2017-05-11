/* This code is based on Malte Skarupke's fantastic flat hash map (https://github.com/skarupke/flat_hash_map).
 * The original code is licensed under the Boost Software License, Version 1.0. A copy of the entire
 * license can be found in external/licenses/LICENSE_1_0.txt.
 *
 * Our version mostly fixes some small quirks and adapts the code to match our style guide.
*/
#ifndef QUBUS_UTIL_DENSE_HASH_MAP_HPP
#define QUBUS_UTIL_DENSE_HASH_MAP_HPP

#include <qubus/util/detail/dense_hash_table.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace qubus
{
namespace util
{

template <typename Key, typename Value, typename Hasher = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<Key, Value>>>
class dense_hash_map
    : public detail::dense_hash_table<
          std::pair<Key, Value>, Key, Hasher,
          detail::key_or_value_hasher<Key, std::pair<Key, Value>, Hasher>, Equal,
          detail::key_or_value_equality<Key, std::pair<Key, Value>, Equal>, Allocator,
          typename std::allocator_traits<Allocator>::template rebind_alloc<
              detail::dense_hash_table_entry<std::pair<Key, Value>>>>
{
    using base_type = detail::dense_hash_table<
        std::pair<Key, Value>, Key, Hasher,
        detail::key_or_value_hasher<Key, std::pair<Key, Value>, Hasher>, Equal,
        detail::key_or_value_equality<Key, std::pair<Key, Value>, Equal>, Allocator,
        typename std::allocator_traits<Allocator>::template rebind_alloc<
            detail::dense_hash_table_entry<std::pair<Key, Value>>>>;

public:
    using key_type = Key;
    using mapped_type = Value;

    using base_type::base_type;

    dense_hash_map() = default;

    Value& operator[](const Key& key)
    {
        return emplace(key, convertible_to_value()).first->second;
    }

    Value& operator[](Key&& key)
    {
        return emplace(std::move(key), convertible_to_value()).first->second;
    }

    Value& at(const Key& key)
    {
        auto found = this->find(key);
        if (found == this->end())
            throw std::out_of_range("Argument passed to at() was not in the map.");
        return found->second;
    }

    const Value& at(const Key& key) const
    {
        auto found = this->find(key);
        if (found == this->end())
            throw std::out_of_range("Argument passed to at() was not in the map.");
        return found->second;
    }

    using base_type::emplace;

    std::pair<typename base_type::iterator, bool> emplace()
    {
        return emplace(key_type(), convertible_to_value());
    }

    friend bool operator==(const dense_hash_map& lhs, const dense_hash_map& rhs)
    {
        if (lhs.size() != rhs.size())
            return false;

        for (const typename base_type::value_type& value : lhs)
        {
            auto found = rhs.find(value.first);

            if (found == rhs.end())
                return false;

            if (value.second != found->second)
                return false;
        }

        return true;
    }

    friend bool operator!=(const dense_hash_map& lhs, const dense_hash_map& rhs)
    {
        return !(lhs == rhs);
    }

private:
    struct convertible_to_value
    {
        explicit operator Value() const
        {
            return Value();
        }
    };
};
}
}

#endif