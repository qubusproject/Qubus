#ifndef QBB_QUBUS_PATTERN_LOCAL_VARIABLE_DEF_HPP
#define QBB_QUBUS_PATTERN_LOCAL_VARIABLE_DEF_HPP

#include <qbb/qubus/IR/local_variable_def_expr.hpp>

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

template <typename Decl, typename Initializer>
class local_variable_def_pattern
{
public:
    local_variable_def_pattern(Decl decl_, Initializer initializer_)
    : decl_(std::move(decl_)), initializer_(std::move(initializer_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const local_variable_def_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<local_variable_def_expr>())
        {
            if (decl_.match(concret_value->decl()))
            {
                if (initializer_.match(concret_value->initializer()))
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
        decl_.reset();
        initializer_.reset();
    }

private:
    Decl decl_;
    Initializer initializer_;
};

template <typename Decl, typename Initializer>
local_variable_def_pattern<Decl, Initializer>
local_variable_def(Decl decl, Initializer initializer)
{
    return local_variable_def_pattern<Decl, Initializer>(decl, initializer);
}
}
}
}

#endif