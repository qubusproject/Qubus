/* This code is based on Malte Skarupke's fantastic flat hash map (https://github.com/skarupke/flat_hash_map).
 * The original code is licensed under the Boost Software License, Version 1.0. A copy of the entire
 * license can be found in external/licenses/LICENSE_1_0.txt.
 *
 * Our version mostly fixes some small quirks and adapts the code to match our style guide.
*/
#ifndef QUBUS_UTIL_DENSE_HASH_TABLE_HPP
#define QUBUS_UTIL_DENSE_HASH_TABLE_HPP

#include <qubus/util/noinline.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace qubus
{
namespace util
{

struct prime_number_hash_policy;
struct power_of_two_hash_policy;

namespace detail
{
template <typename Result, typename Functor>
struct functor_storage : Functor
{
    functor_storage() = default;

    explicit functor_storage(const Functor& functor) : Functor(functor)
    {
    }

    template <typename... Args>
    Result operator()(Args&&... args)
    {
        return static_cast<Functor&>(*this)(std::forward<Args>(args)...);
    }

    template <typename... Args>
    Result operator()(Args&&... args) const
    {
        return static_cast<const Functor&>(*this)(std::forward<Args>(args)...);
    }
};

template <typename Result, typename... Args>
struct functor_storage<Result, Result (*)(Args...)>
{
    using function_ptr = Result (*)(Args...);

    function_ptr function;

    explicit functor_storage(function_ptr function) : function(function)
    {
    }

    Result operator()(Args... args) const
    {
        return function(std::forward<Args>(args)...);
    }

    explicit operator function_ptr&()
    {
        return function;
    }

    explicit operator const function_ptr&()
    {
        return function;
    }
};

template <typename KeyType, typename ValueType, typename Hasher>
struct key_or_value_hasher : functor_storage<std::size_t, Hasher>
{
    using hasher_storage = functor_storage<std::size_t, Hasher>;

    key_or_value_hasher() = default;

    explicit key_or_value_hasher(const Hasher& hash) : hasher_storage(hash)
    {
    }

    std::size_t operator()(const KeyType& key)
    {
        return static_cast<hasher_storage&>(*this)(key);
    }

    std::size_t operator()(const KeyType& key) const
    {
        return static_cast<const hasher_storage&>(*this)(key);
    }

    std::size_t operator()(const ValueType& value)
    {
        return static_cast<hasher_storage&>(*this)(value.first);
    }

    std::size_t operator()(const ValueType& value) const
    {
        return static_cast<const hasher_storage&>(*this)(value.first);
    }

    template <typename F, typename S>
    std::size_t operator()(const std::pair<F, S>& value)
    {
        return static_cast<hasher_storage&>(*this)(value.first);
    }

    template <typename F, typename S>
    std::size_t operator()(const std::pair<F, S>& value) const
    {
        return static_cast<const hasher_storage&>(*this)(value.first);
    }
};

template <typename KeyType, typename ValueType, typename KeyEqual>
struct key_or_value_equality : functor_storage<bool, KeyEqual>
{
    using equality_storage = functor_storage<bool, KeyEqual>;

    key_or_value_equality() = default;

    explicit key_or_value_equality(const KeyEqual& equality) : equality_storage(equality)
    {
    }

    bool operator()(const KeyType& lhs, const KeyType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs, rhs);
    }

    bool operator()(const KeyType& lhs, const ValueType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs, rhs.first);
    }

    bool operator()(const ValueType& lhs, const KeyType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs);
    }

    bool operator()(const ValueType& lhs, const ValueType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs.first);
    }

    template <typename F, typename S>
    bool operator()(const KeyType& lhs, const std::pair<F, S>& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs, rhs.first);
    }

    template <typename F, typename S>
    bool operator()(const std::pair<F, S>& lhs, const KeyType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs);
    }

    template <typename F, typename S>
    bool operator()(const ValueType& lhs, const std::pair<F, S>& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs.first);
    }

    template <typename F, typename S>
    bool operator()(const std::pair<F, S>& lhs, const ValueType& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs.first);
    }

    template <typename FL, typename SL, typename FR, typename SR>
    bool operator()(const std::pair<FL, SL>& lhs, const std::pair<FR, SR>& rhs)
    {
        return static_cast<equality_storage&>(*this)(lhs.first, rhs.first);
    }
};

template <typename T>
struct dense_hash_table_entry
{
    static constexpr dense_hash_table_entry special_end_entry()
    {
        dense_hash_table_entry end;

        end.distance_from_desired = special_end_value;

        return end;
    }

    bool has_value() const
    {
        return distance_from_desired >= 0;
    }

    bool is_empty() const
    {
        return distance_from_desired < 0;
    }

    bool is_at_desired_position() const
    {
        return distance_from_desired <= 0;
    }

    template <typename... Args>
    void emplace(std::int8_t distance, Args&&... args)
    {
        new (std::addressof(value)) T(std::forward<Args>(args)...);

        distance_from_desired = distance;
    }

    void destroy_value()
    {
        value.~T();

        distance_from_desired = -1;
    }

    std::int8_t distance_from_desired = -1;

    static constexpr std::int8_t special_end_value = 0;

    union {
        T value;
    };
};

template <typename T>
struct dense_hash_table_entry_constexpr
{
    static constexpr dense_hash_table_entry_constexpr special_end_entry()
    {
        dense_hash_table_entry_constexpr end;

        end.distance_from_desired = dense_hash_table_entry<T>::special_end_value;

        return end;
    }

    std::int8_t distance_from_desired = -1;

    typename std::aligned_storage<sizeof(T), alignof(T)>::type bytes = {};
};

static constexpr std::int8_t min_lookups = 4;

inline std::int8_t log2(std::size_t value)
{
    static constexpr std::int8_t table[64] = {
        63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,  61, 51, 37, 40, 49, 18,
        28, 20, 55, 30, 34, 11, 43, 14, 22, 4,  62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19,
        29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5};

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;

    return table[((value - (value >> 1)) * 0x07EDD5E59A4E28C2) >> 58];
}

void throw_out_of_range();

template <typename T, bool>
struct assign_if_true
{
    void operator()(T& lhs, const T& rhs)
    {
        lhs = rhs;
    }
    void operator()(T& lhs, T&& rhs)
    {
        lhs = std::move(rhs);
    }
};

template <typename T>
struct assign_if_true<T, false>
{
    void operator()(T&, const T&)
    {
    }
    void operator()(T&, T&&)
    {
    }
};

constexpr std::size_t next_power_of_two(std::size_t i)
{
    --i;

    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;

    ++i;

    return i;
}

template <typename...>
using void_t = void;

template <typename T, typename = void>
struct hash_policy_selector
{
    using type = prime_number_hash_policy;
};

template <typename T>
struct hash_policy_selector<T, void_t<typename T::hash_policy>>
{
    using type = typename T::hash_policy;
};

template <typename T, typename FindKey, typename ArgumentHash, typename Hasher,
          typename ArgumentEqual, typename Equal, typename ArgumentAlloc, typename EntryAlloc>
class dense_hash_table : private EntryAlloc, private Hasher, private Equal
{
private:
    using entry_type = detail::dense_hash_table_entry<T>;
    using allocator_traits = std::allocator_traits<EntryAlloc>;
    using entry_pointer_type = typename allocator_traits::pointer;

    struct convertible_to_iterator;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = ArgumentHash;
    using key_equal = ArgumentEqual;
    using allocator_type = EntryAlloc;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    dense_hash_table()
    {
        setup_default_table();
    }

