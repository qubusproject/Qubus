#ifndef QUBUS_ARCHITECTURE_IDENTIFIER_HPP
#define QUBUS_ARCHITECTURE_IDENTIFIER_HPP

#include <hpx/config.hpp>

#include <qubus/util/multi_method.hpp>

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/base_object.hpp>

#include <qubus/util/unused.hpp>

#include <type_traits>
#include <utility>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace qubus
{

template <typename T>
class architecture_identifier_base
{
protected:
    ~architecture_identifier_base() = default;
};

template <typename T>
struct is_architecture_identifier : std::is_base_of<architecture_identifier_base<T>, T>
{
};

class architecture_identifier
{
public:
    architecture_identifier() = default;

    template <typename T, typename Enabler =
                              typename std::enable_if<is_architecture_identifier<T>::value>::type>
    architecture_identifier(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<architecture_identifier_wrapper<T>>(std::move(value), tag);
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    util::index_t type_tag() const
    {
        return self_->tag();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<architecture_identifier_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    template <typename T>
    const T* try_as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return &static_cast<architecture_identifier_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            return nullptr;
        }
    }

    static const util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& self_;
    }

private:
    class architecture_identifier_interface
    {
    public:
        architecture_identifier_interface() = default;
        virtual ~architecture_identifier_interface() = default;

        architecture_identifier_interface(const architecture_identifier_interface&) = delete;
        architecture_identifier_interface&
        operator=(const architecture_identifier_interface&) = delete;

        architecture_identifier_interface(architecture_identifier_interface&&) = delete;
        architecture_identifier_interface& operator=(architecture_identifier_interface&&) = delete;

        virtual std::type_index rtti() const = 0;
        virtual util::index_t tag() const = 0;

        template <typename Archive>
        void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
        {
        }

        HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(architecture_identifier_interface);
    };

    template <typename T>
    class architecture_identifier_wrapper : public architecture_identifier_interface
    {
    public:
        virtual ~architecture_identifier_wrapper() = default;

        architecture_identifier_wrapper() = default;

        explicit architecture_identifier_wrapper(T value_, util::index_t tag_)
        : value_(std::move(value_)), tag_(tag_)
        {
        }

        std::type_index rtti() const override final
        {
            return typeid(T);
        }

        util::index_t tag() const override final
        {
            return tag_;
        }

        const T& get() const
        {
            return value_;
        }

        T& get()
        {
            return value_;
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
        {
            ar& hpx::serialization::base_object<architecture_identifier_interface>(*this);

            ar& value_;
            ar& tag_;
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(architecture_identifier_wrapper);

    private:
        T value_;
        util::index_t tag_;
    };

    std::shared_ptr<architecture_identifier_interface> self_;

    static util::implementation_table implementation_table_;
};

extern util::multi_method<bool(const util::virtual_<architecture_identifier>&,
                               const util::virtual_<architecture_identifier>&)>
    is_same_architecture;

inline bool operator==(const architecture_identifier& lhs, const architecture_identifier& rhs)
{
    return is_same_architecture(lhs, rhs);
}

inline bool operator!=(const architecture_identifier& lhs, const architecture_identifier& rhs)
{
    return !(lhs == rhs);
}

class host_architecture_identifier : public architecture_identifier_base<host_architecture_identifier>
{
public:
    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

namespace architectures
{
constexpr auto host = host_architecture_identifier();
}

}

#endif
