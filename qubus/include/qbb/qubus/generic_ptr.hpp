#ifndef QBB_QUBUS_GENERIC_PTR_HPP
#define QBB_QUBUS_GENERIC_PTR_HPP

#include <memory>
#include <typeinfo>
#include <ostream>

namespace qubus
{
class generic_ptr
{
public:
    generic_ptr() = default;
    
    template <typename T>
    generic_ptr(T ptr_)
    : self_{new generic_ptr_wrapper<T>(ptr_)}
    {
    }

    generic_ptr(const generic_ptr& other) : self_{other.self_ ? other.self_->clone() : nullptr}
    {
    }

    generic_ptr(generic_ptr&&) = default;

    generic_ptr& operator=(const generic_ptr& other)
    {
        *this = generic_ptr(other);

        return *this;
    }

    generic_ptr& operator=(generic_ptr&&) = default;

    template <typename T>
    T as() const
    {
        if(!self_)
            return T{};
        
        if (typeid(*self_) == typeid(generic_ptr_wrapper<T>))
        {
            return static_cast<const generic_ptr_wrapper<T>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    void dump(std::ostream& os) const
    {
        self_->dump(os);
    }

    explicit operator bool() const
    {
        return self_ && self_->is_valid();
    }
private:
    class generic_ptr_interface
    {
    public:
        generic_ptr_interface() = default;
        virtual ~generic_ptr_interface() = default;

        generic_ptr_interface(const generic_ptr_interface&) = delete;
        generic_ptr_interface& operator=(const generic_ptr_interface&) = delete;

        virtual std::unique_ptr<generic_ptr_interface> clone() const = 0;
        virtual void dump(std::ostream& os) const = 0;
        
        virtual bool is_valid() const = 0;
    };

    template <typename T>
    class generic_ptr_wrapper final : public generic_ptr_interface
    {
    public:
        generic_ptr_wrapper(T wrapped_ptr_) : wrapped_ptr_{std::move(wrapped_ptr_)}
        {
        }

        virtual ~generic_ptr_wrapper() = default;

        T get() const
        {
            return wrapped_ptr_;
        }

        std::unique_ptr<generic_ptr_interface> clone() const override
        {
            return std::unique_ptr<generic_ptr_interface>{new generic_ptr_wrapper<T>(wrapped_ptr_)};
        }

        void dump(std::ostream& os) const override
        {
            os << wrapped_ptr_;
        }

        bool is_valid() const override
        {
            return !!wrapped_ptr_;
        }
    private:
        T wrapped_ptr_;
    };

    std::unique_ptr<generic_ptr_interface> self_;
};

inline std::ostream& operator<<(std::ostream& os, const generic_ptr& value)
{
    value.dump(os);

    return os;
}
}

#endif