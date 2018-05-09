#ifndef QUBUS_PATTERN_TYPEOF_HPP
#define QUBUS_PATTERN_TYPEOF_HPP

#include <qubus/pattern/variable.hpp>

#include <qubus/IR/type_inference.hpp>

#include <utility>

namespace qubus
{
namespace pattern
{
template <typename TypePattern>
class typeof_pattern
{
public:
    explicit typeof_pattern(TypePattern type_pattern_)
    : type_pattern_(std::move(type_pattern_))
    {
    }

    bool match(const expression& value, const variable<std::reference_wrapper<const expression>>* var = nullptr) const
    {
        auto result = type_pattern_.match(typeof_(value));

        if (!result)
            return false;

        if(var)
        {
            var->set(value);
        }

        return true;
    }

    void reset() const
    {
        type_pattern_.reset();
    }

private:
    TypePattern type_pattern_;
};

template <typename TypePattern>
typeof_pattern<TypePattern> typeof_(TypePattern type_pattern)
{
    return typeof_pattern<TypePattern>(std::move(type_pattern));
}
}
}

#endif //QUBUS_TYPEOF_HPP
