#ifndef QBB_KUBUS_INTRINSIC_FUNCTION_EXPR_HPP
#define QBB_KUBUS_INTRINSIC_FUNCTION_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <string>
#include <vector>

namespace qbb
{
namespace kubus
{

class intrinsic_function_expr
{
public:
    intrinsic_function_expr(std::string name_, std::vector<expression> args_);

    const std::string& name() const;

    const std::vector<expression>& args() const;

    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::string name_;
    std::vector<expression> args_;
    
    mutable annotation_map annotations_;
};
}
}

#endif