    explicit dense_hash_table(size_type bucket_count_, ArgumentHash hash_ = ArgumentHash(),
                              ArgumentEqual equal_ = ArgumentEqual(),
                              ArgumentAlloc alloc_ = ArgumentAlloc())
    : EntryAlloc(std::move(alloc_)), Hasher(std::move(hash_)), Equal(std::move(equal_))
    {
        rehash(bucket_count_);
    }

    dense_hash_table(size_type bucket_count_, ArgumentAlloc alloc_)
    : dense_hash_table(bucket_count_, ArgumentHash(), ArgumentEqual(), std::move(alloc_))
    {
    }

    dense_hash_table(size_type bucket_count_, ArgumentHash hash_, ArgumentAlloc alloc_)
    : dense_hash_table(bucket_count_, std::move(hash_), ArgumentEqual(), std::move(alloc_))
    {
    }

    explicit dense_hash_table(ArgumentAlloc alloc_)
    : dense_hash_table(0, ArgumentHash(), ArgumentEqual(), std::move(alloc_))
    {
    }

    template <typename It>
    dense_hash_table(It first_, It last_, size_type bucket_count_ = 0,
                     ArgumentHash hash_ = ArgumentHash(),
                     ArgumentEqual equal_ = ArgumentEqual(),
                     ArgumentAlloc alloc_ = ArgumentAlloc())
    : dense_hash_table(bucket_count_, std::move(hash_), std::move(equal_), std::move(alloc_))
    {
        insert(first_, last_);
    }

    template <typename It>
    dense_hash_table(It first_, It last_, size_type bucket_count_, ArgumentAlloc alloc_)
    : dense_hash_table(std::move(first_), std::move(last_), bucket_count_, ArgumentHash(), ArgumentEqual(), std::move(alloc_))
    {
    }

    template <typename It>
    dense_hash_table(It first_, It last_, size_type bucket_count_, ArgumentHash hash_,
                     ArgumentAlloc alloc_)
    : dense_hash_table(std::move(first_), std::move(last_), bucket_count_, std::move(hash_), ArgumentEqual(), std::move(alloc_))
    {
    }

    dense_hash_table(std::initializer_list<T> il_, size_type bucket_count_ = 0,
                     const ArgumentHash& hash_ = ArgumentHash(),
                     const ArgumentEqual& equal_ = ArgumentEqual(),
                     const ArgumentAlloc& alloc_ = ArgumentAlloc())
    : dense_hash_table(bucket_count_, std::move(hash_), std::move(equal_), std::move(alloc_))
    {
        if (bucket_count_ == 0)
            rehash(il_.size());

        insert(il_.begin(), il_.end());
    }

    dense_hash_table(std::initializer_list<T> il_, size_type bucket_count_,
                     ArgumentAlloc alloc_)
    : dense_hash_table(il_, bucket_count_, ArgumentHash(), ArgumentEqual(), std::move(alloc_))
    {
    }

    dense_hash_table(std::initializer_list<T> il_, size_type bucket_count_, ArgumentHash hash_,
                     ArgumentAlloc alloc_)
    : dense_hash_table(il_, bucket_count_, std::move(hash_), ArgumentEqual(), std::move(alloc_))
    {
    }

    dense_hash_table(const dense_hash_table& other)
    : dense_hash_table(
          other, allocator_traits::select_on_container_copy_construction(other.get_allocator()))
    {
    }

    dense_hash_table(const dense_hash_table& other, const ArgumentAlloc& alloc)
    : EntryAlloc(alloc), Hasher(other), Equal(other), max_load_factor_(other.max_load_factor_)
    {
        rehash_for_other_container(other);

        try
        {
            insert(other.begin(), other.end());
        }
        catch (...)
        {
            clear();

            deallocate_data(entries_, num_slots_minus_one_, max_lookups_);

            throw;
        }
    }

    dense_hash_table(dense_hash_table&& other) noexcept
    : EntryAlloc(std::move(other)), Hasher(std::move(other)), Equal(std::move(other))
    {
        swap_pointers(other);
    }

    dense_hash_table(dense_hash_table&& other, const ArgumentAlloc& alloc) noexcept
    : EntryAlloc(std::move(alloc)), Hasher(std::move(other)), Equal(std::move(other))
    {
        swap_pointers(other);
    }

    dense_hash_table& operator=(const dense_hash_table& other)
    {
        if (this == std::addressof(other))
            return *this;

        clear();

        if (allocator_traits::propagate_on_container_copy_assignment::value)
        {
            if (static_cast<EntryAlloc&>(*this) != static_cast<const EntryAlloc&>(other))
            {
                reset_to_empty_state();
            }

            assign_if_true<EntryAlloc,
                           allocator_traits::propagate_on_container_copy_assignment::value>()(
                *this, other);
        }

        max_load_factor_ = other.max_load_factor_;
        static_cast<Hasher&>(*this) = other;
        static_cast<Equal&>(*this) = other;
        rehash_for_other_container(other);
        insert(other.begin(), other.end());

        return *this;
    }

    dense_hash_table& operator=(dense_hash_table&& other) noexcept
    {
        if (this == std::addressof(other))
            return *this;

        if (allocator_traits::propagate_on_container_move_assignment::value)
        {
            clear();
            reset_to_empty_state();
            assign_if_true<EntryAlloc,
                           allocator_traits::propagate_on_container_move_assignment::value>()(
                *this, std::move(other));
            swap_pointers(other);
        }
        else if (static_cast<EntryAlloc&>(*this) == static_cast<EntryAlloc&>(other))
        {
            swap_pointers(other);
        }
        else
        {
            clear();
            max_load_factor_ = other.max_load_factor_;
            rehash_for_other_container(other);
            for (T& elem : other)
                emplace(std::move(elem));
            other.clear();
        }

        static_cast<Hasher&>(*this) = std::move(other);
        static_cast<Equal&>(*this) = std::move(other);

        return *this;
    }

    ~dense_hash_table()
    {
        clear();

        deallocate_data(entries_, num_slots_minus_one_, max_lookups_);
    }

    const allocator_type& get_allocator() const
    {
        return static_cast<const allocator_type&>(*this);
    }
    const ArgumentEqual& key_eq() const
    {
        return static_cast<const ArgumentEqual&>(*this);
    }
    const ArgumentHash& hash_function() const
    {
        return static_cast<const ArgumentHash&>(*this);
    }

    template <typename ValueType>
    struct templated_iterator
    {
        entry_pointer_type current = entry_pointer_type();

        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        friend bool operator==(const templated_iterator& lhs, const templated_iterator& rhs)
        {
            return lhs.current == rhs.current;
        }

        friend bool operator!=(const templated_iterator& lhs, const templated_iterator& rhs)
        {
            return !(lhs == rhs);
        }

        templated_iterator& operator++()
        {
            do
            {
                ++current;
            } while (current->is_empty());

            return *this;
        }

        templated_iterator operator++(int)
        {
            templated_iterator copy(*this);

            ++*this;

            return copy;
        }

        ValueType& operator*() const
        {
            return current->value;
        }

        ValueType* operator->() const
        {
            return std::addressof(current->value);
        }

        operator templated_iterator<const value_type>() const
        {
            return {current};
        }
    };

    using iterator = templated_iterator<value_type>;
    using const_iterator = templated_iterator<const value_type>;

    iterator begin()
    {
        for (entry_pointer_type it = entries_;; ++it)
        {
            if (it->has_value())
                return {it};
        }
    }

