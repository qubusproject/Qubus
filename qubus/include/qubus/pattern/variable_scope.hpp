#ifndef QUBUS_PATTERN_VARIABLE_SCOPE_HPP
#define QUBUS_PATTERN_VARIABLE_SCOPE_HPP

#include <qubus/IR/expression.hpp>

#include <qubus/pattern/for.hpp>
#include <qubus/pattern/core.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{

template <typename VariableDeclaration, typename Scope>
class variable_scope_pattern
{
public:
    variable_scope_pattern(VariableDeclaration decl_, Scope scope_)
    : decl_(std::move(decl_)), scope_(std::move(scope_))
    {
    }

    bool match(const expression& value, const variable<std::reference_wrapper<const expression>>* var = nullptr) const
    {
        auto p = for_(decl_, _, _, _, scope_);

        if (p.match(value))
        {
            if (var)
            {
                var->set(value);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    void reset() const
    {
        decl_.reset();
        scope_.reset();
    }

private:
    VariableDeclaration decl_;
    Scope scope_;
};

template <typename VariableDeclaration, typename Scope>
variable_scope_pattern<VariableDeclaration, Scope> variable_scope(VariableDeclaration decl,
                                                                  Scope scope)
{
    return variable_scope_pattern<VariableDeclaration, Scope>(decl, scope);
}
}
}

#endif
