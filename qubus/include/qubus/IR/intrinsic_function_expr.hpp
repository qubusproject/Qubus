#ifndef QUBUS_INTRINSIC_FUNCTION_EXPR_HPP
#define QUBUS_INTRINSIC_FUNCTION_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/util/unused.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <string>
#include <vector>

namespace qubus
{

class intrinsic_function_expr final : public expression_base<intrinsic_function_expr>
{
public:
    intrinsic_function_expr() = default;
    intrinsic_function_expr(std::string name_, std::vector<std::unique_ptr<expression>> args_);

    virtual ~intrinsic_function_expr() = default;

    const std::string& name() const;

    auto args() const
    {
        return args_ | boost::adaptors::indirected;
    }

    intrinsic_function_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
            std::vector<std::unique_ptr<expression>> new_children) const override final;

private:
    std::string name_;
    std::vector<std::unique_ptr<expression>> args_;
};

bool operator==(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs);
bool operator!=(const intrinsic_function_expr& lhs, const intrinsic_function_expr& rhs);

inline std::unique_ptr<intrinsic_function_expr> intrinsic_function(std::string name, std::vector<std::unique_ptr<expression>> args)
{
    return std::make_unique<intrinsic_function_expr>(std::move(name), std::move(args));
}

}

#endif