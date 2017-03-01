#ifndef QUBUS_FOREIGN_COMPUTELET_HPP
#define QUBUS_FOREIGN_COMPUTELET_HPP

#include <hpx/config.hpp>

#include <qubus/architecture_identifier.hpp>
#include <qubus/global_id.hpp>

#include <qubus/IR/type.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/runtime/serialization/serialize.hpp>

#include <qubus/util/function_traits.hpp>
#include <qubus/util/unused.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace qubus
{

template <typename F, typename Indices>
struct function_thunk_mixin_impl;

template <std::size_t, typename T>
struct type_generator
{
    using type = T;
};

template <typename F, std::size_t... Indices>
struct function_thunk_mixin_impl<F, std::index_sequence<Indices...>>
{
private:
    template <typename... Args>
    static void thunk_body(F* f, Args... args)
    {
        (*f)(util::function_traits<F>::template arg<Indices>::type::construct_from_reference(
            args)...);
    }

public:
    static constexpr void* thunk = reinterpret_cast<void*>(
        static_cast<void (*)(F*, typename type_generator<Indices, void*>::type...)>(&thunk_body));
};

template <typename F>
struct function_thunk_mixin
    : function_thunk_mixin_impl<F, std::make_index_sequence<util::function_traits<F>::arity>>
{
};

template <typename F>
struct foreign_computelet_traits : function_thunk_mixin<F>
{
};

class foreign_computelet_registry
{
public:
    class version
    {
    public:
        version() = default;
        virtual ~version() = default;

        virtual const architecture_identifier& target() const = 0;

        virtual const void* get_function() const = 0;
        virtual const void* get_data() const = 0;
    };

    template <typename Implementation>
    void add_version(const std::string& kernel_id, architecture_identifier target,
                     Implementation implementation)
    {
        std::lock_guard<std::mutex> lock(version_table_mutex_);

        auto& subtable = version_table_[kernel_id];

        auto search_result =
            std::find_if(subtable.begin(), subtable.end(),
                         [&target](const auto& entry) { return entry->target() == target; });

        if (search_result == search_result)
        {
            auto version = std::make_unique<version_wrapper<Implementation>>(
                std::move(target), std::move(implementation));

            subtable.push_back(std::move(version));
        }
        else
        {
            throw 0;
        }
    }

    const version& lookup_version(const std::string& kernel_id,
                                  const architecture_identifier& target) const;

private:
    template <typename T>
    class version_wrapper : public version
    {
    public:
        version_wrapper() = default;

        explicit version_wrapper(architecture_identifier target_, T value_)
        : target_(std::move(target_)), value_(std::move(value_))
        {
        }

        const architecture_identifier& target() const override final
        {
            return target_;
        }

        const void* get_function() const override final
        {
            return foreign_computelet_traits<T>::thunk;
        }

        const void* get_data() const override final
        {
            return &value_;
        }

    private:
        architecture_identifier target_;
        T value_;
    };

    // Since the registry might be used during static initialisation, we have to use a full OS mutex.
    mutable std::mutex version_table_mutex_;
    std::map<std::string, std::vector<std::unique_ptr<version>>> version_table_;
};

foreign_computelet_registry& get_foreign_computelet_registry();

class foreign_computelet
{
public:
    using version = foreign_computelet_registry::version;

    foreign_computelet() = default;
    foreign_computelet(std::string id_, type result_type_, std::vector<type> argument_types_);

    template <typename T>
    foreign_computelet& add_version(architecture_identifier target, T version)
    {
        get_foreign_computelet_registry().add_version(id_, std::move(target), std::move(version));

        return *this;
    }

    const version& lookup_version(const architecture_identifier& target) const
    {
        return get_foreign_computelet_registry().lookup_version(id_, target);
    }

    const type& result_type() const
    {
        return result_type_;
    }

    const std::vector<type>& argument_types() const
    {
        return argument_types_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& id_;
        ar& result_type_;
        ar& argument_types_;
    }

private:
    std::string id_;
    type result_type_;
    std::vector<type> argument_types_;
};

inline bool operator==(const foreign_computelet& QUBUS_UNUSED(lhs),
                       const foreign_computelet& QUBUS_UNUSED(rhs))
{
    return false;
}

inline bool operator!=(const foreign_computelet& lhs, const foreign_computelet& rhs)
{
    return !(lhs == rhs);
}
}

#endif