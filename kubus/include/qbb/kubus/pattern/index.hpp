#ifndef QBB_KUBUS_PATTERN_INDEX_HPP
#define QBB_KUBUS_PATTERN_INDEX_HPP

#include <qbb/kubus/IR/index_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename ID>
class index_pattern
{
public:
    index_pattern(ID id_)
    : id_(id_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<index_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<index_expr>())
        {
            if (id_.match(concret_value->id()))
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

private:
    ID id_;
};

template <typename ID>
index_pattern<ID> index(ID id)
{
    return index_pattern<ID>(id);
}

inline index_pattern<any> index()
{
    return index_pattern<any>(_);
}
}
}
}

#endif