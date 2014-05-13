#ifndef QBB_UTIL_DISPATCH_TABLE_HPP
#define QBB_UTIL_DISPATCH_TABLE_HPP

#include <qbb/util/detail/virtual.hpp>
#include <qbb/util/multi_array.hpp>
#include <qbb/util/integer_sequence.hpp>
#include <qbb/util/meta/unique.hpp>
#include <qbb/util/function_traits.hpp>
#include <qbb/util/make_array.hpp>
#include <qbb/util/nested_for_each.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <array>
#include <typeindex>
#include <functional>
#include <utility>
#include <map>
#include <exception>

#include <mutex>
#include <condition_variable>
#include <atomic>

namespace qbb
{
namespace util
{

class implementation
{
public:
    explicit implementation(const std::pair<std::type_index, index_t>& rtti_tag_pair_)
    : rtti_tag_pair_(rtti_tag_pair_)
    {
    }

    index_t tag() const
    {
        return rtti_tag_pair_.second;
    }

    const std::type_index& rtti() const
    {
        return rtti_tag_pair_.first;
    }

private:
    std::pair<std::type_index, index_t> rtti_tag_pair_;
};

class makes_implementation
{
public:
    using result_type = implementation;

    implementation operator()(const std::pair<std::type_index, index_t>& rtti_tag_pair) const
    {
        return implementation(rtti_tag_pair);
    }
};

class implementation_rtti
{
public:
    using result_type = std::type_index;

    std::type_index operator()(const implementation& impl) const
    {
        return impl.rtti();
    }
};

class implementation_tag
{
public:
    using result_type = index_t;

    index_t operator()(const implementation& impl) const
    {
        return impl.tag();
    }
};

class implementation_table
{
public:
    template <typename T>
    index_t register_type()
    {
        auto iter_result_pair = implementations_.insert(
            std::make_pair(std::type_index(typeid(T)), to_uindex(implementations_.size())));

        if (iter_result_pair.second)
        {
            ++version_;
        }
        
        return iter_result_pair.first->second;
    }

    auto implementations() const
        -> decltype(std::declval<const std::map<std::type_index, index_t>&>() |
                    boost::adaptors::transformed(makes_implementation()))
    {
        return implementations_ | boost::adaptors::transformed(makes_implementation());
    }

    std::size_t number_of_implementations() const
    {
        return implementations_.size();
    }

    const std::atomic<int>& version() const
    {
        return version_;
    }

private:
    std::map<std::type_index, index_t> implementations_;
    std::atomic<int> version_;
};

class invalid_key_exception : public std::exception
{
public:
    virtual ~invalid_key_exception() = default;
    
    const char* what() const noexcept override
    {
        return "Invalid key during table lookup";
    }
};

template <typename T, index_t Rank>
class dense_table
{
public:
    void set(const std::array<index_t, Rank>& indices, const T& value)
    {
        table_(indices) = value;
    }

    const T& lookup(const std::array<index_t, Rank>& indices) const
    {
        const auto& entry = table_(indices);

        if (entry)
        {
            return entry;
        }
        else
        {
            throw invalid_key_exception();
        }
    }

    void clear()
    {
        std::fill(table_.begin(), table_.end(), T{});
    }

    void reshape(const std::array<index_t, Rank>& shape)
    {
        table_.reshape(shape);
    }

private:
    multi_array<T, Rank> table_;
};

template <typename T, index_t Rank>
class sparse_table
{
public:
    void set(const std::array<index_t, Rank>& indices, const T& value)
    {
        map_.insert({indices, value});
    }

    const T& lookup(const std::array<index_t, Rank>& indices) const
    {
        auto iter = map_.find(indices);

        if (iter != map_.end())
        {
            return iter->second;
        }
        else
        {
            throw invalid_key_exception();
        }
    }

    void clear()
    {
        map_.clear();
    }

    void reshape(const std::array<index_t, Rank>&)
    {
    }

private:
    std::map<std::array<index_t, Rank>, T> map_;
};

namespace detail
{

template <typename... PolymorphicArgs>
std::array<const implementation_table*, sizeof...(PolymorphicArgs)>
get_implementation_tables_impl(meta::type_sequence<PolymorphicArgs...>)
{
    return {{&PolymorphicArgs::get_implementation_table()...}};
}
}

template <typename PolymorphicArgs>
std::array<const implementation_table*, PolymorphicArgs::size()> get_implementation_tables()
{
    return detail::get_implementation_tables_impl(PolymorphicArgs());
}

class ambiguous_call_exception : public std::exception
{
public:
    virtual ~ambiguous_call_exception() = default;
    
    const char* what() const noexcept override
    {
        return "Multimethod call is ambiguous.";
    }
};

template <typename, template <typename, index_t> class Table>
class dispatch_table;

template <typename ReturnType, typename... Args, template <typename, index_t> class Table>
class dispatch_table<ReturnType(Args...), Table>
{
public:
    using specialization_t = std::function<ReturnType(remove_virtual<Args>...)>;
    using tag_type = index_t;

