#ifndef QUBUS_UTIL_LFU_CACHE_HPP
#define QUBUS_UTIL_LFU_CACHE_HPP

#include <qubus/util/assert.hpp>

#include <boost/heap/fibonacci_heap.hpp>
#include <map>
#include <functional>
#include <tuple>
#include <utility>

namespace qubus
{
namespace util
{

// Class providing fixed-size (by number of records)
// LFU-replacement cache of a function with signature
// V f(K).
template <typename K, typename T>
class lfu_cache
{
public:
    using key_type = K;
    using value_type = T;

    lfu_cache(std::function<value_type(const key_type&)> fn_, std::size_t capacity_, long int threshold_)
    : fn_(std::move(fn_)), capacity_(std::move(capacity_)), threshold_(std::move(threshold_))
    {
        QBB_ASSERT(capacity_ != 0, "Cache capacity needs to be non-zero.");
        QBB_ASSERT(threshold_ >= 0, "Use count threashold needs to be positive.");
    }

    value_type operator()(const key_type& k) const
    {
        const auto it = key_entry_map_.find(k);

        if (it == key_entry_map_.end())
        {
            const value_type v = fn_(k);
            insert(k, v);

            return v;
        }
        else
        {
            auto handle = it->second;

            auto entry = *handle;

            auto current_use_count = std::get<0>(entry);

            if (current_use_count < threshold_)
            {
                ++std::get<0>(entry);
                eviction_queue_.increase(handle, entry);
            }

            return std::get<2>(entry);
        }
    }

private:
    struct eviction_queue_comperator
    {
        bool operator()(const std::tuple<long int, key_type, value_type>& lhs, const std::tuple<long int, key_type, value_type>& rhs) const
        {
            return std::get<0>(lhs) > std::get<0>(rhs);
        }
    };

    using evition_queue_type = boost::heap::fibonacci_heap<std::tuple<long int, key_type, value_type>, boost::heap::compare<eviction_queue_comperator>>;
    using key_entry_map_type = std::map<key_type, typename evition_queue_type::handle_type>;

    void insert(const key_type& k, const value_type& v) const
    {
        QBB_ASSERT(eviction_queue_.size() <= capacity_, "Cache entry count exceeds the capacity.");

        if (eviction_queue_.size() == capacity_)
        {
            key_entry_map_.erase(std::get<1>(eviction_queue_.top()));
            eviction_queue_.pop();
        }

        auto handle = eviction_queue_.push(std::make_tuple(1, k, v));
        key_entry_map_.insert(typename key_entry_map_type::value_type(k, handle));

        QBB_ASSERT(eviction_queue_.size() == key_entry_map_.size(), "Inconsistent number of cache entries.");
    }

    std::function<value_type(const key_type&)> fn_;
    std::size_t capacity_;
    long int threshold_;
    mutable evition_queue_type eviction_queue_;
    mutable key_entry_map_type key_entry_map_;
};

}
}

#endif
