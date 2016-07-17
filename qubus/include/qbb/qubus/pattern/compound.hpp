#ifndef QBB_QUBUS_PATTERN_COMPOUND_HPP
#define QBB_QUBUS_PATTERN_COMPOUND_HPP

#include <qbb/qubus/IR/compound_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/sequence.hpp>

#include <utility>
#include <functional>

namespace qbb
{
namespace qubus
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
    bool match(const BaseType& value, const variable<const compound_expr&>* var = nullptr) const
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

    void reset() const
    {
        body_.reset();
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

template <typename Tasks>
compound_pattern<Tasks> sequenced_tasks(Tasks tasks)
{
    return compound_pattern<Tasks>(tasks);
}

template <typename... Tasks>
auto sequenced_tasks_n(Tasks... tasks)
{
    return compound(sequence(tasks...));
}

}
}
}

#endif