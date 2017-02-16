#ifndef QUBUS_PATTERN_CONSTRUCT_HPP
#define QUBUS_PATTERN_CONSTRUCT_HPP

#include <qubus/IR/construct_expr.hpp>

#include <qubus/pattern/variable.hpp>
#include <qubus/pattern/sequence.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{

template <typename ResultType, typename Parameters>
class construct_pattern
{
public:
    construct_pattern(ResultType result_type_, Parameters parameters_)
    : result_type_(std::move(result_type_)), parameters_(std::move(parameters_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const construct_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<construct_expr>())
        {
            if (result_type_.match(concret_value->result_type()))
            {
                if (parameters_.match(concret_value->parameters()))
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
        result_type_.reset();
        parameters_.reset();
    }

private:
    ResultType result_type_;
    Parameters parameters_;
};

template <typename ResultType, typename Parameters>
construct_pattern<ResultType, Parameters> construct(ResultType result_type, Parameters parameters)
{
    return construct_pattern<ResultType, Parameters>(result_type, parameters);
}

template <typename ResultType, typename... Parameters>
auto construct_n(ResultType result_type, Parameters... parameters)
{
    return construct(result_type, sequence(parameters...));
}
}
}

#endif
