#ifndef QUBUS_TYPE_HPP
#define QUBUS_TYPE_HPP

#include <hpx/config.hpp>

#include <qubus/util/multi_method.hpp>

#include <qubus/IR/expression_traits.hpp>

#include <qubus/util/unused.hpp>

#include <hpx/runtime/serialization/base_object.hpp>
#include <hpx/runtime/serialization/serialize.hpp>

#include <hpx/include/serialization.hpp>

#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace qubus
{

namespace types
{
class unknown : public type_base<unknown>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};
}

class type
{
public:
    type() : type(types::unknown{})
    {
    }

    template <typename T, typename Enabler = typename std::enable_if<is_type<T>::value>::type>
    type(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_unique<type_wrapper<T>>(value, tag);
    }

    type(const type& other) : self_(other.self_ ? other.self_->clone() : nullptr)
    {
    }

    type& operator=(const type& other)
    {
        self_ = other.self_ ? other.self_->clone() : nullptr;

        return *this;
    }

    type(type&& other) = default;
    type& operator=(type&& other) = default;

    bool is_primitive() const
    {
        return self_->is_primitive();
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<type_wrapper<value_type>*>(self_.get())->get();
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
            return &static_cast<type_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            return nullptr;
        }
    }

    explicit operator bool() const
    {
        return static_cast<bool>(self_);
    }

    util::index_t type_tag() const
    {
        return self_->tag();
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
    class type_interface
    {
    public:
        virtual ~type_interface() = default;

        virtual bool is_primitive() const = 0;

        virtual std::type_index rtti() const = 0;
        virtual util::index_t tag() const = 0;

        virtual std::unique_ptr<type_interface> clone() const = 0;

        template <typename Archive>
        void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
        {
        }

        HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(type_interface);
    };

    template <typename T>
    class type_wrapper : public type_interface
    {
    public:
        type_wrapper() = default;

        explicit type_wrapper(T value_, util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~type_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }

        bool is_primitive() const override final
        {
            return value_.is_primitive();
        }

        std::type_index rtti() const override final
        {
            return typeid(T);
        }

        util::index_t tag() const override final
        {
            return tag_;
        }

        std::unique_ptr<type_interface> clone() const override final
        {
            return std::make_unique<type_wrapper<T>>(value_, tag_);
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
        {
            ar& hpx::serialization::base_object<type_interface>(*this);

            ar& value_;
            ar& tag_;
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(type_wrapper);

    private:
        T value_;
        util::index_t tag_;
    };

    std::unique_ptr<type_interface> self_;

    inline static util::implementation_table implementation_table_;
};

bool operator==(const type& lhs, const type& rhs);
bool operator!=(const type& lhs, const type& rhs);

namespace types
{

class double_ : public type_base<double_>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

class float_ : public type_base<float_>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

class integer : public type_base<integer>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

class bool_ : public type_base<bool_>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

class index : public type_base<index>
{
public:
    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
    {
    }
};

constexpr long int dynamic_rank = -1;

class multi_index : public type_base<multi_index>
{
public:
    multi_index() = default;

    explicit multi_index(long int rank_) : rank_(rank_)
    {
    }

    bool is_primitive() const
    {
        return true;
    }

    long int rank() const
    {
        return rank_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& rank_;
    }

private:
    long int rank_;
};

class complex : public type_base<complex>
{
public:
    complex() = default;

    explicit complex(type real_type_) : real_type_(real_type_)
    {
    }

    bool is_primitive() const
    {
        return true;
    }

    type real_type() const
    {
        return real_type_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& real_type_;
    }

private:
    type real_type_;
};

class integer_range_type : public type_base<integer_range_type>
{
public:
    integer_range_type() = default;

    bool is_primitive() const
    {
        return true;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
    }
};

constexpr integer_range_type integer_range = {};

class array : public type_base<array>
{
public:
    array() = default;

    explicit array(type value_type_, util::index_t rank_)
    : value_type_{std::move(value_type_)}, rank_(std::move(rank_))
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    util::index_t rank() const
    {
        return rank_;
    }

    bool is_primitive() const
    {
        return false;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& value_type_;
        ar& rank_;
    }

private:
    type value_type_;
    util::index_t rank_;
};

class array_slice : public type_base<array_slice>
{
public:
    array_slice() = default;

    explicit array_slice(type value_type_, util::index_t rank_)
    : value_type_{std::move(value_type_)}, rank_(std::move(rank_))
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    util::index_t rank() const
    {
        return rank_;
    }

    bool is_primitive() const
    {
        return false;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& value_type_;
        ar& rank_;
    }

private:
    type value_type_;
    util::index_t rank_;
};

class struct_ : public type_base<struct_>
{
public:
    struct member
    {
        member() = default;

        member(type datatype, std::string id) : datatype(std::move(datatype)), id(std::move(id))
        {
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
        {
            ar& datatype;
            ar& id;
        }

        friend bool operator==(const member& lhs, const member& rhs)
        {
            return lhs.datatype == rhs.datatype && lhs.id == rhs.id;
        }

        friend bool operator!=(const member& lhs, const member& rhs)
        {
            return !(lhs == rhs);
        }

        type datatype;
        std::string id;
    };

    struct_() = default;

    explicit struct_(std::string id_, std::vector<member> members_)
    : id_(std::move(id_)), members_(std::move(members_))
    {
    }

    const std::string& id() const
    {
        return id_;
    }

    const std::vector<member>& members() const
    {
        return members_;
    }

    const type& operator[](const std::string& id) const;

    std::size_t member_index(const std::string& id) const;

    auto begin() const
    {
        return members_.begin();
    }

    auto end() const
    {
        return members_.end();
    }

    std::size_t member_count() const
    {
        return members_.size();
    }

    bool is_primitive() const
    {
        return false;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& id_;
        ar& members_;
    }

private:
    std::string id_;
    std::vector<member> members_;
};

type sparse_tensor(type value_type);
}

// Helper functions

type value_type(const type& array_like);
}

namespace std
{
template <>
struct hash<qubus::type>
{
    using argument_type = qubus::type;
    using result_type = std::size_t;

    std::size_t operator()(const qubus::type& value) const noexcept;
};

template <>
struct hash<qubus::types::struct_::member>
{
    using argument_type = qubus::types::struct_::member;
    using result_type = std::size_t;

    std::size_t operator()(const qubus::types::struct_::member& value) const noexcept;
};
}

#endif