    const_iterator begin() const
    {
        for (entry_pointer_type it = entries_;; ++it)
        {
            if (it->has_value())
                return {it};
        }
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    iterator end()
    {
        return {entries_ + static_cast<ptrdiff_t>(num_slots_minus_one_ + max_lookups_)};
    }

    const_iterator end() const
    {
        return {entries_ + static_cast<ptrdiff_t>(num_slots_minus_one_ + max_lookups_)};
    }

    const_iterator cend() const
    {
        return end();
    }

    iterator find(const FindKey& key)
    {
        std::size_t index = hash_policy_.index_for_hash(hash_object(key), num_slots_minus_one_);
        entry_pointer_type it = entries_ + ptrdiff_t(index);

        for (std::int8_t distance = 0; it->distance_from_desired >= distance; ++distance, ++it)
        {
            if (compares_equal(key, it->value))
                return {it};
        }

        return end();
    }

    const_iterator find(const FindKey& key) const
    {
        return const_cast<dense_hash_table*>(this)->find(key);
    }

    std::size_t count(const FindKey& key) const
    {
        return find(key) == end() ? 0 : 1;
    }

    std::pair<iterator, iterator> equal_range(const FindKey& key)
    {
        iterator found = find(key);

        if (found == end())
            return {found, found};

        return {found, std::next(found)};
    }

    std::pair<const_iterator, const_iterator> equal_range(const FindKey& key) const
    {
        const_iterator found = find(key);

        if (found == end())
            return {found, found};

        return {found, std::next(found)};
    }

    template <typename Key, typename... Args>
    std::pair<iterator, bool> emplace(Key&& key, Args&&... args)
    {
        std::size_t index = hash_policy_.index_for_hash(hash_object(key), num_slots_minus_one_);
        entry_pointer_type current_entry = entries_ + ptrdiff_t(index);
        std::int8_t distance_from_desired = 0;

        for (; current_entry->distance_from_desired >= distance_from_desired;
             ++current_entry, ++distance_from_desired)
        {
            if (compares_equal(key, current_entry->value))
                return {{current_entry}, false};
        }

        return emplace_new_key(distance_from_desired, current_entry, std::forward<Key>(key),
                               std::forward<Args>(args)...);
    }

    std::pair<iterator, bool> insert(const value_type& value)
    {
        return emplace(value);
    }

    std::pair<iterator, bool> insert(value_type&& value)
    {
        return emplace(std::move(value));
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator /*unused*/, Args&&... args)
    {
        return emplace(std::forward<Args>(args)...).first;
    }

    iterator insert(const_iterator /*unused*/, const value_type& value)
    {
        return emplace(value).first;
    }

    iterator insert(const_iterator /*unused*/, value_type&& value)
    {
        return emplace(std::move(value)).first;
    }

    template <typename It>
    void insert(It begin, It end)
    {
        for (; begin != end; ++begin)
        {
            emplace(*begin);
        }
    }

    void insert(std::initializer_list<value_type> il)
    {
        insert(il.begin(), il.end());
    }

    void rehash(std::size_t num_buckets)
    {
        num_buckets =
            std::max(num_buckets, static_cast<std::size_t>(std::ceil(
                                      num_elements_ / static_cast<double>(max_load_factor_))));

        if (num_buckets == 0)
        {
            reset_to_empty_state();
            return;
        }

        auto new_prime_index = hash_policy_.next_size_over(num_buckets);

        if (num_buckets == bucket_count())
            return;

        std::int8_t new_max_lookups = compute_max_lookups(num_buckets);
        entry_pointer_type new_buckets(
            allocator_traits::allocate(*this, num_buckets + new_max_lookups));

        for (entry_pointer_type
                 it = new_buckets,
                 real_end = it + static_cast<ptrdiff_t>(num_buckets + new_max_lookups - 1);
             it != real_end; ++it)
        {
            it->distance_from_desired = -1;
        }

        new_buckets[num_buckets + new_max_lookups - 1].distance_from_desired =
            entry_type::special_end_value;

        std::swap(entries_, new_buckets);
        std::swap(num_slots_minus_one_, num_buckets);

        --num_slots_minus_one_;

        hash_policy_.commit(new_prime_index);

        std::int8_t old_max_lookups = max_lookups_;
        max_lookups_ = new_max_lookups;
        num_elements_ = 0;

        for (entry_pointer_type it = new_buckets,
                                end = it + static_cast<ptrdiff_t>(num_buckets + old_max_lookups);
             it != end; ++it)
        {
            if (it->has_value())
            {
                emplace(std::move(it->value));
                it->destroy_value();
            }
        }

        deallocate_data(new_buckets, num_buckets, old_max_lookups);
    }

    void reserve(std::size_t num_elements)
    {
        std::size_t required_buckets = num_buckets_for_reserve(num_elements);

        if (required_buckets > bucket_count())
            rehash(required_buckets);
    }

    // the return value is a type that can be converted to an iterator
    // the reason for doing this is that it's not free to find the
    // iterator pointing at the next element. if you care about the
    // next iterator, turn the return value into an iterator
    convertible_to_iterator erase(const_iterator to_erase)
    {
        entry_pointer_type current = to_erase.current;
        current->destroy_value();

        --num_elements_;

        for (entry_pointer_type next = current + ptrdiff_t(1); !next->is_at_desired_position();
             ++current, ++next)
        {
            current->emplace(next->distance_from_desired - 1, std::move(next->value));
            next->destroy_value();
        }

        return {to_erase.current};
    }

    iterator erase(const_iterator begin_it, const_iterator end_it)
    {
        for (entry_pointer_type it = begin_it.current, end = end_it.current; it != end; ++it)
        {
            if (it->has_value())
            {
                it->destroy_value();
                --num_elements_;
            }
        }

        if (end_it == this->end())
            return this->end();

        ptrdiff_t num_to_move =
            std::min(static_cast<ptrdiff_t>(end_it.current->distance_from_desired),
                     end_it.current - begin_it.current);

        entry_pointer_type to_return = end_it.current - num_to_move;

        for (entry_pointer_type it = end_it.current; !it->is_at_desired_position();)
        {
            entry_pointer_type target = it - num_to_move;
            target->emplace(it->distance_from_desired - num_to_move, std::move(it->value));
            it->destroy_value();
            ++it;
            num_to_move = std::min(static_cast<ptrdiff_t>(it->distance_from_desired), num_to_move);
        }

        return {to_return};
    }

    std::size_t erase(const FindKey& key)
    {
        auto found = find(key);

        if (found == end())
            return 0;

        erase(found);
        return 1;
    }

    void clear()
    {
        for (entry_pointer_type
                 it = entries_,
                 end = it + static_cast<ptrdiff_t>(num_slots_minus_one_ + max_lookups_);
             it != end; ++it)
        {
            if (it->has_value())
                it->destroy_value();
        }

        num_elements_ = 0;
    }

    void shrink_to_fit()
    {
        rehash_for_other_container(*this);
    }

    void swap(dense_hash_table& other)
    {
        using std::swap;

        swap_pointers(other);
        swap(static_cast<ArgumentHash&>(*this), static_cast<ArgumentHash&>(other));
        swap(static_cast<ArgumentEqual&>(*this), static_cast<ArgumentEqual&>(other));

        if (allocator_traits::propagate_on_container_swap::value)
            swap(static_cast<EntryAlloc&>(*this), static_cast<EntryAlloc&>(other));
    }

    std::size_t size() const
    {
        return num_elements_;
    }

    std::size_t max_size() const
    {
        return (allocator_traits::max_size(*this)) / sizeof(entry_type);
    }

    std::size_t bucket_count() const
    {
        return num_slots_minus_one_ + 1;
    }

    size_type max_bucket_count() const
    {
        return (allocator_traits::max_size(*this) - min_lookups) / sizeof(entry_type);
    }

    std::size_t bucket(const FindKey& key) const
    {
        return hash_policy_.index_for_hash(hash_object(key), num_slots_minus_one_);
    }

    float load_factor() const
    {
        std::size_t buckets = bucket_count();

        if (buckets > 0)
            return static_cast<float>(num_elements_) / bucket_count();

        return 0;
    }

    void max_load_factor(float value)
    {
        max_load_factor_ = value;
    }

    float max_load_factor() const
    {
        return max_load_factor_;
    }

    bool empty() const
    {
        return num_elements_ == 0;
    }

private:
    entry_pointer_type entries_;
    std::size_t num_slots_minus_one_ = 0;
    typename hash_policy_selector<ArgumentHash>::type hash_policy_;
    std::int8_t max_lookups_ = detail::min_lookups - 1;
    float max_load_factor_ = 0.5f;
    std::size_t num_elements_ = 0;

    static std::int8_t compute_max_lookups(std::size_t num_buckets)
    {
        std::int8_t desired = detail::log2(num_buckets);

        return std::max(detail::min_lookups, desired);
    }

    std::size_t num_buckets_for_reserve(std::size_t num_elements) const
    {
        return static_cast<std::size_t>(
            std::ceil(num_elements / std::min(0.5, static_cast<double>(max_load_factor_))));
    }

    void rehash_for_other_container(const dense_hash_table& other)
    {
        rehash(std::min(num_buckets_for_reserve(other.size()), other.bucket_count()));
    }

    void swap_pointers(dense_hash_table& other)
    {
        using std::swap;

        swap(hash_policy_, other.hash_policy_);
        swap(entries_, other.entries_);
        swap(num_slots_minus_one_, other.num_slots_minus_one_);
        swap(num_elements_, other.num_elements_);
        swap(max_lookups_, other.max_lookups_);
        swap(max_load_factor_, other.max_load_factor_);
    }

    template <typename Key, typename... Args>
    QUBUS_NOINLINE(std::pair<iterator, bool>)
    emplace_new_key(std::int8_t distance_from_desired, entry_pointer_type current_entry, Key&& key,
                    Args&&... args)
    {
        using std::swap;

        if (num_slots_minus_one_ == 0 || distance_from_desired == max_lookups_ ||
            static_cast<double>(num_elements_ + 1) / static_cast<double>(bucket_count()) >
                max_load_factor_)
        {
            grow();
            return emplace(std::forward<Key>(key), std::forward<Args>(args)...);
        }

        if (current_entry->is_empty())
        {
            current_entry->emplace(distance_from_desired, std::forward<Key>(key),
                                   std::forward<Args>(args)...);
            ++num_elements_;
            return {{current_entry}, true};
        }

        value_type to_insert(std::forward<Key>(key), std::forward<Args>(args)...);
        swap(distance_from_desired, current_entry->distance_from_desired);
        swap(to_insert, current_entry->value);
        iterator result = {current_entry};

        for (++distance_from_desired, ++current_entry;; ++current_entry)
        {
            if (current_entry->is_empty())
            {
                current_entry->emplace(distance_from_desired, std::move(to_insert));
                ++num_elements_;
                return {result, true};
            }

            if (current_entry->distance_from_desired < distance_from_desired)
            {
                swap(distance_from_desired, current_entry->distance_from_desired);
                swap(to_insert, current_entry->value);
                ++distance_from_desired;
            }
            else
            {
                ++distance_from_desired;
                if (distance_from_desired == max_lookups_)
                {
                    swap(to_insert, result.current->value);
                    grow();
                    return emplace(std::move(to_insert));
                }
            }
        }
    }

    void grow()
    {
        rehash(std::max(std::size_t(4), 2 * bucket_count()));
    }

    void deallocate_data(entry_pointer_type begin, std::size_t num_slots_minus_one,
                         std::int8_t max_lookups)
    {
        allocator_traits::deallocate(*this, begin, num_slots_minus_one + max_lookups + 1);
    }

    void setup_default_table()
    {
        entry_pointer_type new_buckets = allocator_traits::allocate(*this, min_lookups);

        for (entry_pointer_type it = new_buckets,
                                real_end = it + static_cast<ptrdiff_t>(min_lookups - 1);
             it != real_end; ++it)
        {
            it->distance_from_desired = -1;
        }

        new_buckets[min_lookups - 1].distance_from_desired = entry_type::special_end_value;

        entries_ = new_buckets;
    }

    void reset_to_empty_state()
    {
        deallocate_data(entries_, num_slots_minus_one_, max_lookups_);

        setup_default_table();
        num_slots_minus_one_ = 0;
        hash_policy_.reset();
        max_lookups_ = detail::min_lookups - 1;
    }

    template <typename U>
    std::size_t hash_object(const U& key)
    {
        return static_cast<Hasher&>(*this)(key);
    }

    template <typename U>
    std::size_t hash_object(const U& key) const
    {
        return static_cast<const Hasher&>(*this)(key);
    }

    template <typename L, typename R>
    bool compares_equal(const L& lhs, const R& rhs)
    {
        return static_cast<Equal&>(*this)(lhs, rhs);
    }

    struct convertible_to_iterator
    {
        entry_pointer_type it;

        explicit operator iterator()
        {
            if (it->has_value())
                return {it};

            return ++iterator{it};
        }

        explicit operator const_iterator()
        {
            if (it->has_value())
                return {it};

            return ++const_iterator{it};
        }
    };
};
}

