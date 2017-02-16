#ifndef QUBUS_PATTERN_LITERAL_HPP
#define QUBUS_PATTERN_LITERAL_HPP

#include <qbb/qubus/IR/literal_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{
template <typename Value>
class double_literal_pattern
{
public:
    explicit double_literal_pattern(Value value_) : value_(std::move(value_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const double_literal_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<double_literal_expr>())
        {
            if (value_.match(concret_value->value()))
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
        value_.reset();
    }
private:
    Value value_;
};

template <typename Value>
class float_literal_pattern
{
public:
    explicit float_literal_pattern(Value value_) : value_(std::move(value_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const float_literal_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<float_literal_expr>())
        {
            if (value_.match(concret_value->value()))
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
        value_.reset();
    }
private:
    Value value_;
};

template <typename Value>
class integer_literal_pattern
{
public:
    explicit integer_literal_pattern(Value value_) : value_(std::move(value_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const integer_literal_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<integer_literal_expr>())
        {
            if (value_.match(concret_value->value()))
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
        value_.reset();
    }
private:
    Value value_;
};

template <typename Value>
double_literal_pattern<Value> double_literal(Value value)
{
    return double_literal_pattern<Value>(value);
}

template <typename Value>
float_literal_pattern<Value> float_literal(Value value)
{
    return float_literal_pattern<Value>(value);
}

template <typename Value>
integer_literal_pattern<Value> integer_literal(Value value)
{
    return integer_literal_pattern<Value>(value);
}
}
}

#endif