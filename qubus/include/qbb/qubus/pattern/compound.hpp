#ifndef QBB_QUBUS_PATTERN_COMPOUND_HPP
#define QBB_QUBUS_PATTERN_COMPOUND_HPP

#include <qbb/qubus/IR/compound_expr.hpp>

#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/pattern/sequence.hpp>
#include <qbb/qubus/pattern/value.hpp>
#include <qbb/qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace pattern
{

template <typename Order, typename Body>
class compound_pattern
{
public:
    compound_pattern(Order order_, Body body_) : order_(std::move(order_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const compound_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<compound_expr>())
        {
            if (order_.match(concret_value->order()))
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

    void reset() const
    {
        order_.reset();
        body_.reset();
    }

private:
    Order order_;
    Body body_;
};

template<typename Order, typename Body>
auto compound(Order order, Body body)
{
    return compound_pattern<Order, Body>(order, body);
}

template <typename Body>
auto compound(Body body)
{
    return compound(_, body);
}

template <typename... Body>
auto compound_n(Body... body)
{
    return compound(sequence(body...));
}

template <typename Tasks>
auto sequenced_tasks(Tasks tasks)
{
    return compound_pattern<decltype(value(execution_order::sequential)), Tasks>(
        value(execution_order::sequential), tasks);
}

template <typename... Tasks>
auto sequenced_tasks_n(Tasks... tasks)
{
    return sequenced_tasks(sequence(tasks...));
}

template <typename Tasks>
auto unordered_tasks(Tasks tasks)
{
    return compound_pattern<decltype(value(execution_order::unordered)), Tasks>(
            value(execution_order::unordered), tasks);
}

template <typename... Tasks>
auto unordered_tasks_n(Tasks... tasks)
{
    return unordered_tasks(sequence(tasks...));
}

}
}

#endif