struct prime_number_hash_policy
{
    static std::size_t mod0(std::size_t /*unused*/)
    {
        return 0llu;
    }

    static std::size_t mod2(std::size_t hash)
    {
        return hash % 2llu;
    }

    static std::size_t mod3(std::size_t hash)
    {
        return hash % 3llu;
    }

    static std::size_t mod5(std::size_t hash)
    {
        return hash % 5llu;
    }

    static std::size_t mod7(std::size_t hash)
    {
        return hash % 7llu;
    }

    static std::size_t mod11(std::size_t hash)
    {
        return hash % 11llu;
    }

    static std::size_t mod13(std::size_t hash)
    {
        return hash % 13llu;
    }

    static std::size_t mod17(std::size_t hash)
    {
        return hash % 17llu;
    }

    static std::size_t mod23(std::size_t hash)
    {
        return hash % 23llu;
    }

    static std::size_t mod29(std::size_t hash)
    {
        return hash % 29llu;
    }

    static std::size_t mod37(std::size_t hash)
    {
        return hash % 37llu;
    }

    static std::size_t mod47(std::size_t hash)
    {
        return hash % 47llu;
    }

    static std::size_t mod59(std::size_t hash)
    {
        return hash % 59llu;
    }

    static std::size_t mod73(std::size_t hash)
    {
        return hash % 73llu;
    }

    static std::size_t mod97(std::size_t hash)
    {
        return hash % 97llu;
    }

    static std::size_t mod127(std::size_t hash)
    {
        return hash % 127llu;
    }

    static std::size_t mod151(std::size_t hash)
    {
        return hash % 151llu;
    }

    static std::size_t mod197(std::size_t hash)
    {
        return hash % 197llu;
    }

