#ifndef QUBUS_QTL_IR_MULTI_INDEX_EXPR_HPP
#define QUBUS_QTL_IR_MULTI_INDEX_EXPR_HPP

#include <hpx/config.hpp>

#include <qubus/qtl/index.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/IR/access.hpp>

#include <memory>
#include <vector>

namespace qubus
{
namespace qtl
{

class multi_index_expr : public access_expr_base<multi_index_expr>
{
public:
    multi_index_expr() = default;
    explicit multi_index_expr(variable_declaration multi_index_,
                              std::vector<variable_declaration> element_indices_);

    virtual ~multi_index_expr() = default;

    const variable_declaration& multi_index() const;
    const std::vector<variable_declaration>& element_indices() const;

    multi_index_expr* clone() const override final;

    const expression& child(std::size_t index) const override final;

    std::size_t arity() const override final;

    std::unique_ptr<expression> substitute_subexpressions(
        std::vector<std::unique_ptr<expression>> new_children) const override final;

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& multi_index_;
        ar& element_indices_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(multi_index_expr);

private:
    variable_declaration multi_index_;
    std::vector<variable_declaration> element_indices_;

    mutable annotation_map annotations_;
};

bool operator==(const multi_index_expr& lhs, const multi_index_expr& rhs);
bool operator!=(const multi_index_expr& lhs, const multi_index_expr& rhs);

template <long int Rank>
std::unique_ptr<multi_index_expr> capture_multi_index(const multi_index<Rank>& idx)
{
    auto multi_index = idx.var();

    std::vector<variable_declaration> element_indices;
    element_indices.reserve(idx.rank());

    for (long int i = 0; i < idx.rank(); ++i)
    {
        element_indices.push_back(idx[i].var());
    }

    return std::make_unique<multi_index_expr>(std::move(multi_index), std::move(element_indices));
}
}
}

#endif
