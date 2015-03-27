#ifndef QBB_KUBUS_TYPE_HPP
#define QBB_KUBUS_TYPE_HPP

#include <qbb/util/multi_method.hpp>

#include <qbb/kubus/IR/expression_traits.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include <functional>
#include <type_traits>

namespace qbb
{
namespace kubus
{

namespace types
{
class unknown
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

}

template<>
struct is_type<types::unknown> : std::true_type
{
};

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

        self_ = std::make_shared<type_wrapper<T>>(value, tag);
    }

    type(const type&) = default;
    type(type&) = default;

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
    
    qbb::util::index_t tag() const
    {
        return self_->tag();
    }

    static const qbb::util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

private:
    class type_interface
    {
    public:
        virtual ~type_interface() = default;

        virtual bool is_primitive() const = 0;

        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;
    };

    template <typename T>
    class type_wrapper final : public type_interface
    {
    public:
        explicit type_wrapper(T value_, qbb::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~type_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }

        bool is_primitive() const override
        {
            return value_.is_primitive();
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        qbb::util::index_t tag() const override
        {
            return tag_;
        }

    private:
        T value_;
        qbb::util::index_t tag_;
    };

    std::shared_ptr<type_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};

bool operator==(const type& lhs, const type& rhs);
bool operator!=(const type& lhs, const type& rhs);

namespace types
{

class double_
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

class float_
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

class integer
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

class index
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

constexpr long int dynamic_rank = -1;

class multi_index
{
public:
    multi_index(long int rank_)
    : rank_(rank_)
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
private:
    long int rank_;
};

class complex
{
public:
    explicit complex(type real_type_)
    : real_type_(real_type_)
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
private:
    type real_type_;
};

class tensor
{
public:
    explicit tensor(type value_type_) : value_type_{std::move(value_type_)}
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    bool is_primitive() const
    {
        return false;
    }

private:
    type value_type_;
};

class array
{
public:
    explicit array(type value_type_) : value_type_{std::move(value_type_)}
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    bool is_primitive() const
    {
        return false;
    }

private:
    type value_type_;
};

class sparse_tensor
{
public:
    explicit sparse_tensor(type value_type_) : value_type_{std::move(value_type_)}
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    bool is_primitive() const
    {
        return false;
    }

private:
    type value_type_;
};

}

template<>
struct is_type<types::double_> : std::true_type
{
};

template<>
struct is_type<types::float_> : std::true_type
{
};

template<>
struct is_type<types::integer> : std::true_type
{
};

template<>
struct is_type<types::index> : std::true_type
{
};

template<>
struct is_type<types::multi_index> : std::true_type
{
};

template<>
struct is_type<types::complex> : std::true_type
{
};

template<>
struct is_type<types::array> : std::true_type
{
};

template<>
struct is_type<types::tensor> : std::true_type
{
};

template<>
struct is_type<types::sparse_tensor> : std::true_type
{
};

}
}

namespace std
{
    template<>
    struct hash<qbb::kubus::type>
    {
        using argument_type = qbb::kubus::type;
        using result_type = std::size_t;
        
        std::size_t operator()(const qbb::kubus::type& value) const noexcept;
    };
}

#endif
