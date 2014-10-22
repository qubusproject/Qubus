#ifndef QBB_KUBUS_PATTERN_FOR_ALL_HPP
#define QBB_KUBUS_PATTERN_FOR_ALL_HPP

#include <qbb/kubus/IR/for_all_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Index, typename Body>
class for_all_pattern
{
public:
    for_all_pattern(Index index_, Body body_) : index_(std::move(index_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<for_all_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_all_expr>())
        {
            if (index_.match(concret_value->index()))
            {
                if (body_.match(concret_value->body()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

private:
    Index index_;
    Body body_;
};

template <typename Index, typename Body>
for_all_pattern<Index, Body> for_all(Index index, Body body)
{
    return for_all_pattern<Index, Body>(index, body);
}
}
}
}

#endif