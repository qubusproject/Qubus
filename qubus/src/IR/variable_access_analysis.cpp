#include <qubus/IR/variable_access_analysis.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/optional_ref.hpp>
#include <qubus/util/unreachable.hpp>

#include <algorithm>
#include <stack>
#include <unordered_map>
#include <utility>

namespace qubus
{

namespace
{
std::tuple<std::reference_wrapper<const variable_ref_expr>,
           std::vector<std::reference_wrapper<const access_qualifier_expr>>>
decompose_access(const access_expr& access)
{
    if (auto variable_ref = access.try_as<variable_ref_expr>())
    {
        return {*variable_ref, {}};
    }
    else if (auto qualifier = access.try_as<access_qualifier_expr>())
    {
        util::optional_ref<const variable_ref_expr> direct_access;

        std::vector<std::reference_wrapper<const access_qualifier_expr>> qualifiers;

        qualifiers.push_back(*qualifier);

        const access_qualifier_expr* current_qualifier = qualifier;

        for (;;)
        {
            const auto& next_expression = current_qualifier->qualified_access();

            if (auto qualifier = next_expression.try_as<access_qualifier_expr>())
            {
                qualifiers.push_back(*qualifier);

                current_qualifier = qualifier;
            }
            else
            {
                direct_access = current_qualifier->qualified_access().as<variable_ref_expr>();

                break;
            }
        }

        return {*direct_access, std::move(qualifiers)};
    }

    QUBUS_UNREACHABLE_BECAUSE("'access' is of an unknown access type.");
}
}

access::access(const access_expr& access_) : access(decompose_access(access_))
{
}

access::access(const variable_ref_expr& direct_access_,
               std::vector<std::reference_wrapper<const access_qualifier_expr>> qualifiers_)
: direct_access_(direct_access_),
  qualifiers_(std::move(qualifiers_)),
  location_([this]() -> const expression& {
      if (this->qualifiers_.empty())
      {
          return this->direct_access_;
      }
      else
      {
          return this->qualifiers_.back();
      }
  }())
{
}

const expression& access::location() const
{
    return location_;
}

access::access(std::tuple<std::reference_wrapper<const variable_ref_expr>,
                          std::vector<std::reference_wrapper<const access_qualifier_expr>>>
                   data_)
: access(std::get<0>(data_), std::move(std::get<1>(data_)))
{
}

class access_set_node
{
public:
    access_set_node(const expression& location_, std::vector<access> local_read_accesses_,
                    std::vector<access> local_write_accesses_,
                    std::vector<std::unique_ptr<access_set_node>> subsets_)
    : location_(location_),
      local_read_accesses_(std::move(local_read_accesses_)),
      local_write_accesses_(std::move(local_write_accesses_)),
      subsets_(std::move(subsets_))
    {
    }

    access_set_node(const access_set_node&) = delete;
    access_set_node& operator=(const access_set_node&) = delete;

    access_set_node(access_set_node&&) = delete;
    access_set_node& operator=(access_set_node&&) = delete;

    const expression& location() const
    {
        return location_;
    }

    std::vector<access> get_read_accesses(access_kind kind) const
    {
        using pattern::_;

        std::vector<access> read_accesses = local_read_accesses_;

        for (const auto& subset : subsets_)
        {
            auto subset_accesses = subset->get_read_accesses(kind);

            read_accesses.insert(read_accesses.end(), subset_accesses.begin(),
                                 subset_accesses.end());
        }

        if (kind == access_kind::external)
        {
            // TODO: Filter out internal accesses.
        }

        return read_accesses;
    }

    std::vector<access> get_write_accesses(access_kind kind) const
    {
        using pattern::_;

        std::vector<access> write_accesses = local_write_accesses_;

        for (const auto& subset : subsets_)
        {
            auto subset_accesses = subset->get_write_accesses(kind);

            write_accesses.insert(write_accesses.end(), subset_accesses.begin(),
                                  subset_accesses.end());
        }

        if (kind == access_kind::external)
        {
            // TODO: Filter out internal accesses.
        }

        return write_accesses;
    }

    auto subsets() const
    {
        return subsets_ | boost::adaptors::indirected;
    }

private:
    std::reference_wrapper<const expression> location_;

    std::vector<access> local_read_accesses_;
    std::vector<access> local_write_accesses_;

    std::vector<std::unique_ptr<access_set_node>> subsets_;
};

access_set::access_set(const access_set_node& root_, access_kind kind_)
: root_(&root_), kind_(kind_)
{
}

const expression& access_set::location() const
{
    return root_->location();
};

std::vector<access> access_set::get_read_accesses() const
{
    return root_->get_read_accesses(kind_);
}

std::vector<access> access_set::get_write_accesses() const
{
    return root_->get_write_accesses(kind_);
}

class variable_access_index
{
public:
    explicit variable_access_index(std::unique_ptr<access_set_node> global_access_set_)
    : global_access_set_(std::move(global_access_set_))
    {
        add_set_to_index(*this->global_access_set_);
    }

    access_set query_accesses_for_location(const expression& location, access_kind kind) const
    {
        auto search_result = location_index_.find(&location);

        QUBUS_ASSERT(search_result != location_index_.end(), "Invalid location during query.");

        return access_set(*search_result->second, kind);
    }

private:
    void add_set_to_index(const access_set_node& set)
    {
        const expression& location = set.location();

        location_index_.emplace(&location, &set);

        for (const auto& subset : set.subsets())
        {
            add_set_to_index(subset);
        }
    }

