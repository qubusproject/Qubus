#ifndef QUBUS_QTL_IR_OBJECT_EXPR_HPP
#define QUBUS_QTL_IR_OBJECT_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/object.hpp>

#include <qubus/IR/access.hpp>

#include <memory>

namespace qubus
{
namespace qtl
{

class object_expr final : public access_expr_base<object_expr>
{
public:
    object_expr() = default;
    explicit object_expr(object obj_);

    virtual ~object_expr() = default;

    const object& obj() const;

    object_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    object obj_;

    mutable annotation_map annotations_;
};

bool operator==(const object_expr& lhs, const object_expr& rhs);
bool operator!=(const object_expr& lhs, const object_expr& rhs);

inline std::unique_ptr<object_expr> obj(object obj)
{
    return std::make_unique<object_expr>(std::move(obj));
}

}
}

#endif
