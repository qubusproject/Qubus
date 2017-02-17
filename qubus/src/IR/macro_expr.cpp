#include <qubus/IR/macro_expr.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/IR.hpp>

#include <qubus/util/assert.hpp>

#include <utility>

namespace qubus
{
macro_expr::macro_expr(std::vector<variable_declaration> params_, std::unique_ptr<expression> body_)
: params_(std::move(params_)), body_(std::move(body_))
{
}

const std::vector<variable_declaration>& macro_expr::params() const
{
    return params_;
}

const expression& macro_expr::body() const
{
    return *body_;
}

macro_expr* macro_expr::clone() const
{
    return new macro_expr(params_, qubus::clone(*body_));
}

const expression& macro_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *body_;
    }
    else
    {
        throw 0;
    }
}

std::size_t macro_expr::arity() const
{
    return 1;
}

std::unique_ptr<expression> macro_expr::substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 1)
        throw 0;

    return std::make_unique<macro_expr>(params_, std::move(new_children[0]));
}

bool operator==(const macro_expr& lhs, const macro_expr& rhs)
{
    return lhs.params() == rhs.params() && lhs.body() == rhs.body();
}

bool operator!=(const macro_expr& lhs, const macro_expr& rhs)
{
    return !(lhs == rhs);
}

std::unique_ptr<expression> expand_macro(const macro_expr& macro, std::vector<std::unique_ptr<expression>> args)
{
    using pattern::value;
    
    std::size_t n_params = macro.params().size();
    
    if (args.size() != n_params)
        throw 0; //wrong number of arguments
 
    auto body = clone(macro.body());
 
    for (std::size_t i = 0; i < n_params; ++i)
    {
        auto m = pattern::make_matcher<expression, std::unique_ptr<expression>>()
                    .case_(variable_ref(value(macro.params()[i])), [&]
                        {
                            return clone(*args[i]);
                        }
                    );
                    
        body = pattern::substitute(*body, m);
    }
    
    return body;
}

}