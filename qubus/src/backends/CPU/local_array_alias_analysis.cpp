#include <qbb/qubus/backends/local_array_alias_analysis.hpp>

#include <hpx/lcos/local/promise.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/util/handle.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/indirected.hpp>

#include <algorithm>
#include <utility>

namespace qbb
{
namespace qubus
{

index_expr::index_expr(util::index_t value_) : constant_(value_)
{
}

index_expr::index_expr(util::handle var_id) : coefficients_{{var_id, 1}}, constant_(0)
{
}

index_expr::index_expr(std::map<util::handle, util::index_t> coefficients_, util::index_t constant_)
: coefficients_(std::move(coefficients_)), constant_(constant_)
{
}

const std::map<util::handle, util::index_t>& index_expr::coefficients() const
{
    return coefficients_;
}

util::index_t index_expr::constant() const
{
    return constant_;
}

bool index_expr::is_const() const
{
    simplify();

    return coefficients_.empty();
}

bool index_expr::is_nonzero_const() const
{
    return is_const() && constant() != 0;
}

bool index_expr::is_zero() const
{
    return is_const() && constant() == 0;
}

void index_expr::simplify() const
{
    for (auto it = coefficients_.begin(); it != coefficients_.end();)
    {
        if (it->second == 0)
        {
            it = coefficients_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

index_expr operator+(index_expr lhs, index_expr rhs)
{
    auto new_coefficients = lhs.coefficients();

    for (const auto& entry : rhs.coefficients())
    {
        auto& value = new_coefficients[entry.first];

        value += entry.second;
    }

    util::index_t new_constant = lhs.constant() + rhs.constant();

    return index_expr(new_coefficients, new_constant);
}

index_expr operator-(index_expr lhs, index_expr rhs)
{
    auto new_coefficients = lhs.coefficients();

    for (const auto& entry : rhs.coefficients())
    {
        auto& value = new_coefficients[entry.first];

        value -= entry.second;
    }

    util::index_t new_constant = lhs.constant() - rhs.constant();

    return index_expr(new_coefficients, new_constant);
}

index_expr operator*(index_expr lhs, index_expr rhs)
{
    if (!lhs.coefficients().empty() && !rhs.coefficients().empty())
        throw 0;

    auto new_coefficients = !lhs.coefficients().empty() ? lhs.coefficients() : rhs.coefficients();
    util::index_t factor = !lhs.coefficients().empty() ? rhs.constant() : lhs.constant();

    for (auto& entry : new_coefficients)
    {
        entry.second *= factor;
    }

    util::index_t new_constant = lhs.constant() * rhs.constant();

    return index_expr(new_coefficients, new_constant);
}

index_expr operator-(index_expr arg)
{
    auto new_coefficients = arg.coefficients();

    for (auto& entry : new_coefficients)
    {
        entry.second = -entry.second;
    }

    util::index_t new_constant = -arg.constant();

    return index_expr(new_coefficients, new_constant);
}

bool operator==(const index_expr& lhs, const index_expr& rhs)
{
    return (lhs - rhs).is_zero();
}

bool operator!=(const index_expr& lhs, const index_expr& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const index_expr& lhs, const index_expr& rhs)
{
    lhs.simplify();
    rhs.simplify();

    const auto& lhs_cst = lhs.constant();
    const auto& rhs_cst = rhs.constant();

    return std::tie(lhs.coefficients(), lhs_cst) < std::tie(rhs.coefficients(), rhs_cst);
}

namespace
{

index_expr index_expr_to_isl_aff(const expression& expr)
{
    pattern::variable<expression> a, b;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<unary_op_tag> utag;
    pattern::variable<util::index_t> value;
    pattern::variable<variable_declaration> var;

    auto m = pattern::make_matcher<expression, index_expr>()
                 .case_(binary_operator(btag, a, b),
                        [&]
                        {
                            auto lhs = index_expr_to_isl_aff(a.get());
                            auto rhs = index_expr_to_isl_aff(b.get());

                            switch (btag.get())
                            {
                            case binary_op_tag::plus:
                                return lhs + rhs;
                            case binary_op_tag::minus:
                                return lhs - rhs;
                            case binary_op_tag::multiplies:
                                return lhs * rhs;
                            default:
                                throw 0;
                            }
                        })
                 .case_(unary_operator(utag, a),
                        [&]
                        {
                            auto arg = index_expr_to_isl_aff(a.get());

                            switch (utag.get())
                            {
                            case unary_op_tag::plus:
                                return arg;
                            case unary_op_tag::negate:
                                return -arg;
                            default:
                                throw 0;
                            }
                        })
                 .case_(integer_literal(value),
                        [&]
                        {
                            return index_expr(value.get());
                        })
                 .case_(variable_ref(var), [&]
                        {
                            return index_expr(var.get().id());
                        });

    return pattern::match(expr, m);
}

struct local_array_access_info
{
    local_array_access_info(variable_declaration accessed_array, reference data_ref,
                            std::vector<expression> indices)
    : accessed_array(accessed_array), data_ref(data_ref), indices(indices)
    {
    }

    variable_declaration accessed_array;
    reference data_ref;
    std::vector<expression> indices;

    hpx::lcos::local::promise<llvm::MDNode*> alias_scope_promise;
    hpx::lcos::local::promise<llvm::MDNode*> noalias_set_promise;
};

struct local_array_access_indices_subindex
{
    explicit local_array_access_indices_subindex(llvm::MDNode* scope) : scope(scope)
    {
    }

    void add(local_array_access_info& info)
    {
        children.push_back(&info);
    }

    llvm::MDNode* scope;
    std::vector<local_array_access_info*> children;
};

struct local_array_access_array_identity_subindex
{

    explicit local_array_access_array_identity_subindex(std::string domain_name,
                                                        llvm::MDNode* domain, llvm_environment* env)
    : domain_name(domain_name), domain(domain), env(env)
    {
    }

    void add(local_array_access_info& info)
    {
        std::vector<index_expr> idx_expr = index_expr_from_expression(info.indices);

        auto& entry = children[idx_expr];

        if (!entry)
        {
            std::string scope_name = domain_name + "." + std::to_string(children.size());

            llvm::MDNode* alias_scope = env->md_builder().createAliasScope(scope_name, domain);

            entry = util::make_unique<local_array_access_indices_subindex>(alias_scope);
        }

        entry->add(info);
    }

    std::string domain_name;
    llvm::MDNode* domain;
    llvm_environment* env;

    std::map<std::vector<index_expr>, std::unique_ptr<local_array_access_indices_subindex>>
        children;
};

struct local_array_access_index_root
{
    local_array_access_index_root(util::handle token, llvm_environment* env)
    : token(token), env(env)
    {
    }

    void add(local_array_access_info& info)
    {
        auto& entry = children[info.accessed_array];

        if (!entry)
        {
            std::string domain_name =
                "qubus.local_alias_domain." + token.str() + "." + info.accessed_array.id().str();

            llvm::MDNode* domain = env->md_builder().createAliasScopeDomain(domain_name);

            entry = util::make_unique<local_array_access_array_identity_subindex>(domain_name,
                                                                                  domain, env);
        }

        entry->add(info);
    }

    util::handle token;
    llvm_environment* env;
    std::map<variable_declaration, std::unique_ptr<local_array_access_array_identity_subindex>>
        children;
};

bool never_reference_same_element(const std::vector<index_expr>& acc1,
                                  const std::vector<index_expr>& acc2)
{
    if (acc1.size() != acc2.size())
    {
        return false;
    }

    for (std::size_t i = 0; i < acc1.size(); ++i)
    {
        auto diff = acc1[i] - acc2[i];

        if (diff.is_nonzero_const())
        {
            return true;
        }
    }

    return false;
}
}

bool contains_loops(const expression& expr)
{
    using pattern::_;

    auto m = pattern::make_matcher<expression, int>().case_(for_(_, _, _, _, _), [&]
                                                            {
                                                                return 0;
                                                            });

    return !!pattern::search(expr, m);
}

std::vector<index_expr> index_expr_from_expression(const std::vector<expression>& indices)
{
    std::vector<index_expr> result;

    for (const auto& index : indices)
    {
        result.push_back(index_expr_to_isl_aff(index));
    }

    return result;
}

class local_array_access_alias_analysis_impl
{
public:
    local_array_access_alias_analysis_impl(util::handle token_, llvm_environment& env_)
    : token_(token_), env_(&env_)
    {
    }

    ~local_array_access_alias_analysis_impl();

    local_array_access_alias_analysis_impl(const local_array_access_alias_analysis_impl&) = delete;
    local_array_access_alias_analysis_impl&
    operator=(const local_array_access_alias_analysis_impl&) = delete;

    alias_info query(variable_declaration accessed_array, std::vector<expression> indices,
                     reference data_ref);

private:
    void emit_noalias_info();

    util::handle token_;
    llvm_environment* env_;
    std::vector<local_array_access_info> access_table_;
};

local_array_access_alias_analysis_impl::~local_array_access_alias_analysis_impl()
{
    emit_noalias_info();
}

alias_info local_array_access_alias_analysis_impl::query(variable_declaration accessed_array,
                                                         std::vector<expression> indices,
                                                         reference data_ref)
{
    local_array_access_info info(accessed_array, data_ref, indices);

    hpx::lcos::shared_future<llvm::MDNode*> alias_scope = info.alias_scope_promise.get_future();
    hpx::lcos::shared_future<llvm::MDNode*> noalias_set = info.noalias_set_promise.get_future();

    access_table_.push_back(std::move(info));

    return alias_info(alias_scope, noalias_set);
}

void local_array_access_alias_analysis_impl::emit_noalias_info()
{
    using namespace boost::adaptors;

    local_array_access_index_root index(token_, env_);

    for (auto& entry : access_table_)
    {
        index.add(entry);
    }

    for (const auto& array_ident_subindex : index.children | map_values | indirected)
    {
        for (const auto& indices_subindex : array_ident_subindex.children)
        {
            std::vector<llvm::Metadata*> alias_set = {};

            for (const auto& indices_subindex2 : array_ident_subindex.children)
            {
                if (never_reference_same_element(indices_subindex.first, indices_subindex2.first))
                {
                    alias_set.push_back(indices_subindex2.second->scope);
                }
            }

            for (const auto& access_info : indices_subindex.second->children)
            {
                access_info->alias_scope_promise.set_value(indices_subindex.second->scope);
                access_info->noalias_set_promise.set_value(
                    llvm::MDNode::get(llvm::getGlobalContext(), alias_set));
            }
        }
    }
}

local_array_access_alias_analysis::local_array_access_alias_analysis(util::handle token_,
                                                                     llvm_environment& env_)
: impl_(util::make_unique<local_array_access_alias_analysis_impl>(token_, env_))
{
}

local_array_access_alias_analysis::~local_array_access_alias_analysis() = default;

alias_info local_array_access_alias_analysis::query(variable_declaration accessed_array,

                                                    std::vector<expression> indices,
                                                    reference data_ref)
{
    return impl_->query(std::move(accessed_array), std::move(indices), std::move(data_ref));
}
}
}