    static std::size_t mod251(std::size_t hash)
    {
        return hash % 251llu;
    }

    static std::size_t mod313(std::size_t hash)
    {
        return hash % 313llu;
    }

    static std::size_t mod397(std::size_t hash)
    {
        return hash % 397llu;
    }

    static std::size_t mod499(std::size_t hash)
    {
        return hash % 499llu;
    }

    static std::size_t mod631(std::size_t hash)
    {
        return hash % 631llu;
    }

    static std::size_t mod797(std::size_t hash)
    {
        return hash % 797llu;
    }

    static std::size_t mod1009(std::size_t hash)
    {
        return hash % 1009llu;
    }

    static std::size_t mod1259(std::size_t hash)
    {
        return hash % 1259llu;
    }

    static std::size_t mod1597(std::size_t hash)
    {
        return hash % 1597llu;
    }

    static std::size_t mod2011(std::size_t hash)
    {
        return hash % 2011llu;
    }

    static std::size_t mod2539(std::size_t hash)
    {
        return hash % 2539llu;
    }

    static std::size_t mod3203(std::size_t hash)
    {
        return hash % 3203llu;
    }

    static std::size_t mod4027(std::size_t hash)
    {
        return hash % 4027llu;
    }

    static std::size_t mod5087(std::size_t hash)
    {
        return hash % 5087llu;
    }

    static std::size_t mod6421(std::size_t hash)
    {
        return hash % 6421llu;
    }

    static std::size_t mod8089(std::size_t hash)
    {
        return hash % 8089llu;
    }

    static std::size_t mod10193(std::size_t hash)
    {
        return hash % 10193llu;
    }

    static std::size_t mod12853(std::size_t hash)
    {
        return hash % 12853llu;
    }

    static std::size_t mod16193(std::size_t hash)
    {
        return hash % 16193llu;
    }

    static std::size_t mod20399(std::size_t hash)
    {
        return hash % 20399llu;
    }

    static std::size_t mod25717(std::size_t hash)
    {
        return hash % 25717llu;
    }

    static std::size_t mod32401(std::size_t hash)
    {
        return hash % 32401llu;
    }

    static std::size_t mod40823(std::size_t hash)
    {
        return hash % 40823llu;
    }

    static std::size_t mod51437(std::size_t hash)
    {
        return hash % 51437llu;
    }

    static std::size_t mod64811(std::size_t hash)
    {
        return hash % 64811llu;
    }

    static std::size_t mod81649(std::size_t hash)
    {
        return hash % 81649llu;
    }

    static std::size_t mod102877(std::size_t hash)
    {
        return hash % 102877llu;
    }

    static std::size_t mod129607(std::size_t hash)
    {
        return hash % 129607llu;
    }

    static std::size_t mod163307(std::size_t hash)
    {
        return hash % 163307llu;
    }

    static std::size_t mod205759(std::size_t hash)
    {
        return hash % 205759llu;
    }

    static std::size_t mod259229(std::size_t hash)
    {
        return hash % 259229llu;
    }

    static std::size_t mod326617(std::size_t hash)
    {
        return hash % 326617llu;
    }

    static std::size_t mod411527(std::size_t hash)
    {
        return hash % 411527llu;
    }

    static std::size_t mod518509(std::size_t hash)
    {
        return hash % 518509llu;
    }

    static std::size_t mod653267(std::size_t hash)
    {
        return hash % 653267llu;
    }

    static std::size_t mod823117(std::size_t hash)
    {
        return hash % 823117llu;
    }

    static std::size_t mod1037059(std::size_t hash)
    {
        return hash % 1037059llu;
    }

    static std::size_t mod1306601(std::size_t hash)
    {
        return hash % 1306601llu;
    }

    static std::size_t mod1646237(std::size_t hash)
    {
        return hash % 1646237llu;
    }

    static std::size_t mod2074129(std::size_t hash)
    {
        return hash % 2074129llu;
    }

    static std::size_t mod2613229(std::size_t hash)
    {
        return hash % 2613229llu;
    }

    static std::size_t mod3292489(std::size_t hash)
    {
        return hash % 3292489llu;
    }

    static std::size_t mod4148279(std::size_t hash)
    {
        return hash % 4148279llu;
    }

    static std::size_t mod5226491(std::size_t hash)
    {
        return hash % 5226491llu;
    }

    static std::size_t mod6584983(std::size_t hash)
    {
        return hash % 6584983llu;
    }

    static std::size_t mod8296553(std::size_t hash)
    {
        return hash % 8296553llu;
    }

    static std::size_t mod10453007(std::size_t hash)
    {
        return hash % 10453007llu;
    }

    static std::size_t mod13169977(std::size_t hash)
    {
        return hash % 13169977llu;
    }

    static std::size_t mod16593127(std::size_t hash)
    {
        return hash % 16593127llu;
    }

    static std::size_t mod20906033(std::size_t hash)
    {
        return hash % 20906033llu;
    }

    static std::size_t mod26339969(std::size_t hash)
    {
        return hash % 26339969llu;
    }

    static std::size_t mod33186281(std::size_t hash)
    {
        return hash % 33186281llu;
    }

    static std::size_t mod41812097(std::size_t hash)
    {
        return hash % 41812097llu;
    }

    static std::size_t mod52679969(std::size_t hash)
    {
        return hash % 52679969llu;
    }

    static std::size_t mod66372617(std::size_t hash)
    {
        return hash % 66372617llu;
    }

    static std::size_t mod83624237(std::size_t hash)
    {
        return hash % 83624237llu;
    }

    static std::size_t mod105359939(std::size_t hash)
    {
        return hash % 105359939llu;
    }

    static std::size_t mod132745199(std::size_t hash)
    {
        return hash % 132745199llu;
    }

    static std::size_t mod167248483(std::size_t hash)
    {
        return hash % 167248483llu;
    }

    static std::size_t mod210719881(std::size_t hash)
    {
        return hash % 210719881llu;
    }

    static std::size_t mod265490441(std::size_t hash)
    {
        return hash % 265490441llu;
    }

    static std::size_t mod334496971(std::size_t hash)
    {
        return hash % 334496971llu;
    }

    static std::size_t mod421439783(std::size_t hash)
    {
        return hash % 421439783llu;
    }

    static std::size_t mod530980861(std::size_t hash)
    {
        return hash % 530980861llu;
    }

    static std::size_t mod668993977(std::size_t hash)
    {
        return hash % 668993977llu;
    }

    static std::size_t mod842879579(std::size_t hash)
    {
        return hash % 842879579llu;
    }

    static std::size_t mod1061961721(std::size_t hash)
    {
        return hash % 1061961721llu;
    }

    static std::size_t mod1337987929(std::size_t hash)
    {
        return hash % 1337987929llu;
    }

    static std::size_t mod1685759167(std::size_t hash)
    {
        return hash % 1685759167llu;
    }

    static std::size_t mod2123923447(std::size_t hash)
    {
        return hash % 2123923447llu;
    }

    static std::size_t mod2675975881(std::size_t hash)
    {
        return hash % 2675975881llu;
    }

    static std::size_t mod3371518343(std::size_t hash)
    {
        return hash % 3371518343llu;
    }

    static std::size_t mod4247846927(std::size_t hash)
    {
        return hash % 4247846927llu;
    }

    static std::size_t mod5351951779(std::size_t hash)
    {
        return hash % 5351951779llu;
    }

    static std::size_t mod6743036717(std::size_t hash)
    {
        return hash % 6743036717llu;
    }

