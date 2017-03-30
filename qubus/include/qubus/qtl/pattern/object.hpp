#ifndef QUBUS_QTL_PATTERN_OBJECT_HPP
#define QUBUS_QTL_PATTERN_OBJECT_HPP

#include <qubus/qtl/IR/object_expr.hpp>
#include <qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace pattern
{
template <typename Object>
class object_pattern
{
public:
    explicit object_pattern(Object object_) : object_(object_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const qubus::pattern::variable<const object_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<object_expr>())
        {
            if (object_.match(concret_value->obj()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        object_.reset();
    }

private:
    Object object_;
};

template <typename Object>
auto obj(Object object)
{
    return object_pattern<Object>(std::move(object));
}

}
}
}

#endif
