#ifndef QBB_UTIL_DENSE_HASH_MAP_HPP
#define QBB_UTIL_DENSE_HASH_MAP_HPP

#include <qbb/util/unreachable.hpp>
#include <qbb/util/assert.hpp>

#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>

namespace qubus
{
namespace util
{

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Bucket = std::pair<Key, T>,
          typename Allocator = std::allocator<Bucket>>
class dense_hash_map
{
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = Bucket;
    using iterator = typename std::vector<value_type, Allocator>::iterator;
    using const_iterator = typename std::vector<value_type, Allocator>::const_iterator;
    using hasher = Hash;
    using key_equal = KeyEqual;

    explicit dense_hash_map(Key empty_key_, std::size_t bucket_count_ = 40)
    : empty_key_(std::move(empty_key_)), num_of_used_buckets_(0),
      buckets_(bucket_count_, value_type(empty_key_, T()))
    {
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return emplace(value);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        if (size() + 1 >= std::max(std::size_t(max_load_factor() * bucket_count()), std::size_t(1)))
            resize_and_rehash();

        value_type value(std::forward<Args>(args)...);

        QBB_ASSERT(value.first != empty_key_, "The empty key is not a valid key.");

        auto guessed_bucket_id = hasher()(value.first) % bucket_count();
        auto bucket_id = guessed_bucket_id;

        do
        {
            auto& bucket = buckets_[bucket_id];

            if (key_equal()(bucket.first, value.first))
            {
                return {buckets_.begin() + bucket_id, false};
            }
            else if(key_equal()(bucket.first, empty_key_))
            {
                bucket = std::move(value);
                ++num_of_used_buckets_;

                return {buckets_.begin() + bucket_id, true};
            }

            bucket_id = (bucket_id + 1) % bucket_count();
        } while(bucket_id != guessed_bucket_id);

        QBB_UNREACHABLE();
    }

    iterator erase(const_iterator pos)
    {
        auto first = buckets_.begin();
        iterator non_const_pos = first + (pos - first);

        return erase(non_const_pos);
    }

    iterator erase(iterator pos)
    {
        *pos = value_type(empty_key_, T());

        --num_of_used_buckets_;

        return pos + 1;
    }

    std::size_t erase(const key_type& key)
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto bucket_id = find_bucket(key);

        if (is_valid_bucket(bucket_id))
        {
            erase(buckets_.begin() + bucket_id);

            return 1;
        }

        return 0;
    }

    iterator find(const key_type& key)
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto bucket_id = find_bucket(key);

        if (is_valid_bucket(bucket_id))
        {
            return buckets_.begin() + bucket_id;
        }
        else
        {
            return invalid_iterator();
        }
    }

    const_iterator find(const key_type& key) const
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto bucket_id = find_bucket(key);

        if (is_valid_bucket(bucket_id))
        {
            return buckets_.begin() + bucket_id;
        }
        else
        {
            return invalid_iterator();
        }
    }

    mapped_type& at(const key_type& key)
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto pos = find(key);

        if (pos != buckets_.end())
        {
            return pos->second;
        }
        else
        {
            throw std::out_of_range("The key is not contained in this map.");
        }
    }

    const mapped_type& at(const key_type& key) const
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto pos = find(key);

        if (pos != buckets_.end())
        {
            return pos->second;
        }
        else
        {
            throw std::out_of_range("The key is not contained in this map.");
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

    const value_type* data() const
    {
        return buckets_.data();
    }

    iterator invalid_iterator()
    {
        return buckets_.end();
    }

    const_iterator invalid_iterator() const
    {
        return buckets_.end();
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

    auto find_bucket(const key_type& key) const
    {
        QBB_ASSERT(key != empty_key_, "The empty key is not a valid key.");

        auto guessed_bucket_id = hasher()(key) % bucket_count();
        auto bucket_id = guessed_bucket_id;

        do
        {
            auto& bucket = buckets_[bucket_id];

            if (key_equal()(bucket.first, key))
            {
                return bucket_id;
            }

            bucket_id = (bucket_id + 1) % bucket_count();
        } while(bucket_id != guessed_bucket_id);

        return buckets_.size();
    }

    bool is_valid_bucket(std::size_t bucket_id) const
    {
        return bucket_id != buckets_.size();
    }

    key_type empty_key_;
    std::size_t num_of_used_buckets_;
    std::vector<value_type, Allocator> buckets_;
};
}
}

#endif