    static std::size_t mod8495693897(std::size_t hash)
    {
        return hash % 8495693897llu;
    }

    static std::size_t mod10703903591(std::size_t hash)
    {
        return hash % 10703903591llu;
    }

    static std::size_t mod13486073473(std::size_t hash)
    {
        return hash % 13486073473llu;
    }

    static std::size_t mod16991387857(std::size_t hash)
    {
        return hash % 16991387857llu;
    }

    static std::size_t mod21407807219(std::size_t hash)
    {
        return hash % 21407807219llu;
    }

    static std::size_t mod26972146961(std::size_t hash)
    {
        return hash % 26972146961llu;
    }

    static std::size_t mod33982775741(std::size_t hash)
    {
        return hash % 33982775741llu;
    }

    static std::size_t mod42815614441(std::size_t hash)
    {
        return hash % 42815614441llu;
    }

    static std::size_t mod53944293929(std::size_t hash)
    {
        return hash % 53944293929llu;
    }

    static std::size_t mod67965551447(std::size_t hash)
    {
        return hash % 67965551447llu;
    }

    static std::size_t mod85631228929(std::size_t hash)
    {
        return hash % 85631228929llu;
    }

    static std::size_t mod107888587883(std::size_t hash)
    {
        return hash % 107888587883llu;
    }

    static std::size_t mod135931102921(std::size_t hash)
    {
        return hash % 135931102921llu;
    }

    static std::size_t mod171262457903(std::size_t hash)
    {
        return hash % 171262457903llu;
    }

    static std::size_t mod215777175787(std::size_t hash)
    {
        return hash % 215777175787llu;
    }

    static std::size_t mod271862205833(std::size_t hash)
    {
        return hash % 271862205833llu;
    }

    static std::size_t mod342524915839(std::size_t hash)
    {
        return hash % 342524915839llu;
    }

    static std::size_t mod431554351609(std::size_t hash)
    {
        return hash % 431554351609llu;
    }

    static std::size_t mod543724411781(std::size_t hash)
    {
        return hash % 543724411781llu;
    }

    static std::size_t mod685049831731(std::size_t hash)
    {
        return hash % 685049831731llu;
    }

    static std::size_t mod863108703229(std::size_t hash)
    {
        return hash % 863108703229llu;
    }

    static std::size_t mod1087448823553(std::size_t hash)
    {
        return hash % 1087448823553llu;
    }

    static std::size_t mod1370099663459(std::size_t hash)
    {
        return hash % 1370099663459llu;
    }

    static std::size_t mod1726217406467(std::size_t hash)
    {
        return hash % 1726217406467llu;
    }

    static std::size_t mod2174897647073(std::size_t hash)
    {
        return hash % 2174897647073llu;
    }

    static std::size_t mod2740199326961(std::size_t hash)
    {
        return hash % 2740199326961llu;
    }

    static std::size_t mod3452434812973(std::size_t hash)
    {
        return hash % 3452434812973llu;
    }

    static std::size_t mod4349795294267(std::size_t hash)
    {
        return hash % 4349795294267llu;
    }

    static std::size_t mod5480398654009(std::size_t hash)
    {
        return hash % 5480398654009llu;
    }

    static std::size_t mod6904869625999(std::size_t hash)
    {
        return hash % 6904869625999llu;
    }

    static std::size_t mod8699590588571(std::size_t hash)
    {
        return hash % 8699590588571llu;
    }

    static std::size_t mod10960797308051(std::size_t hash)
    {
        return hash % 10960797308051llu;
    }

    static std::size_t mod13809739252051(std::size_t hash)
    {
        return hash % 13809739252051llu;
    }

    static std::size_t mod17399181177241(std::size_t hash)
    {
        return hash % 17399181177241llu;
    }

    static std::size_t mod21921594616111(std::size_t hash)
    {
        return hash % 21921594616111llu;
    }

    static std::size_t mod27619478504183(std::size_t hash)
    {
        return hash % 27619478504183llu;
    }

    static std::size_t mod34798362354533(std::size_t hash)
    {
        return hash % 34798362354533llu;
    }

    static std::size_t mod43843189232363(std::size_t hash)
    {
        return hash % 43843189232363llu;
    }

    static std::size_t mod55238957008387(std::size_t hash)
    {
        return hash % 55238957008387llu;
    }

    static std::size_t mod69596724709081(std::size_t hash)
    {
        return hash % 69596724709081llu;
    }

    static std::size_t mod87686378464759(std::size_t hash)
    {
        return hash % 87686378464759llu;
    }

    static std::size_t mod110477914016779(std::size_t hash)
    {
        return hash % 110477914016779llu;
    }

    static std::size_t mod139193449418173(std::size_t hash)
    {
        return hash % 139193449418173llu;
    }

    static std::size_t mod175372756929481(std::size_t hash)
    {
        return hash % 175372756929481llu;
    }

    static std::size_t mod220955828033581(std::size_t hash)
    {
        return hash % 220955828033581llu;
    }

    static std::size_t mod278386898836457(std::size_t hash)
    {
        return hash % 278386898836457llu;
    }

    static std::size_t mod350745513859007(std::size_t hash)
    {
        return hash % 350745513859007llu;
    }

    static std::size_t mod441911656067171(std::size_t hash)
    {
        return hash % 441911656067171llu;
    }

    static std::size_t mod556773797672909(std::size_t hash)
    {
        return hash % 556773797672909llu;
    }

    static std::size_t mod701491027718027(std::size_t hash)
    {
        return hash % 701491027718027llu;
    }

    static std::size_t mod883823312134381(std::size_t hash)
    {
        return hash % 883823312134381llu;
    }

    static std::size_t mod1113547595345903(std::size_t hash)
    {
        return hash % 1113547595345903llu;
    }

    static std::size_t mod1402982055436147(std::size_t hash)
    {
        return hash % 1402982055436147llu;
    }

    static std::size_t mod1767646624268779(std::size_t hash)
    {
        return hash % 1767646624268779llu;
    }

    static std::size_t mod2227095190691797(std::size_t hash)
    {
        return hash % 2227095190691797llu;
    }

    static std::size_t mod2805964110872297(std::size_t hash)
    {
        return hash % 2805964110872297llu;
    }

    static std::size_t mod3535293248537579(std::size_t hash)
    {
        return hash % 3535293248537579llu;
    }

    static std::size_t mod4454190381383713(std::size_t hash)
    {
        return hash % 4454190381383713llu;
    }

    static std::size_t mod5611928221744609(std::size_t hash)
    {
        return hash % 5611928221744609llu;
    }

    static std::size_t mod7070586497075177(std::size_t hash)
    {
        return hash % 7070586497075177llu;
    }

    static std::size_t mod8908380762767489(std::size_t hash)
    {
        return hash % 8908380762767489llu;
    }

    static std::size_t mod11223856443489329(std::size_t hash)
    {
        return hash % 11223856443489329llu;
    }

    static std::size_t mod14141172994150357(std::size_t hash)
    {
        return hash % 14141172994150357llu;
    }

    static std::size_t mod17816761525534927(std::size_t hash)
    {
        return hash % 17816761525534927llu;
    }

    static std::size_t mod22447712886978529(std::size_t hash)
    {
        return hash % 22447712886978529llu;
    }

    static std::size_t mod28282345988300791(std::size_t hash)
    {
        return hash % 28282345988300791llu;
    }

    static std::size_t mod35633523051069991(std::size_t hash)
    {
        return hash % 35633523051069991llu;
    }

