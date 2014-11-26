#include <qbb/kubus/IR/macro_expr.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
macro_expr::macro_expr(std::vector<variable_declaration> params_, expression body_)
: params_(std::move(params_)), body_(std::move(body_))
{
}

const std::vector<variable_declaration>& macro_expr::params() const
{
    return params_;
}

const expression& macro_expr::body() const
{
    return body_;
}

std::vector<expression> macro_expr::sub_expressions() const
{
    return {body_};
}

expression macro_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    QBB_ASSERT(subexprs.size() == 1, "invalid number of subexpressions");
    
    return macro_expr(params(), subexprs[0]);
}

annotation_map& macro_expr::annotations() const
{
    return annotations_;
}

annotation_map& macro_expr::annotations()
{
    return annotations_;
}

bool operator==(const macro_expr& lhs, const macro_expr& rhs)
{
    return lhs.params() == rhs.params() && lhs.body() == rhs.body();
}

bool operator!=(const macro_expr& lhs, const macro_expr& rhs)
{
    return !(lhs == rhs);
}

expression expand_macro(const macro_expr& macro, const std::vector<expression>& args)
{
    using pattern::value;
    
    std::size_t n_params = macro.params().size();
    
    if (args.size() != n_params)
        throw 0; //wrong number of arguments
 
    auto body = macro.body();
 
    for (std::size_t i = 0; i < n_params; ++i)
    {
        auto m = pattern::make_matcher<expression, expression>()
                    .case_(variable_ref(value(macro.params()[i])), [&]
                        {
                            return args[i];
                        }
                    );
                    
        body = pattern::substitute(body, m);
    }
    
    return body;
}

}
}