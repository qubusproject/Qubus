#ifndef QBB_QUBUS_PATTERN_FOREIGN_CALL_HPP
#define QBB_QUBUS_PATTERN_FOREIGN_CALL_HPP

#include <qbb/qubus/IR/foreign_call_expr.hpp>

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

template <typename ForeignComputelet, typename Arguments>
class foreign_call_pattern
{
public:
    foreign_call_pattern(ForeignComputelet foreign_computelet_, Arguments arguments_)
    : foreign_computelet_(std::move(foreign_computelet_)), arguments_(std::move(arguments_))
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
        foreign_computelet_.reset();
        arguments_.reset();
    }

private:
    ForeignComputelet foreign_computelet_;
    Arguments arguments_;
};

template <typename ForeignComputelet, typename Arguments>
foreign_call_pattern<ForeignComputelet, Arguments> call_foreign(ForeignComputelet foreign_computelet, Arguments arguments)
{
    return foreign_call_pattern<ForeignComputelet, Arguments>(foreign_computelet, arguments);
}

template <typename ForeignComputelet, typename... Arguments>
auto call_foreign_n(ForeignComputelet foreign_computelet, Arguments... arguments)
{
    return call_foreign(foreign_computelet, sequence(arguments...));
}
}
}
}

#endif
