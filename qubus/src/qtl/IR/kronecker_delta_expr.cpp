#include <qbb/qubus/qtl/IR/kronecker_delta_expr.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qubus
{
namespace qtl
{

kronecker_delta_expr::kronecker_delta_expr(util::index_t extent_,
                                           std::unique_ptr<expression> first_index_,
                                           std::unique_ptr<expression> second_index_)
: extent_{std::move(extent_)}, first_index_(take_over_child(first_index_)),
  second_index_(take_over_child(second_index_))
{
}

util::index_t kronecker_delta_expr::extent() const
{
    return extent_;
}

const expression& kronecker_delta_expr::first_index() const
{
    return *first_index_;
}

const expression& kronecker_delta_expr::second_index() const
{
    return *second_index_;
}

kronecker_delta_expr* kronecker_delta_expr::clone() const
{
    return new kronecker_delta_expr(extent_, qubus::clone(*first_index_),
                                    qubus::clone(*second_index_));
}

const expression& kronecker_delta_expr::child(std::size_t index) const
{
    if (index == 0)
    {
        return *first_index_;
    }
    else if (index == 1)
    {
        return *second_index_;
    }
    else
    {
        throw 0;
    }
}

std::size_t kronecker_delta_expr::arity() const
{
    return 2;
}

std::unique_ptr<expression> kronecker_delta_expr::substitute_subexpressions(
    std::vector<std::unique_ptr<expression>> new_children) const
{
    if (new_children.size() != 2)
        throw 0;

    return std::make_unique<kronecker_delta_expr>(extent_, std::move(new_children[0]),
                                                  std::move(new_children[1]));
}

bool operator==(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs)
{
    return lhs.extent() == rhs.extent() && lhs.first_index() == rhs.first_index() &&
           lhs.second_index() == rhs.second_index();
}

bool operator!=(const kronecker_delta_expr& lhs, const kronecker_delta_expr& rhs)
{
    return !(lhs == rhs);
}

QBB_DEFINE_MULTI_METHOD_SPECIALIZATION_WITH_NAME(equal, std::equal_to<kronecker_delta_expr>(), kronecker_delta_expr_equal);

}
}
