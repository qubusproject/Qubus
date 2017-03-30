#ifndef QUBUS_PATTERN_FOREIGN_CALL_HPP
#define QUBUS_PATTERN_FOREIGN_CALL_HPP

#include <qubus/IR/foreign_call_expr.hpp>

#include <qubus/pattern/sequence.hpp>
#include <qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace pattern
{

template <typename ForeignComputelet, typename Arguments, typename Result>
class foreign_call_pattern
{
public:
    foreign_call_pattern(ForeignComputelet foreign_computelet_, Arguments arguments_,
                         Result result_)
    : foreign_computelet_(std::move(foreign_computelet_)),
      arguments_(std::move(arguments_)),
      result_(std::move(result_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const foreign_call_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<foreign_call_expr>())
        {
            if (foreign_computelet_.match(concret_value->computelet()))
            {
                if (arguments_.match(concret_value->args()))
                {
                    if (result_.match(concret_value->result()))
                    {
                        if (var)
                        {
                            var->set(*concret_value);
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

    void reset() const
    {
        foreign_computelet_.reset();
        arguments_.reset();
        result_.reset();
    }

private:
    ForeignComputelet foreign_computelet_;
    Arguments arguments_;
    Result result_;
};

template <typename ForeignComputelet, typename Arguments, typename Result>
foreign_call_pattern<ForeignComputelet, Arguments, Result>
call_foreign(ForeignComputelet foreign_computelet, Arguments arguments, Result result)
{
    return foreign_call_pattern<ForeignComputelet, Arguments, Result>(
        std::move(foreign_computelet), std::move(arguments), std::move(result));
}
}
}

#endif
