#ifndef QBB_QUBUS_KRONECKER_DELTA_HPP
#define QBB_QUBUS_KRONECKER_DELTA_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{

class kronecker_delta_expr
{
public:
    kronecker_delta_expr() = default;
    kronecker_delta_expr(util::index_t extent_, expression first_index_, expression second_index_);

    util::index_t extent() const;
    const expression& first_index() const;
    const expression& second_index() const;

    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;

    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & extent_;
        ar & first_index_;
        ar & second_index_;
    }
private:
    util::index_t extent_;
    expression first_index_;
    expression second_index_;

    mutable annotation_map annotations_;
};

bool operator==(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs);
bool operator!=(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs);

template<>
struct is_expression<kronecker_delta_expr> : std::true_type
{
};

}
}

#endif
