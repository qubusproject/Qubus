#ifndef QUBUS_AXIOM_ANALYSIS_HPP
#define QUBUS_AXIOM_ANALYSIS_HPP

#include <qubus/pass_manager.hpp>

#include <qubus/IR/expression.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace qubus
{

class axiom
{
public:
    explicit axiom(std::unique_ptr<const expression> expr_);
    const expression& as_expr() const;

private:
    std::shared_ptr<const expression> expr_;
};

class axiom_scope
{
public:
    axiom_scope(const expression& context_, std::vector<axiom> axioms_,
                std::vector<std::unique_ptr<axiom_scope>> subscopes_);

    const expression& context() const
    {
        return *context_;
    }

    const std::vector<axiom>& axioms() const
    {
        return axioms_;
    }

    const axiom_scope* parent() const
    {
        return parent_;
    }

    void set_parent(const axiom_scope* parent)
    {
        parent_ = parent;
    }

    auto subscopes() const
    {
        return subscopes_ | boost::adaptors::indirected;
    }

    std::size_t arity() const
    {
        return subscopes_.size();
    }

private:
    const expression* context_;
    std::vector<axiom> axioms_;

    const axiom_scope* parent_;
    std::vector<std::unique_ptr<axiom_scope>> subscopes_;
};

class axiom_analysis_result
{
public:
    explicit axiom_analysis_result(std::unique_ptr<axiom_scope> root_scope_);

    std::vector<axiom> get_valid_axioms(const expression& ctx) const;

private:
    std::unique_ptr<axiom_scope> root_scope_;

    std::unordered_map<const expression*, const axiom_scope*> axiom_scope_index_;
};

class axiom_analysis_pass
{
public:
    using result_type = axiom_analysis_result;

    axiom_analysis_result run(const expression& root, analysis_manager& manager,
                              pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
