#ifndef QBB_KUBUS_PATTERN_SCOPED_VIEW_HPP
#define QBB_KUBUS_PATTERN_SCOPED_VIEW_HPP

#include <qbb/kubus/IR/scoped_view_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

class scoped_view_pattern
{
public:
    template <typename BaseType>
    bool match(const BaseType& value, const variable<scoped_view_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<scoped_view_expr>())
        {
            if (var)
            {
                var->set(*concret_value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
};

inline scoped_view_pattern scoped_view()
{
    return scoped_view_pattern();
}
}
}
}

#endif