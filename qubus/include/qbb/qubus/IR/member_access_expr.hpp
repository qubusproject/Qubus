#ifndef QBB_QUBUS_MEMBER_ACCESS_EXPR_HPP
#define QBB_QUBUS_MEMBER_ACCESS_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/runtime/serialization/string.hpp>

#include <string>

namespace qbb
{
namespace qubus
{

class member_access_expr : public expression_base<member_access_expr>
{
public:
    member_access_expr() = default;
    member_access_expr(expression object_, std::string member_name_);

    const expression& object() const;
    const std::string& member_name() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & object_;
        ar & member_name_;
    }
private:
    expression object_;
    std::string member_name_;

    mutable annotation_map annotations_;
};

bool operator==(const member_access_expr& lhs, const member_access_expr& rhs);
bool operator!=(const member_access_expr& lhs, const member_access_expr& rhs);

}
}

#endif