    using args = meta::type_sequence<Args...>;
    
    using polymorphic_args_seq = polymorphic_args<meta::type_sequence<Args...>>;
    using polymorphic_types_seq = meta::unique<polymorphic_args_seq>;

    static constexpr std::size_t arity()
    {
        return polymorphic_args_seq::size();
    }

    const specialization_t& find(const std::array<tag_type, arity()>& key) const
    {
        if (is_outdated())
        {
            std::unique_lock<std::mutex> lock_guard(table_mutex_);
            table_updated_cv_.wait(lock_guard);
        }

        try
        {
            return table_.lookup(key);
        }
        catch (const invalid_key_exception&)
        {
            throw std::bad_function_call();
        }
    }

    void build_dispatch_table(
        const std::map<std::array<std::type_index, arity()>, specialization_t>& specializations)
    {
        if (is_outdated())
        {
            std::lock_guard<std::mutex> guard(table_mutex_);

            auto implementation_tables = get_implementation_tables<polymorphic_args_seq>();

            table_.clear();
            
            std::array<index_t, arity()> table_shape;
            
            for(std::size_t i = 0; i < arity(); ++i)
            {
                table_shape[i] = implementation_tables[i]->number_of_implementations();
            }
            
            table_.reshape(table_shape);

            nested_for_each(begin(implementation_tables), end(implementation_tables),
                            [](const implementation_table*& impl_table)
                            { return impl_table->implementations(); },
                            [&](const std::vector<implementation>& impls)
                            { add_best_specialization(specializations, impls); });

            for (std::size_t i = 0; i < implementation_tables.size(); ++i)
            {
                versions_[i] = implementation_tables[i]->version().load();
            }

            table_updated_cv_.notify_all();
        }
    }

    bool is_outdated() const
    {
        using polymorphic_args_seq = polymorphic_args<meta::type_sequence<Args...>>;
        using polymorphic_types_seq = meta::unique<polymorphic_args_seq>;

        return is_outdated_impl(
            polymorphic_types_seq(), std::integral_constant<std::size_t, 0>(),
            std::integral_constant<std::size_t, polymorphic_types_seq::size()>());
    }

private:
    int overload_distance(const std::array<std::type_index, arity()>& specialization_args,
                          const std::array<std::type_index, arity()>& call_args)
    {
        auto poly_args_rtti = polymorphic_args_rtti<Args...>();

        int distance = 0;

        for (std::size_t i = 0; i < arity(); ++i)
        {
            if (specialization_args[i] != call_args[i])
            {
                if (specialization_args[i] == poly_args_rtti[i])
                {
                    ++distance;
                }
                else
                {
                    return -1;
                }
            }
        }

        return distance;
    }

    template <typename PolymorphicTypes, std::size_t I, std::size_t N>
    bool is_outdated_impl(PolymorphicTypes, std::integral_constant<std::size_t, I>,
                          std::integral_constant<std::size_t, N>) const
    {
        return PolymorphicTypes::template at<I>::get_implementation_table().version() >
                   versions_[I] ||
               is_outdated_impl(PolymorphicTypes(), std::integral_constant<std::size_t, I + 1>(),
                                std::integral_constant<std::size_t, N>());
    }

    template <typename PolymorphicTypes, std::size_t N>
    bool is_outdated_impl(PolymorphicTypes, std::integral_constant<std::size_t, N>,
                          std::integral_constant<std::size_t, N>) const
    {
        return false;
    }

    void add_best_specialization(
        const std::map<std::array<std::type_index, arity()>, specialization_t>& specializations,
        const std::vector<implementation>& implementations)
    {
        constexpr int huge_number = 1000000;

        int current_distance = huge_number;
        specialization_t best_specialization;

        for (const auto& specialization : specializations)
        {
            auto call_arg_types = make_array<std::type_index, arity()>(
                implementations | boost::adaptors::transformed(implementation_rtti()));

            int distance = overload_distance(specialization.first, call_arg_types);

            if (distance < 0)
            {
                continue;
            }

            if (distance < current_distance)
            {
                best_specialization = specialization.second;
                current_distance = distance;
            }
            else if (distance == current_distance)
            {
                best_specialization = [](remove_virtual<Args>...) -> ReturnType
                { throw ambiguous_call_exception(); };

                current_distance = 0;

                break;
            }
        }

        if (best_specialization)
        {
            auto tags = make_array<index_t, arity()>(
                implementations | boost::adaptors::transformed(implementation_tag()));

            table_.set(tags, best_specialization);
        }
    }

    Table<specialization_t, arity()> table_;

    std::array<std::atomic<int>, polymorphic_types_seq::size()> versions_;

    mutable std::mutex table_mutex_;
    mutable std::condition_variable table_updated_cv_;
};

template <typename T>
using sparse_dispatch_table = dispatch_table<T, sparse_table>;

template <typename T>
using dense_dispatch_table = dispatch_table<T, dense_table>;

}
}

#endif