#ifndef QBB_KUBUS_DENSE_HASH_MAP_HPP
#define QBB_KUBUS_DENSE_HASH_MAP_HPP

#include <functional>
#include <utility>
#include <vector>
#include <memory>

namespace qbb
{
namespace util
{

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
class dense_hash_map
{
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using iterator = typename std::vector<value_type, Allocator>::iterator;
    using const_iterator = typename std::vector<value_type, Allocator>::const_iterator;
    using hasher = Hash;
    using key_equal = KeyEqual;

    explicit dense_map(Key empty_key_, std::size_t bucket_count_ = 40)
    : empty_key_(std::move(empty_key_)), num_of_used_buckets_(0),
      buckets_(bucket_count_, value_type(empty_key_, T()))
    {
    }

    bool insert(const value_type& value)
    {
        return emplace(value);
    }

    template <typename... Args>
    bool emplace(Args&&... args)
    {
        if (size() + 1 >= max_load_factor() * bucket_count())
            resize_and_rehash();

        value_type value(std::forward<Args>(args)...);

        auto bucket_id = hasher()(value.first) % bucket_count();

        auto& bucket = buckets_[bucket_id];
        
        if (key_equal()(bucket.first, value.first))
        {
            return false;
        }
        else
        {
            for (;;)
            {
                auto& bucket = buckets_[bucket_id];

                if (key_equal()(key.first, empty_key_))
                {
                    bucket = std::move(value);
                    return true;
                }

                bucket_id = (bucket_id + 1) % bucket_count();
            }
        }
    }

    float load_factor() const
    {
        return num_of_used_buckets_ / static_cast<float>(buckets_.size());
    }

    float max_load_factor() const
    {
        return 0.75;
    }

    std::size_t size() const
    {
        return num_of_used_buckets_;
    }

    std::size_t bucket_count() const
    {
        return buckets_.size();
    }

private:
    void resize_and_rehash()
    {
        dense_hash_map new_map(empty_key_, 2 * bucket_count());

        for (auto& value : buckets_)
        {
            if (!key_equal()(value.first, empty_key_))
            {
                new_map.emplace(std::move(value));
            }
        }

        *this = std::move(new_map);
    }

    key_type empty_key_;
    std::size_t num_of_used_buckets_;
    std::vector<value_type, Allocator> buckets_;
};
}
}

#endif