    static std::size_t mod44895425773957261(std::size_t hash)
    {
        return hash % 44895425773957261llu;
    }

    static std::size_t mod56564691976601587(std::size_t hash)
    {
        return hash % 56564691976601587llu;
    }

    static std::size_t mod71267046102139967(std::size_t hash)
    {
        return hash % 71267046102139967llu;
    }

    static std::size_t mod89790851547914507(std::size_t hash)
    {
        return hash % 89790851547914507llu;
    }

    static std::size_t mod113129383953203213(std::size_t hash)
    {
        return hash % 113129383953203213llu;
    }

    static std::size_t mod142534092204280003(std::size_t hash)
    {
        return hash % 142534092204280003llu;
    }

    static std::size_t mod179581703095829107(std::size_t hash)
    {
        return hash % 179581703095829107llu;
    }

    static std::size_t mod226258767906406483(std::size_t hash)
    {
        return hash % 226258767906406483llu;
    }

    static std::size_t mod285068184408560057(std::size_t hash)
    {
        return hash % 285068184408560057llu;
    }

    static std::size_t mod359163406191658253(std::size_t hash)
    {
        return hash % 359163406191658253llu;
    }

    static std::size_t mod452517535812813007(std::size_t hash)
    {
        return hash % 452517535812813007llu;
    }

    static std::size_t mod570136368817120201(std::size_t hash)
    {
        return hash % 570136368817120201llu;
    }

    static std::size_t mod718326812383316683(std::size_t hash)
    {
        return hash % 718326812383316683llu;
    }

    static std::size_t mod905035071625626043(std::size_t hash)
    {
        return hash % 905035071625626043llu;
    }

    static std::size_t mod1140272737634240411(std::size_t hash)
    {
        return hash % 1140272737634240411llu;
    }

    static std::size_t mod1436653624766633509(std::size_t hash)
    {
        return hash % 1436653624766633509llu;
    }

    static std::size_t mod1810070143251252131(std::size_t hash)
    {
        return hash % 1810070143251252131llu;
    }

    static std::size_t mod2280545475268481167(std::size_t hash)
    {
        return hash % 2280545475268481167llu;
    }

    static std::size_t mod2873307249533267101(std::size_t hash)
    {
        return hash % 2873307249533267101llu;
    }

    static std::size_t mod3620140286502504283(std::size_t hash)
    {
        return hash % 3620140286502504283llu;
    }

    static std::size_t mod4561090950536962147(std::size_t hash)
    {
        return hash % 4561090950536962147llu;
    }

    static std::size_t mod5746614499066534157(std::size_t hash)
    {
        return hash % 5746614499066534157llu;
    }

    static std::size_t mod7240280573005008577(std::size_t hash)
    {
        return hash % 7240280573005008577llu;
    }

    static std::size_t mod9122181901073924329(std::size_t hash)
    {
        return hash % 9122181901073924329llu;
    }

    static std::size_t mod11493228998133068689(std::size_t hash)
    {
        return hash % 11493228998133068689llu;
    }

    static std::size_t mod14480561146010017169(std::size_t hash)
    {
        return hash % 14480561146010017169llu;
    }

    static std::size_t mod18446744073709551557(std::size_t hash)
    {
        return hash % 18446744073709551557llu;
    }

    std::size_t index_for_hash(std::size_t hash, std::size_t /*num_slots_minus_one*/) const
    {
        static constexpr std::size_t (*const mod_functions[])(std::size_t) = {
            &mod0,
            &mod2,
            &mod3,
            &mod5,
            &mod7,
            &mod11,
            &mod13,
            &mod17,
            &mod23,
            &mod29,
            &mod37,
            &mod47,
            &mod59,
            &mod73,
            &mod97,
            &mod127,
            &mod151,
            &mod197,
            &mod251,
            &mod313,
            &mod397,
            &mod499,
            &mod631,
            &mod797,
            &mod1009,
            &mod1259,
            &mod1597,
            &mod2011,
            &mod2539,
            &mod3203,
            &mod4027,
            &mod5087,
            &mod6421,
            &mod8089,
            &mod10193,
            &mod12853,
            &mod16193,
            &mod20399,
            &mod25717,
            &mod32401,
            &mod40823,
            &mod51437,
            &mod64811,
            &mod81649,
            &mod102877,
            &mod129607,
            &mod163307,
            &mod205759,
            &mod259229,
            &mod326617,
            &mod411527,
            &mod518509,
            &mod653267,
            &mod823117,
            &mod1037059,
            &mod1306601,
            &mod1646237,
            &mod2074129,
            &mod2613229,
            &mod3292489,
            &mod4148279,
            &mod5226491,
            &mod6584983,
            &mod8296553,
            &mod10453007,
            &mod13169977,
            &mod16593127,
            &mod20906033,
            &mod26339969,
            &mod33186281,
            &mod41812097,
            &mod52679969,
            &mod66372617,
            &mod83624237,
            &mod105359939,
            &mod132745199,
            &mod167248483,
            &mod210719881,
            &mod265490441,
            &mod334496971,
            &mod421439783,
            &mod530980861,
            &mod668993977,
            &mod842879579,
            &mod1061961721,
            &mod1337987929,
            &mod1685759167,
            &mod2123923447,
            &mod2675975881,
            &mod3371518343,
            &mod4247846927,
            &mod5351951779,
            &mod6743036717,
            &mod8495693897,
            &mod10703903591,
            &mod13486073473,
            &mod16991387857,
            &mod21407807219,
            &mod26972146961,
            &mod33982775741,
            &mod42815614441,
            &mod53944293929,
            &mod67965551447,
            &mod85631228929,
            &mod107888587883,
            &mod135931102921,
            &mod171262457903,
            &mod215777175787,
            &mod271862205833,
            &mod342524915839,
            &mod431554351609,
            &mod543724411781,
            &mod685049831731,
            &mod863108703229,
            &mod1087448823553,
            &mod1370099663459,
            &mod1726217406467,
            &mod2174897647073,
            &mod2740199326961,
            &mod3452434812973,
            &mod4349795294267,
            &mod5480398654009,
            &mod6904869625999,
            &mod8699590588571,
            &mod10960797308051,
            &mod13809739252051,
            &mod17399181177241,
            &mod21921594616111,
            &mod27619478504183,
            &mod34798362354533,
            &mod43843189232363,
            &mod55238957008387,
            &mod69596724709081,
            &mod87686378464759,
            &mod110477914016779,
            &mod139193449418173,
            &mod175372756929481,
            &mod220955828033581,
            &mod278386898836457,
            &mod350745513859007,
            &mod441911656067171,
            &mod556773797672909,
            &mod701491027718027,
            &mod883823312134381,
            &mod1113547595345903,
            &mod1402982055436147,
            &mod1767646624268779,
            &mod2227095190691797,
            &mod2805964110872297,
            &mod3535293248537579,
            &mod4454190381383713,
            &mod5611928221744609,
            &mod7070586497075177,
            &mod8908380762767489,
            &mod11223856443489329,
            &mod14141172994150357,
            &mod17816761525534927,
            &mod22447712886978529,
            &mod28282345988300791,
            &mod35633523051069991,
            &mod44895425773957261,
            &mod56564691976601587,
            &mod71267046102139967,
            &mod89790851547914507,
            &mod113129383953203213,
            &mod142534092204280003,
            &mod179581703095829107,
            &mod226258767906406483,
            &mod285068184408560057,
            &mod359163406191658253,
            &mod452517535812813007,
            &mod570136368817120201,
            &mod718326812383316683,
            &mod905035071625626043,
            &mod1140272737634240411,
            &mod1436653624766633509,
            &mod1810070143251252131,
            &mod2280545475268481167,
            &mod2873307249533267101,
            &mod3620140286502504283,
            &mod4561090950536962147,
            &mod5746614499066534157,
            &mod7240280573005008577,
            &mod9122181901073924329,
            &mod11493228998133068689,
            &mod14480561146010017169,
            &mod18446744073709551557};
        return mod_functions[prime_index](hash);
    }

