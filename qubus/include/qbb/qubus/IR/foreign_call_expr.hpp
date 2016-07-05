#ifndef QBB_QUBUS_FOREIGN_CALL_EXPR_HPP
#define QBB_QUBUS_FOREIGN_CALL_EXPR_HPP

#include <qbb/qubus/foreign_computelet.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/string.hpp>
#include <hpx/runtime/serialization/vector.hpp>

#include <string>
#include <vector>

namespace qbb
{
namespace qubus
{

class foreign_call_expr : public expression_base<foreign_call_expr>
{
public:
    foreign_call_expr() = default;
    foreign_call_expr(foreign_computelet computelet_, std::vector<expression> args_);

    type result_type() const;
    const foreign_computelet& computelet() const;
    const std::vector<expression>& args() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & computelet_;
        ar & args_;
    }
private:
    foreign_computelet computelet_;
    std::vector<expression> args_;

    mutable annotation_map annotations_;
};

bool operator==(const foreign_call_expr& lhs, const foreign_call_expr& rhs);
bool operator!=(const foreign_call_expr& lhs, const foreign_call_expr& rhs);

}
}

#endif
