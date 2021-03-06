#ifndef QUBUS_MEMBER_ACCESS_EXPR_HPP
#define QUBUS_MEMBER_ACCESS_EXPR_HPP

#include <qubus/IR/access_qualifier.hpp>
#include <qubus/util/unused.hpp>

#include <string>

namespace qubus
{

class member_access_expr final : public access_qualifier_base<member_access_expr>
{
public:
    member_access_expr() = default;
    member_access_expr(std::unique_ptr<access_expr> object_, std::string member_name_);

    virtual ~member_access_expr() = default;

    const access_expr& object() const;
    const std::string& member_name() const;

    const access_expr& qualified_access() const override final;

    member_access_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    std::unique_ptr<access_expr> object_;
    std::string member_name_;
};

inline std::unique_ptr<member_access_expr> member_access(std::unique_ptr<access_expr> object, std::string member_name)
{
    return std::make_unique<member_access_expr>(std::move(object), std::move(member_name));
}

bool operator==(const member_access_expr& lhs, const member_access_expr& rhs);
bool operator!=(const member_access_expr& lhs, const member_access_expr& rhs);

}

#endif