    std::unique_ptr<access_set_node> global_access_set_;

    std::unordered_map<const expression*, const access_set_node*> location_index_;
};

variable_access_analyis_result::variable_access_analyis_result(
    std::unique_ptr<variable_access_index> access_index_)
: access_index_(std::move(access_index_))
{
}

variable_access_analyis_result::~variable_access_analyis_result() = default;

variable_access_analyis_result::variable_access_analyis_result(variable_access_analyis_result&&) =
    default;
variable_access_analyis_result& variable_access_analyis_result::
operator=(variable_access_analyis_result&&) = default;

access_set variable_access_analyis_result::query_accesses_for_location(const expression& location,
                                                                       access_kind kind) const
{
    return access_index_->query_accesses_for_location(location, kind);
}

namespace
{

std::vector<access> get_accesses(const expression& expr)
{
    std::vector<access> accesses;

    std::stack<const expression*> pending_expressions;

    pending_expressions.push(&expr);

    while (!pending_expressions.empty())
    {
        auto current_expr = pending_expressions.top();
        pending_expressions.pop();

        if (auto acc = current_expr->try_as<access_expr>())
        {
            accesses.emplace_back(*acc);

            if (auto qualified_acc = acc->try_as<access_qualifier_expr>())
            {
                for (const auto& child : current_expr->sub_expressions())
                {
                    if (&child != &qualified_acc->qualified_access())
                    {
                        pending_expressions.push(&child);
                    }
                }
            }
            else
            {
                for (const auto& child : current_expr->sub_expressions())
                {
                    pending_expressions.push(&child);
                }
            }
        }
        else
        {
            for (const auto& child : current_expr->sub_expressions())
            {
                pending_expressions.push(&child);
            }
        }
    }

    return accesses;
}

struct access_info
{
    access_info(access direct_access, std::vector<access> indirect_accesses)
    : direct_access(std::move(direct_access)), indirect_accesses(std::move(indirect_accesses))
    {
    }

    access direct_access;
    std::vector<access> indirect_accesses;
};

access_info analyze_access(const access_expr& acc)
{
    if (auto qual_access = acc.try_as<access_qualifier_expr>())
    {
        auto direct_access = access(*qual_access);

        std::vector<access> indirect_accesses;

        for (const auto& child : acc.sub_expressions())
        {
            if (&child != &qual_access->qualified_access())
            {
                auto accesses = get_accesses(child);

                indirect_accesses.insert(indirect_accesses.end(), accesses.begin(), accesses.end());
            }
        }

        return access_info(std::move(direct_access), std::move(indirect_accesses));
    }
    else
    {
        auto direct_access = access(acc);

        std::vector<access> indirect_accesses;

        for (const auto& child : acc.sub_expressions())
        {
            auto accesses = get_accesses(child);

            indirect_accesses.insert(indirect_accesses.end(), accesses.begin(), accesses.end());
        }

        return access_info(std::move(direct_access), std::move(indirect_accesses));
    }
}

std::unique_ptr<access_set_node> compute_access_set(const expression& expr)
{
    using pattern::_;

    pattern::variable<const expression &> lhs, rhs;

    std::vector<access> local_read_accesses;
    std::vector<access> local_write_accesses;
    std::vector<std::unique_ptr<access_set_node>> subsets;

    auto m = pattern::make_matcher<expression, void>()
                 .case_(assign(lhs, rhs),
                        [&] {
                            auto acc_info = analyze_access(lhs.get().as<access_expr>());

                            local_write_accesses.push_back(std::move(acc_info.direct_access));

                            local_read_accesses = get_accesses(rhs.get());
                            local_read_accesses.insert(local_read_accesses.end(),
                                                       acc_info.indirect_accesses.begin(),
                                                       acc_info.indirect_accesses.end());
                        })
                 .case_(plus_assign(lhs, rhs),
                        [&] {
                            auto acc_info = analyze_access(lhs.get().as<access_expr>());

                            local_write_accesses.push_back(acc_info.direct_access);

                            local_read_accesses = get_accesses(rhs.get());
                            local_read_accesses.insert(local_read_accesses.end(),
                                                       acc_info.indirect_accesses.begin(),
                                                       acc_info.indirect_accesses.end());
                            local_read_accesses.push_back(std::move(acc_info.direct_access));
                        })
                 // FIXME: Add spawn
                 .case_(_, [&](const expression& self) {
                     for (std::size_t i = 0; i < self.arity(); ++i)
                     {
                         subsets.push_back(compute_access_set(self.child(i)));
                     }
                 });

    pattern::match(expr, m);

    return std::make_unique<access_set_node>(expr, std::move(local_read_accesses),
                                             std::move(local_write_accesses), std::move(subsets));
}
}

variable_access_analyis_result
variable_access_analysis::run(const expression& root, expression_analysis_manager& manager,
                              pass_resource_manager& resource_manager) const
{
    auto global_access_set = compute_access_set(root);

    auto access_index = std::make_unique<variable_access_index>(std::move(global_access_set));

    return variable_access_analyis_result(std::move(access_index));
}

std::vector<analysis_id> variable_access_analysis::required_analyses() const
{
    return {};
}

QUBUS_REGISTER_EXPRESSION_ANALYSIS_PASS(variable_access_analysis);
}
