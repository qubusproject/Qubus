#ifndef QBB_KUBUS_PATTERN_COMPOUND_HPP
#define QBB_KUBUS_PATTERN_COMPOUND_HPP

#include <qbb/kubus/IR/compound_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Body>
class compound_pattern
{
public:
    compound_pattern(Body body_) : body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<compound_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<compound_expr>())
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

        return false;
    }

private:
    Body body_;
};

template <typename Body>
compound_pattern<Body> compound(Body body)
{
    return compound_pattern<Body>(body);
}

template <typename... Body>
auto compound_n(Body... body)
{
    return compound(sequence(body...));
}
}
}
}

#endif