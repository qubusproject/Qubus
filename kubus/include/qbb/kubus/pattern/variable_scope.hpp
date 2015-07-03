#ifndef QBB_KUBUS_PATTERN_VARIABLE_SCOPE_HPP
#define QBB_KUBUS_PATTERN_VARIABLE_SCOPE_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <qbb/kubus/pattern/for_all.hpp>
#include <qbb/kubus/pattern/for.hpp>
#include <qbb/kubus/pattern/sum.hpp>
#include <qbb/kubus/pattern/core.hpp>

#include <utility>

namespace qbb
{
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

    bool match(const expression& value, const variable<expression>* var = nullptr) const
    {
        auto p = for_all(decl_, scope_) || for_(decl_, _, _, _, scope_) || sum(scope_, decl_);

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
}

#endif
