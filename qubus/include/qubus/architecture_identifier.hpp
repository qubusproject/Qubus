#ifndef QUBUS_ARCHITECTURE_IDENTIFIER_HPP
#define QUBUS_ARCHITECTURE_IDENTIFIER_HPP

#include <hpx/config.hpp>

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/base_object.hpp>

#include <qubus/util/unused.hpp>
#include <qubus/util/span.hpp>

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

template <typename T>
constexpr bool is_architecture_identifier_v = is_architecture_identifier<T>::value;

class architecture_identifier
{
public:
    architecture_identifier() = default;

    template <typename T, typename Enabler =
                              typename std::enable_if<is_architecture_identifier<T>::value>::type>
    architecture_identifier(T value)
    {
        self_ = std::make_shared<architecture_identifier_wrapper<T>>(std::move(value));
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    util::span<const char> payload() const;

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

        virtual util::span<const char> payload() const = 0;

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

        explicit architecture_identifier_wrapper(T value_)
        : value_(std::move(value_))
        {
        }

        std::type_index rtti() const override final
        {
            return typeid(T);
        }

        util::span<const char> payload() const override final
        {
            return value_.payload();
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
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(architecture_identifier_wrapper);

    private:
        T value_;
    };

    std::shared_ptr<architecture_identifier_interface> self_;
};

class host_architecture_identifier : public architecture_identifier_base<host_architecture_identifier>
{
public:
    util::span<const char> payload() const
    {
        return util::span<const char>();
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

namespace arch
{
using host_type = host_architecture_identifier;

constexpr auto host = host_type();
}

}

#endif
