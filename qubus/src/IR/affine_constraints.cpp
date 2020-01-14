#include <qubus/affine_constraints.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/isl/value.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace qubus
{

namespace
{
class affine_expr_variable final : public expression_base<affine_expr_variable>
{
public:
    affine_expr_variable() = default;

    explicit affine_expr_variable(std::string name_) : name_(std::move(name_))
    {
    }

    const std::string& name() const
    {
        return name_;
    }

    affine_expr_variable* clone() const override
    {
        return new affine_expr_variable(name_);
    }

    const expression& child(std::size_t index) const override
    {
        throw 0;
    }

    std::size_t arity() const override
    {
        return 0;
    }

    std::unique_ptr<expression>
    substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const override
    {
        return {};
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& name_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(affine_expr_variable);

private:
    std::string name_;
};

template <typename Name>
class affine_variable_pattern
{
public:
    explicit affine_variable_pattern(Name name_) : name_(std::move(name_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const pattern::variable<const affine_expr_variable&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<affine_expr_variable>())
        {
            if (name_.match(concret_value->name()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        name_.reset();
    }

private:
    Name name_;
};

template <typename Name>
affine_variable_pattern<Name> affine_variable(Name name)
{
    return affine_variable_pattern<Name>(std::move(name));
}

class affine_expr_constant final : public expression_base<affine_expr_constant>
{
public:
    affine_expr_constant() = default;

    explicit affine_expr_constant(std::string name_) : name_(std::move(name_))
    {
    }

    const std::string& name() const
    {
        return name_;
    }

    affine_expr_constant* clone() const override
    {
        return new affine_expr_constant(name_);
    }

    const expression& child(std::size_t index) const override
    {
        throw 0;
    }

    std::size_t arity() const override
    {
        return 0;
    }

    std::unique_ptr<expression>
    substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const override
    {
        return {};
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& name_;
    }

    HPX_SERIALIZATION_POLYMORPHIC(affine_expr_constant);

private:
    std::string name_;
};

template <typename Name>
class affine_constant_pattern
{
public:
    explicit affine_constant_pattern(Name name_) : name_(std::move(name_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const pattern::variable<const affine_expr_constant&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<affine_expr_constant>())
        {
            if (name_.match(concret_value->name()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        name_.reset();
    }

private:
    Name name_;
};

template <typename Name>
affine_constant_pattern<Name> affine_constant(Name name)
{
    return affine_constant_pattern<Name>(std::move(name));
}

class affine_expr_wrapper final : public expression_base<affine_expr_wrapper>
{
public:
    affine_expr_wrapper() = default;

    explicit affine_expr_wrapper(affine_expr value_) : value_(std::move(value_))
    {
    }

    const affine_expr& get() const
    {
        return value_;
    }

    affine_expr_wrapper* clone() const override
    {
        return new affine_expr_wrapper(value_);
    }

    const expression& child(std::size_t index) const override
    {
        throw 0;
    }

    std::size_t arity() const override
    {
        return 0;
    }

    std::unique_ptr<expression>
    substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const override
    {
        return {};
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        QUBUS_UNREACHABLE_BECAUSE("This type should never participate in serialization.");
    }

    HPX_SERIALIZATION_POLYMORPHIC(affine_expr_wrapper);

private:
    affine_expr value_;
};

template <typename Value>
class affine_expr_pattern
{
public:
    explicit affine_expr_pattern(Value value_) : value_(std::move(value_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const pattern::variable<const affine_expr_wrapper&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<affine_expr_wrapper>())
        {
            if (value_.match(concret_value->get()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

    void reset() const
    {
        value_.reset();
    }

private:
    Value value_;
};

template <typename Value>
affine_expr_pattern<Value> wrapped_affine_expr(Value value)
{
    return affine_expr_pattern<Value>(std::move(value));
}

class extract_expression_pattern
{
public:
    explicit extract_expression_pattern(pattern::variable<const expression&> value_)
    : value_(std::move(value_))
    {
    }

    bool match(const affine_expr& value,
               const pattern::variable<const affine_expr&>* var = nullptr) const
    {
        value_.set(value.underlying_expression());

        if (var)
        {
            var->set(value);
        }

        return true;
    }

    void reset() const
    {
        value_.reset();
    }

private:
    pattern::variable<const expression&> value_;
};

extract_expression_pattern extract_expression(pattern::variable<const expression&> value)
{
    return extract_expression_pattern(std::move(value));
}

isl::affine_expr convert_to_isl_aff_expr(const expression& expr, const isl::space& s)
{
    pattern::variable<unary_op_tag> utag;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<util::index_t> ival;
    pattern::variable<const expression &> lhs, rhs, arg;
    pattern::variable<std::string> name;

    auto m = pattern::make_matcher<expression, isl::affine_expr>()
                 .case_(binary_operator(btag, lhs, rhs),
                        [&] {
                            auto lhs_expr = convert_to_isl_aff_expr(lhs.get(), s);
                            auto rhs_expr = convert_to_isl_aff_expr(rhs.get(), s);

                            switch (btag.get())
                            {
                            case binary_op_tag::plus:
                                return lhs_expr + rhs_expr;
                            case binary_op_tag::minus:
                                return lhs_expr - rhs_expr;
                            case binary_op_tag::multiplies:
                                return lhs_expr * rhs_expr;
                            case binary_op_tag::divides:
                                return lhs_expr / rhs_expr;
                            case binary_op_tag::modulus:
                            {
                                QUBUS_ASSERT(is_cst(rhs_expr), "The right-hand side has to be a literal value.");

                                return lhs_expr % rhs_expr.constant_value();
                            }
                            default:
                                QUBUS_UNREACHABLE();
                            }
                        })
                 .case_(unary_operator(utag, arg),
                        [&] {
                            auto arg_expr = convert_to_isl_aff_expr(arg.get(), s);

                            switch (utag.get())
                            {
                            case unary_op_tag::plus:
                                return arg_expr;
                            case unary_op_tag::negate:
                                return -arg_expr;
                            default:
                                QUBUS_UNREACHABLE();
                            }
                        })
                 .case_(integer_literal(ival),
                        [&] {
                            util::index_t val = ival.get();

                            // TODO: Check for overflow during conversion.
                            return isl::affine_expr(s, isl::value(s.get_ctx(), val));
                        })
                 .case_(affine_constant(name),
                        [&] {
                            int pos = s.find_dim_by_name(isl_dim_param, name.get());

                            QUBUS_ASSERT(pos >= 0, "Invalid parameter.");

                            return isl::affine_expr(s, isl_dim_param, pos);
                        })
                 .case_(affine_variable(name), [&] {
                     int pos = s.find_dim_by_name(isl_dim_set, name.get());

                     QUBUS_ASSERT(pos >= 0, "Invalid variable.");

                     return isl::affine_expr(s, isl_dim_set, pos);
                 });

    return pattern::match(expr, m);
}

isl::set convert_to_isl_set(const expression& expr, const isl::space& s)
{
    using pattern::_;

    pattern::variable<const expression &> lhs, rhs, arg;

    auto m =
        pattern::make_matcher<expression, isl::set>()
            .case_(pattern::equal_to(wrapped_affine_expr(extract_expression(lhs)),
                                     wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint = isl::constraint::equality(lhs_aff_expr - rhs_aff_expr);

                       true_domain.add_constraint(constraint);

                       return true_domain;
                   })
            .case_(pattern::not_equal_to(wrapped_affine_expr(extract_expression(lhs)),
                                         wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint = isl::constraint::equality(lhs_aff_expr - rhs_aff_expr);

                       true_domain.add_constraint(constraint);

                       return complement(true_domain);
                   })
            .case_(pattern::less(wrapped_affine_expr(extract_expression(lhs)),
                                 wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint =
                           isl::constraint::inequality(-(lhs_aff_expr - rhs_aff_expr + 1));

                       true_domain.add_constraint(constraint);

                       return true_domain;
                   })
            .case_(pattern::greater(wrapped_affine_expr(extract_expression(lhs)),
                                    wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint =
                           isl::constraint::inequality(lhs_aff_expr - rhs_aff_expr + 1);

                       true_domain.add_constraint(constraint);

                       return true_domain;
                   })
            .case_(pattern::less_equal(wrapped_affine_expr(extract_expression(lhs)),
                                       wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint =
                           isl::constraint::inequality(-(lhs_aff_expr - rhs_aff_expr));

                       true_domain.add_constraint(constraint);

                       return true_domain;
                   })
            .case_(pattern::greater_equal(wrapped_affine_expr(extract_expression(lhs)),
                                          wrapped_affine_expr(extract_expression(rhs))),
                   [&] {
                       auto lhs_aff_expr = convert_to_isl_aff_expr(lhs.get(), s);
                       auto rhs_aff_expr = convert_to_isl_aff_expr(rhs.get(), s);

                       auto true_domain = isl::set::universe(s);

                       auto constraint = isl::constraint::inequality(lhs_aff_expr - rhs_aff_expr);

                       true_domain.add_constraint(constraint);

                       return true_domain;
                   })
            .case_(logical_and(lhs, rhs),
                   [&] {
                       auto lhs_domain = convert_to_isl_set(lhs.get(), s);
                       auto rhs_domain = convert_to_isl_set(rhs.get(), s);

                       return intersect(lhs_domain, rhs_domain);
                   })
            .case_(logical_or(lhs, rhs),
                   [&] {
                       auto lhs_domain = convert_to_isl_set(lhs.get(), s);
                       auto rhs_domain = convert_to_isl_set(rhs.get(), s);

                       return union_(lhs_domain, rhs_domain);
                   })
            .case_(logical_not(arg), [&] {
                auto arg_domain = convert_to_isl_set(arg.get(), s);

                return complement(arg_domain);
            });

    return pattern::match(expr, m);
}
}

affine_expr::affine_expr(std::unique_ptr<expression> expr_, affine_expr_context& ctx_)
: expr_(std::move(expr_)), ctx_(&ctx_)
{
}

affine_expr::affine_expr(const affine_expr& other) : expr_(clone(*other.expr_)), ctx_(other.ctx_)
{
}

affine_expr::affine_expr(affine_expr&& other) noexcept
: expr_(std::move(other.expr_)), ctx_(other.ctx_)
{
}

affine_expr& affine_expr::operator=(const affine_expr& other)
{
    this->expr_ = clone(*other.expr_);
    this->ctx_ = other.ctx_;

    return *this;
}

affine_expr& affine_expr::operator=(affine_expr&& other) noexcept
{
    this->expr_ = std::move(other.expr_);
    this->ctx_ = other.ctx_;

    return *this;
}

const expression& affine_expr::underlying_expression() const
{
    return *expr_;
}

isl::affine_expr affine_expr::convert(isl::context& isl_ctx) const
{
    auto s = ctx_->construct_corresponding_space(isl_ctx);

    return convert_to_isl_aff_expr(*expr_, s);
}

affine_expr operator+(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = std::move(lhs.expr_) + std::move(rhs.expr_);

    return affine_expr(std::move(new_expr), *lhs.ctx_);
}

affine_expr operator-(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = std::move(lhs.expr_) - std::move(rhs.expr_);

    return affine_expr(std::move(new_expr), *lhs.ctx_);
}

affine_expr operator*(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(is_const(lhs) || is_const(rhs),
               "At least one of the operands needs to be constant to form an affine expression.");

    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = std::move(lhs.expr_) * std::move(rhs.expr_);

    return affine_expr(std::move(new_expr), *lhs.ctx_);
}

affine_expr operator/(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(is_const(rhs), "The divisor needs to be constant to form an affine expression");

    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = std::move(lhs.expr_) / std::move(rhs.expr_);

    return affine_expr(std::move(new_expr), *lhs.ctx_);
}

affine_expr operator%(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(is_const(rhs), "The divisor needs to be constant to form an affine expression");

    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = std::move(lhs.expr_) % std::move(rhs.expr_);

    return affine_expr(std::move(new_expr), *lhs.ctx_);
}

affine_expr operator-(affine_expr arg)
{
    auto new_expr = -std::move(arg.expr_);

    return affine_expr(std::move(new_expr), *arg.ctx_);
}

bool is_const(const affine_expr& expr)
{
    using pattern::_;

    auto m = pattern::make_matcher<expression, bool>()
                 .case_(affine_constant(_), [] { return true; })
                 .case_(integer_literal(_), [] { return true; })
                 .case_(_, [] { return false; });

    return pattern::match(*expr.expr_, m);
}

bool is_variable(const affine_expr& expr)
{
    using pattern::_;

    auto m = pattern::make_matcher<expression, bool>()
            .case_(affine_variable(_), [] { return true; })
            .case_(_, [] { return false; });

    return pattern::match(*expr.expr_, m);
}

std::string get_variable_name(const affine_expr& expr)
{
    using pattern::_;

    pattern::variable<std::string> name;

    auto m = pattern::make_matcher<expression, std::string>()
            .case_(affine_variable(name), [&] { return name.get(); })
            .case_(_, [] () -> std::string { throw 0; });

    return pattern::match(*expr.expr_, m);
}

affine_expr_context::affine_expr_context(std::function<bool(const expression&)> invariant_checker_)
: invariant_checker_(std::move(invariant_checker_))
{
}

affine_expr affine_expr_context::declare_variable(const variable_declaration& var)
{
    auto search_result =
            std::find_if(variable_table_.begin(), variable_table_.end(),
                         [&](const auto& entry) { return std::get<0>(entry) == var; });

    if (search_result != variable_table_.end())
    {
        return affine_expr(std::make_unique<affine_expr_variable>(std::get<1>(*search_result)), *this);
    }
    else
    {
        auto name = "i" + std::to_string(free_variable_id_++);

        variable_table_.emplace_back(var, std::move(name));
        const auto& new_entry = variable_table_.back();

        return affine_expr(std::make_unique<affine_expr_variable>(std::get<1>(new_entry)), *this);
    }
}

affine_expr affine_expr_context::define_constant(const expression& value)
{
    auto search_result =
        std::find_if(constant_table_.begin(), constant_table_.end(),
                     [&](const auto& entry) { return *std::get<0>(entry) == value; });

    if (search_result != constant_table_.end())
    {
        return affine_expr(std::make_unique<affine_expr_constant>(std::get<1>(*search_result)),
                           *this);
    }
    else
    {
        auto name = "p" + std::to_string(free_constant_id_++);

        constant_table_.emplace_back(&value, std::move(name));

        return affine_expr(
            std::make_unique<affine_expr_constant>(std::get<1>(constant_table_.back())), *this);
    }
}

boost::optional<variable_declaration>
affine_expr_context::lookup_variable(const std::string& name) const
{
    auto search_result = std::find_if(variable_table_.begin(), variable_table_.end(),
                                      [&](const auto& entry) { return std::get<1>(entry) == name; });

    if (search_result != variable_table_.end())
    {
        return std::get<0>(*search_result);
    }
    else
    {
        return boost::none;
    }
}

std::unique_ptr<expression> affine_expr_context::get_constant(const std::string& name) const
{
    auto search_result =
        std::find_if(constant_table_.begin(), constant_table_.end(),
                     [&](const auto& entry) { return std::get<1>(entry) == name; });

    if (search_result != constant_table_.end())
    {
        return clone(*std::get<0>(*search_result));
    }
    else
    {
        return nullptr;
    }
}

affine_expr affine_expr_context::create_literal(util::index_t value)
{
    return affine_expr(integer_literal(value), *this);
}

isl::space affine_expr_context::construct_corresponding_space(isl::context& isl_ctx) const
{
    isl::space corresponding_space(isl_ctx, constant_table_.size(), variable_table_.size());

    int pos = 0;

    for (const auto& entry : constant_table_)
    {
        corresponding_space.set_dim_name(isl_dim_param, pos, std::get<1>(entry));

        ++pos;
    }

    pos = 0;

    for (const auto& entry : variable_table_)
    {
        corresponding_space.set_dim_name(isl_dim_set, pos, std::get<1>(entry));

        ++pos;
    }

    return corresponding_space;
}

const std::function<bool(const expression&)>& affine_expr_context::get_invariant_checker() const
{
    return invariant_checker_;
}

affine_constraint::affine_constraint(std::unique_ptr<expression> expr_, affine_expr_context& ctx_)
: expr_(std::move(expr_)), ctx_(&ctx_)
{
}

affine_constraint::affine_constraint(const affine_constraint& other) : expr_(clone(*other.expr_)), ctx_(other.ctx_)
{
}

affine_constraint::affine_constraint(affine_constraint&& other) noexcept
        : expr_(std::move(other.expr_)), ctx_(other.ctx_)
{
}

affine_constraint& affine_constraint::operator=(const affine_constraint& other)
{
    this->expr_ = clone(*other.expr_);
    this->ctx_ = other.ctx_;

    return *this;
}

affine_constraint& affine_constraint::operator=(affine_constraint&& other) noexcept
{
    this->expr_ = std::move(other.expr_);
    this->ctx_ = other.ctx_;

    return *this;
}

isl::set affine_constraint::convert(isl::context& isl_ctx) const
{
    auto s = ctx_->construct_corresponding_space(isl_ctx);

    return convert_to_isl_set(*expr_, s);
}

affine_constraint operator&&(affine_constraint lhs, affine_constraint rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = logical_and(std::move(lhs.expr_), std::move(rhs.expr_));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint operator||(affine_constraint lhs, affine_constraint rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = logical_or(std::move(lhs.expr_), std::move(rhs.expr_));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint operator!(affine_constraint arg)
{
    auto new_expr = logical_not(std::move(arg.expr_));

    return affine_constraint(std::move(new_expr), *arg.ctx_);
}

affine_constraint equal_to(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = equal_to(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                             std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint not_equal_to(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = not_equal_to(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                                 std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint less(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = less(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                         std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint greater(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = greater(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                            std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint less_equal(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = less_equal(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                               std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

affine_constraint greater_equal(affine_expr lhs, affine_expr rhs)
{
    QUBUS_ASSERT(lhs.ctx_ == rhs.ctx_, "The context of lhs and rhs should be identical.");

    auto new_expr = greater_equal(std::make_unique<affine_expr_wrapper>(std::move(lhs)),
                                  std::make_unique<affine_expr_wrapper>(std::move(rhs)));

    return affine_constraint(std::move(new_expr), *lhs.ctx_);
}

boost::optional<affine_expr> try_construct_affine_expr(const expression& expr,
                                                       affine_expr_context& ctx)
{
    using pattern::_;

    pattern::variable<unary_op_tag> utag;
    pattern::variable<binary_op_tag> btag;
    pattern::variable<util::index_t> ival;
    pattern::variable<variable_declaration> variable;
    pattern::variable<const expression &> lhs, rhs, arg;

    auto m = pattern::make_matcher<expression, boost::optional<affine_expr>>()
                 .case_(binary_operator(btag, lhs, rhs),
                        [&]() -> boost::optional<affine_expr> {
                            auto lhs_expr = try_construct_affine_expr(lhs.get(), ctx);
                            auto rhs_expr = try_construct_affine_expr(rhs.get(), ctx);

                            if (!lhs_expr || !rhs_expr)
                                return boost::none;

                            switch (btag.get())
                            {
                            case binary_op_tag::plus:
                                return *std::move(lhs_expr) + *std::move(rhs_expr);
                            case binary_op_tag::minus:
                                return *std::move(lhs_expr) - *std::move(rhs_expr);
                            case binary_op_tag::multiplies:
                            {
                                if (!is_const(*lhs_expr) && !is_const(*rhs_expr))
                                    return boost::none;

                                return *std::move(lhs_expr) * *std::move(rhs_expr);
                            }
                            case binary_op_tag::divides:
                            {
                                if (!is_const(*rhs_expr))
                                    return boost::none;

                                return *std::move(lhs_expr) / *std::move(rhs_expr);
                            }
                            case binary_op_tag::modulus:
                            {
                                if (!is_const(*rhs_expr))
                                    return boost::none;

                                return *std::move(lhs_expr) % *std::move(rhs_expr);
                            }
                            default:
                                return boost::none;
                            }
                        })
                 .case_(unary_operator(utag, arg),
                        [&]() -> boost::optional<affine_expr> {
                            auto arg_expr = try_construct_affine_expr(arg.get(), ctx);

                            if (!arg_expr)
                                return boost::none;

                            switch (utag.get())
                            {
                            case unary_op_tag::plus:
                                return *std::move(arg_expr);
                            case unary_op_tag::negate:
                                return -*std::move(arg_expr);
                            default:
                                return boost::none;
                            }
                        })
                 .case_(integer_literal(ival), [&] { return ctx.create_literal(ival.get()); })
                 .case_(var(variable),
                        [&](const expression& self) -> boost::optional<affine_expr> {
                            // TODO: Generalize this for arbitrary integer types.
                            if (variable.get().var_type() != types::integer{})
                                return boost::none;

                            if (!ctx.get_invariant_checker()(self))
                            {
                                return ctx.declare_variable(variable.get());
                            }
                            else
                            {
                                return ctx.define_constant(self);
                            }
                        })
                 .case_(_, [&](const expression& self) -> boost::optional<affine_expr> {
                     if (ctx.get_invariant_checker()(self))
                     {
                         return ctx.define_constant(self);
                     }
                     else
                     {
                         return boost::none;
                     }
                 });

    return pattern::match(expr, m);
}

boost::optional<affine_constraint> try_extract_affine_constraint(const expression& expr,
                                                                 affine_expr_context& ctx)
{
    using pattern::_;

    pattern::variable<const expression &> lhs, rhs, arg;

    auto m =
        pattern::make_matcher<expression, boost::optional<affine_constraint>>()
            .case_(equal_to(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return equal_to(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(not_equal_to(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return not_equal_to(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(less(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return less(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(greater(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return greater(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(less_equal(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return less_equal(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(greater_equal(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_aff_expr = try_construct_affine_expr(lhs.get(), ctx);
                       auto rhs_aff_expr = try_construct_affine_expr(rhs.get(), ctx);

                       if (!lhs_aff_expr || !rhs_aff_expr)
                           return boost::none;

                       return greater_equal(*std::move(lhs_aff_expr), *std::move(rhs_aff_expr));
                   })
            .case_(logical_and(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_constraint = try_extract_affine_constraint(lhs.get(), ctx);
                       auto rhs_constraint = try_extract_affine_constraint(rhs.get(), ctx);

                       if (!lhs_constraint || !rhs_constraint)
                           return boost::none;

                       return *std::move(lhs_constraint) && *std::move(rhs_constraint);
                   })
            .case_(logical_or(lhs, rhs),
                   [&]() -> boost::optional<affine_constraint> {
                       auto lhs_constraint = try_extract_affine_constraint(lhs.get(), ctx);
                       auto rhs_constraint = try_extract_affine_constraint(rhs.get(), ctx);

                       if (!lhs_constraint || !rhs_constraint)
                           return boost::none;

                       return *std::move(lhs_constraint) || *std::move(rhs_constraint);
                   })
            .case_(logical_not(arg),
                   [&]() -> boost::optional<affine_constraint> {
                       auto arg_constraint = try_extract_affine_constraint(arg.get(), ctx);

                       if (!arg_constraint)
                           return boost::none;

                       return !*std::move(arg_constraint);
                   })
            .case_(_, [] { return boost::none; });

    return pattern::match(expr, m);
}

std::unique_ptr<expression> convert_isl_ast_expr_to_qir(const isl::ast_expr& expr,
                                                        const affine_expr_context& ctx)
{
    using pattern::_;

    switch (expr.type())
    {
    case isl_ast_expr_id:
    {
        auto name = expr.get_id().name();

        if (auto var = ctx.lookup_variable(name))
        {
            return variable_ref(*var);
        }
        else if (auto value = ctx.get_constant(name))
        {
            return value;
        }
        else
        {
            throw 0; // Invalid identifier
        }
    }
    case isl_ast_expr_int:
        return integer_literal(expr.get_value().as_integer());
    case isl_ast_expr_op:
        switch (expr.get_op_type())
        {
        case isl_ast_op_add:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) +
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_sub:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) -
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_mul:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) *
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_div:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) /
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_fdiv_q:
            return div_floor(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                             convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_pdiv_q:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) /
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_pdiv_r:
            return convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx) %
                   convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx);
        case isl_ast_op_minus:
            return -convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx);
        case isl_ast_op_min:
        {
            std::vector<std::unique_ptr<expression>> args;
            args.reserve(2);
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx));
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));

            return intrinsic_function("min", std::move(args));
        }
        case isl_ast_op_max:
        {
            std::vector<std::unique_ptr<expression>> args;
            args.reserve(2);
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx));
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));

            return intrinsic_function("max", std::move(args));
        }
        case isl_ast_op_cond:
        case isl_ast_op_select:
        {
            std::vector<std::unique_ptr<expression>> args;
            args.reserve(3);
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx));
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
            args.push_back(convert_isl_ast_expr_to_qir(expr.get_arg(2), ctx));

            return intrinsic_function("select", std::move(args));
        }
        case isl_ast_op_eq:
            return equal_to(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                            convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_le:
            return less_equal(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                              convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_lt:
            return less(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                        convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_ge:
            return greater_equal(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                                 convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_gt:
            return greater(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                           convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_and:
            return logical_and(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                               convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        case isl_ast_op_or:
            return logical_or(convert_isl_ast_expr_to_qir(expr.get_arg(0), ctx),
                              convert_isl_ast_expr_to_qir(expr.get_arg(1), ctx));
        default:
            QUBUS_UNREACHABLE_BECAUSE("Invalid operation type.");
        };
    default:
        QUBUS_UNREACHABLE_BECAUSE("Invalid expression type.");
    }
}

/*std::unique_ptr<expression> isl_ast_to_kir(const isl::ast_node& root, ast_converter_context& ctx)
{
    // static long int current_extracted_fn_id = 0;

    auto& symbol_table = ctx.symbol_table;

    switch (root.type())
    {
        case isl_ast_node_for:
        {
            // Assume that a for loop is of the form
            // for (int i = init; i </<= upper_bound; i += inc)

            ctx.array_subs_scopes.emplace_back();

            auto iterator = root.for_get_iterator();

            if (iterator.type() != isl_ast_expr_id)
                throw 0; // iterator has to be an id expr (might want to use an assert)

            // TODO: add new index to the lookup table
            variable_declaration idx_decl(types::integer{});

            symbol_table.emplace(iterator.get_id().name(), var(idx_decl));

            auto lower_bound = isl_ast_expr_to_kir(root.for_get_init(), ctx);
            auto increment = isl_ast_expr_to_kir(root.for_get_inc(), ctx);

            auto cond = root.for_get_cond();

            auto upper_bound = [&] {
                // Assume that the cond is of the form
                // i </<= expr
                // where i is the current iterator.

                // TODO: assert if cond is not an op

                switch (cond.get_op_type())
                {
                    case isl_ast_op_le:
                    {
                        auto rhs = isl_ast_expr_to_kir(cond.get_arg(1), ctx);

                        return std::unique_ptr<expression>(std::move(rhs) + integer_literal(1));
                    }
                    case isl_ast_op_lt:
                        return isl_ast_expr_to_kir(cond.get_arg(1), ctx);
                    default:
                        throw 0; // unexpected expr in cond
                }
            }();

            auto body = isl_ast_to_kir(root.for_get_body(), ctx);

            auto iter = std::remove_if(ctx.array_substitutions.begin(), ctx.array_substitutions.end(),
                                       [&](const array_substitution& value) {
                                           return std::any_of(ctx.array_subs_scopes.back().begin(),
                                                              ctx.array_subs_scopes.back().end(),
                                                              [&](const variable_declaration& decl) {
                                                                  return value.parent == decl;
                                                              });
                                       });

            ctx.array_substitutions.erase(iter, ctx.array_substitutions.end());

            ctx.array_subs_scopes.pop_back();

            symbol_table.erase(iterator.get_id().name());

            return for_(std::move(idx_decl), std::move(lower_bound), std::move(upper_bound),
                        std::move(increment), std::move(body));
        }
        case isl_ast_node_block:
        {
            std::vector<std::unique_ptr<expression>> sub_exprs;

            for (const auto& child : root.block_get_children())
            {
                sub_exprs.emplace_back(isl_ast_to_kir(child, ctx));
            }

            return sequenced_tasks(std::move(sub_exprs));
        }
        case isl_ast_node_user:
        {
            return isl_ast_expr_to_kir(root.user_get_expr(), ctx);
        }
        case isl_ast_node_if:
        {
            auto condition = isl_ast_expr_to_kir(root.if_get_cond(), ctx);

            auto then_branch = isl_ast_to_kir(root.if_get_then(), ctx);

            auto else_block = root.if_get_else();

            if (else_block)
            {
                auto else_branch = isl_ast_to_kir(*else_block, ctx);

                return if_(std::move(condition), std::move(then_branch), std::move(else_branch));
            }
            else
            {
                return if_(std::move(condition), std::move(then_branch));
            }
        }
        default:
            throw 0; // TODO: Unknown node type
    }
}*/
}