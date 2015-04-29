#ifndef QBB_KUBUS_PATTERN_LOCAL_VARIABLE_DEF_HPP
#define QBB_KUBUS_PATTERN_LOCAL_VARIABLE_DEF_HPP

#include <qbb/kubus/IR/local_variable_def_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Decl, typename Initializer, typename Scope>
class local_variable_def_pattern
{
public:
    local_variable_def_pattern(Decl decl_, Initializer initializer_, Scope scope_)
    : decl_(std::move(decl_)), initializer_(std::move(initializer_)), scope_(std::move(scope_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<local_variable_def_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<local_variable_def_expr>())
        {
            if (decl_.match(concret_value->decl()))
            {
                if (initializer_.match(concret_value->initializer()))
                {
                    if (scope_.match(concret_value->scope()))
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
        decl_.reset();
        initializer_.reset();
        scope_.reset();
    }
private:
    Decl decl_;
    Initializer initializer_;
    Scope scope_;
};

template <typename Decl, typename Initializer, typename Scope>
local_variable_def_pattern<Decl, Initializer, Scope> local_variable_def(Decl decl, Initializer initializer, Scope scope)
{
    return local_variable_def_pattern<Decl, Initializer, Scope>(decl, initializer, scope);
}

}
}
}

#endif