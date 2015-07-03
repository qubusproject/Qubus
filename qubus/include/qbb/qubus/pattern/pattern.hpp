#ifndef QBB_QUBUS_PATTERN_PATTERN_HPP
#define QBB_QUBUS_PATTERN_PATTERN_HPP

#include <memory>
#include <utility>

namespace qbb
{
namespace qubus
{
namespace pattern
{
template <typename BaseType>
class pattern
{
public:
    template <typename T>
    pattern(T value_)
    : self_{std::make_shared<pattern_wrapper<T>>(std::move(value_))}
    {
    }

    bool match(const BaseType& value)
    {
        return self_->match(value);
    }

private:
    class pattern_interface
    {
    public:
        pattern_interface() = default;
        pattern_interface(const pattern_interface&) = delete;
        virtual ~pattern_interface() = default;

        pattern_interface& operator=(const pattern_interface&) = delete;

        virtual bool match(const BaseType& value) const = 0;
    };

    template <typename T>
    class pattern_wrapper final : public pattern_interface
    {
    public:
        explicit pattern_wrapper(T value_)
        : value_(std::move(value_))
        {
        }
        
        virtual ~pattern_wrapper() = default;

        bool match(const BaseType& value) const override
        {
            return value_.match(value);
        }

    private:
        T value_;
    };

    std::shared_ptr<pattern_interface> self_;
};
}
}
}

#endif