    std::uint8_t next_size_over(std::size_t& size) const
    {
        // prime numbers generated by the following method:
        // 1. start with a prime p = 2
        // 2. go to wolfram alpha and get p = NextPrime(2 * p)
        // 3. repeat 2. until you overflow 64 bits
        // you now have large gaps which you would hit if somebody called reserve() with an unlucky number.
        // 4. to fill the gaps for every prime p go to wolfram alpha and get ClosestPrime(p * 2^(1/3)) and ClosestPrime(p * 2^(2/3)) and put those in the gaps
        // 5. get PrevPrime(2^64) and put it at the end
        static constexpr const std::size_t prime_list[] = {2llu,
                                                           3llu,
                                                           5llu,
                                                           7llu,
                                                           11llu,
                                                           13llu,
                                                           17llu,
                                                           23llu,
                                                           29llu,
                                                           37llu,
                                                           47llu,
                                                           59llu,
                                                           73llu,
                                                           97llu,
                                                           127llu,
                                                           151llu,
                                                           197llu,
                                                           251llu,
                                                           313llu,
                                                           397llu,
                                                           499llu,
                                                           631llu,
                                                           797llu,
                                                           1009llu,
                                                           1259llu,
                                                           1597llu,
                                                           2011llu,
                                                           2539llu,
                                                           3203llu,
                                                           4027llu,
                                                           5087llu,
                                                           6421llu,
                                                           8089llu,
                                                           10193llu,
                                                           12853llu,
                                                           16193llu,
                                                           20399llu,
                                                           25717llu,
                                                           32401llu,
                                                           40823llu,
                                                           51437llu,
                                                           64811llu,
                                                           81649llu,
                                                           102877llu,
                                                           129607llu,
                                                           163307llu,
                                                           205759llu,
                                                           259229llu,
                                                           326617llu,
                                                           411527llu,
                                                           518509llu,
                                                           653267llu,
                                                           823117llu,
                                                           1037059llu,
                                                           1306601llu,
                                                           1646237llu,
                                                           2074129llu,
                                                           2613229llu,
                                                           3292489llu,
                                                           4148279llu,
                                                           5226491llu,
                                                           6584983llu,
                                                           8296553llu,
                                                           10453007llu,
                                                           13169977llu,
                                                           16593127llu,
                                                           20906033llu,
                                                           26339969llu,
                                                           33186281llu,
                                                           41812097llu,
                                                           52679969llu,
                                                           66372617llu,
                                                           83624237llu,
                                                           105359939llu,
                                                           132745199llu,
                                                           167248483llu,
                                                           210719881llu,
                                                           265490441llu,
                                                           334496971llu,
                                                           421439783llu,
                                                           530980861llu,
                                                           668993977llu,
                                                           842879579llu,
                                                           1061961721llu,
                                                           1337987929llu,
                                                           1685759167llu,
                                                           2123923447llu,
                                                           2675975881llu,
                                                           3371518343llu,
                                                           4247846927llu,
                                                           5351951779llu,
                                                           6743036717llu,
                                                           8495693897llu,
                                                           10703903591llu,
                                                           13486073473llu,
                                                           16991387857llu,
                                                           21407807219llu,
                                                           26972146961llu,
                                                           33982775741llu,
                                                           42815614441llu,
                                                           53944293929llu,
                                                           67965551447llu,
                                                           85631228929llu,
                                                           107888587883llu,
                                                           135931102921llu,
                                                           171262457903llu,
                                                           215777175787llu,
                                                           271862205833llu,
                                                           342524915839llu,
                                                           431554351609llu,
                                                           543724411781llu,
                                                           685049831731llu,
                                                           863108703229llu,
                                                           1087448823553llu,
                                                           1370099663459llu,
                                                           1726217406467llu,
                                                           2174897647073llu,
                                                           2740199326961llu,
                                                           3452434812973llu,
                                                           4349795294267llu,
                                                           5480398654009llu,
                                                           6904869625999llu,
                                                           8699590588571llu,
                                                           10960797308051llu,
                                                           13809739252051llu,
                                                           17399181177241llu,
                                                           21921594616111llu,
                                                           27619478504183llu,
                                                           34798362354533llu,
                                                           43843189232363llu,
                                                           55238957008387llu,
                                                           69596724709081llu,
                                                           87686378464759llu,
                                                           110477914016779llu,
                                                           139193449418173llu,
                                                           175372756929481llu,
                                                           220955828033581llu,
                                                           278386898836457llu,
                                                           350745513859007llu,
                                                           441911656067171llu,
                                                           556773797672909llu,
                                                           701491027718027llu,
                                                           883823312134381llu,
                                                           1113547595345903llu,
                                                           1402982055436147llu,
                                                           1767646624268779llu,
                                                           2227095190691797llu,
                                                           2805964110872297llu,
                                                           3535293248537579llu,
                                                           4454190381383713llu,
                                                           5611928221744609llu,
                                                           7070586497075177llu,
                                                           8908380762767489llu,
                                                           11223856443489329llu,
                                                           14141172994150357llu,
                                                           17816761525534927llu,
                                                           22447712886978529llu,
                                                           28282345988300791llu,
                                                           35633523051069991llu,
                                                           44895425773957261llu,
                                                           56564691976601587llu,
                                                           71267046102139967llu,
                                                           89790851547914507llu,
                                                           113129383953203213llu,
                                                           142534092204280003llu,
                                                           179581703095829107llu,
                                                           226258767906406483llu,
                                                           285068184408560057llu,
                                                           359163406191658253llu,
                                                           452517535812813007llu,
                                                           570136368817120201llu,
                                                           718326812383316683llu,
                                                           905035071625626043llu,
                                                           1140272737634240411llu,
                                                           1436653624766633509llu,
                                                           1810070143251252131llu,
                                                           2280545475268481167llu,
                                                           2873307249533267101llu,
                                                           3620140286502504283llu,
                                                           4561090950536962147llu,
                                                           5746614499066534157llu,
                                                           7240280573005008577llu,
                                                           9122181901073924329llu,
                                                           11493228998133068689llu,
                                                           14480561146010017169llu,
                                                           18446744073709551557llu};

        const std::size_t* found =
            std::lower_bound(std::begin(prime_list), std::end(prime_list) - 1, size);

        size = *found;

        return static_cast<std::uint8_t>(1 + found - prime_list);
    }

    void commit(std::uint8_t new_prime_index)
    {
        prime_index = new_prime_index;
    }

    void reset()
    {
        prime_index = 0;
    }

private:
    std::uint8_t prime_index = 0;
};

struct power_of_two_hash_policy
{
    std::size_t index_for_hash(std::size_t hash, std::size_t num_slots_minus_one) const
    {
        return hash & num_slots_minus_one;
    }

    std::int8_t next_size_over(std::size_t& size) const
    {
        size = detail::next_power_of_two(size);
        return 0;
    }

    void commit(std::int8_t /*unused*/)
    {
    }

    void reset()
    {
    }
};
}
}

#endif
