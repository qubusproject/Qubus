#include <qbb/qubus/IR/construct_expr.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

construct_expr::construct_expr(type result_type_, std::vector<expression> parameters_)
        : result_type_{std::move(result_type_)}, parameters_(std::move(parameters_))
{
}

const type& construct_expr::result_type() const
{
    return result_type_;
}

const std::vector<expression>& construct_expr::parameters() const
{
    return parameters_;
}

std::vector<expression> construct_expr::sub_expressions() const
{
    std::vector<expression> sub_exprs;
    sub_exprs.reserve(parameters_.size());

    sub_exprs.insert(sub_exprs.end(), parameters_.begin(), parameters_.end());

    return sub_exprs;
}

expression construct_expr::substitute_subexpressions(const std::vector<expression>& subexprs) const
{
    std::vector<expression> new_paramters;

    for(std::size_t i = 0; i < subexprs.size(); ++i)
    {
        new_paramters.emplace_back(subexprs[i]);
    }

    return construct_expr(result_type_, new_paramters);
}

annotation_map& construct_expr::annotations() const
{
    return annotations_;
}

annotation_map& construct_expr::annotations()
{
    return annotations_;
}

bool operator==(const construct_expr& lhs, const construct_expr& rhs)
{
    return lhs.result_type() == rhs.result_type() && lhs.parameters() == rhs.parameters();
}

bool operator!=(const construct_expr& lhs, const construct_expr& rhs)
{
    return !(lhs == rhs);
}

}
}

