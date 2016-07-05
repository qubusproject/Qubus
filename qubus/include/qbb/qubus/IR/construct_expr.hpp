#ifndef QBB_QUBUS_CONSTRUCT_EXPR_HPP
#define QBB_QUBUS_CONSTRUCT_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class construct_expr : public expression_base<construct_expr>
{
public:
    construct_expr() = default;
    construct_expr(type result_type_, std::vector<expression> parameters_);

    const type& result_type() const;

    const std::vector<expression>& parameters() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & result_type_;
        ar & parameters_;
    }
private:
    type result_type_;
    std::vector<expression> parameters_;

    mutable annotation_map annotations_;
};

bool operator==(const construct_expr& lhs, const construct_expr& rhs);
bool operator!=(const construct_expr& lhs, const construct_expr& rhs);

}
}

#endif
