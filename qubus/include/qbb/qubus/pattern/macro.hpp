#ifndef QBB_QUBUS_PATTERN_MACRO_HPP
#define QBB_QUBUS_PATTERN_MACRO_HPP

#include <qbb/qubus/IR/macro_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>

#include <utility>
#include <functional>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename Params, typename Body>
class macro_pattern
{
public:
    macro_pattern(Params params_, Body body_) : params_(std::move(params_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<std::reference_wrapper<const macro_expr>>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<macro_expr>())
        {
            if (params_.match(concret_value->params()))
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
        params_.reset();
        body_.reset();
    }

private:
    Params params_;
    Body body_;
};

template <typename Params, typename Body>
macro_pattern<Params, Body> macro(Params params, Body body)
{
    return macro_pattern<Params, Body>(std::move(params), std::move(body));
}
}
}
}

#endif