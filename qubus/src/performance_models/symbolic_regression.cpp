#include <qbb/qubus/performance_models/symbolic_regression.hpp>

#include <Eigen/Core>
#include <unsupported/Eigen/NonLinearOptimization>

#include <boost/circular_buffer.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>

inline namespace qbb
{
namespace qubus
{

namespace
{

#ifndef __GNUC__
class compiled_expression
{
public:
    static constexpr std::size_t stack_size = 40;

    using bytecode_unit_t = std::uint8_t;

    enum opcode : bytecode_unit_t
    {
        add,
        sub,
        mul,
        div,
        exp,
        log,
        push,
        push_param,
        push_arg,
        kronecker,
        quot_rule,
        prod_rule,
        halt
    };

    compiled_expression() = default;

    explicit compiled_expression(std::vector<bytecode_unit_t> bytecode_)
    : bytecode_(std::move(bytecode_))
    {
    }

    double operator()(const Eigen::VectorXd& parameters, const std::vector<double>& arguments,
                      long int diff_index) const
    {
        std::array<double, stack_size> stack;

        std::int64_t sp = 0;
        std::int64_t pc = 0;

        for (;;)
        {
            opcode op;
            static_assert(sizeof(opcode) == sizeof(bytecode_unit_t),
                          "Unexpected size of bytecode unit.");
            std::memcpy(&op, &bytecode_[pc++], sizeof(opcode));

            switch (op)
            {
            case opcode::push:
            {
                static_assert(sizeof(double) == 8 * sizeof(bytecode_unit_t),
                              "Unexpected size of bytecode unit.");

                double value;
                std::memcpy(&value, &bytecode_[pc], sizeof(double));
                pc += 8;

                stack[sp++] = value;
                break;
            }
            case opcode::push_param:
            {
                static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                              "Unexpected size of bytecode unit.");

                long int index;
                std::memcpy(&index, &bytecode_[pc], sizeof(long int));
                pc += 8;

                stack[sp++] = parameters(index);
                break;
            }
            case opcode::push_arg:
            {
                static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                              "Unexpected size of bytecode unit.");

                long int index;
                std::memcpy(&index, &bytecode_[pc], sizeof(long int));
                pc += 8;

                stack[sp++] = arguments[index];
                break;
            }
            case opcode::add:
                stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
                --sp;
                break;
            case opcode::sub:
                stack[sp - 2] = stack[sp - 2] - stack[sp - 1];
                --sp;
                break;
            case opcode::mul:
                stack[sp - 2] = stack[sp - 2] * stack[sp - 1];
                --sp;
                break;
            case opcode::div:
                stack[sp - 2] = stack[sp - 2] / stack[sp - 1];
                --sp;
                break;
            case opcode::exp:
                stack[sp - 1] = std::exp(stack[sp - 1]);
                break;
            case opcode::log:
                stack[sp - 1] = std::log(stack[sp - 1]);
                break;
            case opcode::kronecker:
            {
                static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                              "Unexpected size of bytecode unit.");

                long int index;
                std::memcpy(&index, &bytecode_[pc], sizeof(long int));
                pc += 8;

                stack[sp++] = index == diff_index ? 1.0 : 0.0;
                break;
            }
            case opcode::quot_rule:
            {
                auto lhs = stack[sp - 4];
                auto rhs = stack[sp - 3];

                auto lhs_diff = stack[sp - 2];
                auto rhs_diff = stack[sp - 1];

                stack[sp - 4] = (rhs * lhs_diff - lhs * rhs_diff) / (rhs * rhs);

                sp -= 3;
                break;
            }
            case opcode::prod_rule:
            {
                auto lhs = stack[sp - 4];
                auto rhs = stack[sp - 3];

                auto lhs_diff = stack[sp - 2];
                auto rhs_diff = stack[sp - 1];

                stack[sp - 4] = lhs * rhs_diff + lhs_diff * rhs;

                sp -= 3;
                break;
            }
            case opcode::halt:
                return stack[sp - 1];
            default:
                QBB_UNREACHABLE_BECAUSE("Invalid opcode.");
            }
        }
    }

private:
    std::vector<bytecode_unit_t> bytecode_;
};
#else
class compiled_expression
{
public:
    static constexpr std::size_t stack_size = 40;

    using bytecode_unit_t = std::uint8_t;

    enum opcode : bytecode_unit_t
    {
        add,
        sub,
        mul,
        div,
        exp,
        log,
        push,
        push_param,
        push_arg,
        kronecker,
        quot_rule,
        prod_rule,
        halt
    };

    compiled_expression() = default;

    explicit compiled_expression(std::vector<bytecode_unit_t> bytecode_)
    : bytecode_(std::move(bytecode_))
    {
    }

    double operator()(const Eigen::VectorXd& parameters, const std::vector<double>& arguments,
                      long int diff_index) const
    {
        static constexpr void* dispatch_table[] = {
            &&do_add,       &&do_sub,       &&do_mul,        &&do_div,      &&do_exp,
            &&do_log,       &&do_push,      &&do_push_param, &&do_push_arg, &&do_kronecker,
            &&do_quot_rule, &&do_prod_rule, &&do_halt};

        std::array<double, stack_size> stack;

        std::int64_t sp = 0;
        std::int64_t pc = 0;

        goto* dispatch_table[next_opcode(pc)];

        {
        do_push:
        {
            static_assert(sizeof(double) == 8 * sizeof(bytecode_unit_t),
                          "Unexpected size of bytecode unit.");

            double value;
            std::memcpy(&value, &bytecode_[pc], sizeof(double));
            pc += 8;

            stack[sp++] = value;
            goto* dispatch_table[next_opcode(pc)];
        }
        do_push_param:
        {
            static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                          "Unexpected size of bytecode unit.");

            long int index;
            std::memcpy(&index, &bytecode_[pc], sizeof(long int));
            pc += 8;

            stack[sp++] = parameters(index);
            goto* dispatch_table[next_opcode(pc)];
        }
        do_push_arg:
        {
            static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                          "Unexpected size of bytecode unit.");

            long int index;
            std::memcpy(&index, &bytecode_[pc], sizeof(long int));
            pc += 8;

            stack[sp++] = arguments[index];
            goto* dispatch_table[next_opcode(pc)];
        }
        do_add:
            stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
            --sp;
            goto* dispatch_table[next_opcode(pc)];
        do_sub:
            stack[sp - 2] = stack[sp - 2] - stack[sp - 1];
            --sp;
            goto* dispatch_table[next_opcode(pc)];
        do_mul:
            stack[sp - 2] = stack[sp - 2] * stack[sp - 1];
            --sp;
            goto* dispatch_table[next_opcode(pc)];
        do_div:
            stack[sp - 2] = stack[sp - 2] / stack[sp - 1];
            --sp;
            goto* dispatch_table[next_opcode(pc)];
        do_exp:
            stack[sp - 1] = std::exp(stack[sp - 1]);
            goto* dispatch_table[next_opcode(pc)];
        do_log:
            stack[sp - 1] = std::log(stack[sp - 1]);
            goto* dispatch_table[next_opcode(pc)];
        do_kronecker:
        {
            static_assert(sizeof(long int) == 8 * sizeof(bytecode_unit_t),
                          "Unexpected size of bytecode unit.");

            long int index;
            std::memcpy(&index, &bytecode_[pc], sizeof(long int));
            pc += 8;

            stack[sp++] = index == diff_index ? 1.0 : 0.0;
            goto* dispatch_table[next_opcode(pc)];
        }
        do_quot_rule:
        {
            auto lhs = stack[sp - 4];
            auto rhs = stack[sp - 3];

            auto lhs_diff = stack[sp - 2];
            auto rhs_diff = stack[sp - 1];

            stack[sp - 4] = (rhs * lhs_diff - lhs * rhs_diff) / (rhs * rhs);

            sp -= 3;
            goto* dispatch_table[next_opcode(pc)];
        }
        do_prod_rule:
        {
            auto lhs = stack[sp - 4];
            auto rhs = stack[sp - 3];

            auto lhs_diff = stack[sp - 2];
            auto rhs_diff = stack[sp - 1];

            stack[sp - 4] = lhs * rhs_diff + lhs_diff * rhs;

            sp -= 3;
            goto* dispatch_table[next_opcode(pc)];
        }
        do_halt:
            return stack[sp - 1];
        }
    }

private:
    opcode next_opcode(std::int64_t& pc) const
    {
        opcode op;
        static_assert(sizeof(opcode) == sizeof(bytecode_unit_t),
                      "Unexpected size of bytecode unit.");
        std::memcpy(&op, &bytecode_[pc++], sizeof(opcode));

        return op;
    }

    std::vector<bytecode_unit_t> bytecode_;
};
#endif

class model_expression
{
public:
    model_expression() = default;
    virtual ~model_expression() = default;

    model_expression(const model_expression&) = delete;
    model_expression& operator=(const model_expression&) = delete;

    model_expression(model_expression&&) = delete;
    model_expression& operator=(model_expression&&) = delete;

    virtual long int arity() const = 0;
    virtual const model_expression& child(long int index) const = 0;
    virtual model_expression& child(long int index) = 0;

    auto children() const
    {
        return boost::irange<long int>(0, this->arity()) |
               boost::adaptors::transformed(
                   [this](long int index) -> decltype(auto) { return this->child(index); });
    }

    auto children()
    {
        return boost::irange<long int>(0, this->arity()) |
               boost::adaptors::transformed(
                   [this](long int index) -> decltype(auto) { return this->child(index); });
    }

    model_expression* parent() const
    {
        return parent_;
    }

    void set_parent(model_expression& parent)
    {
        parent_ = &parent;
    }

    virtual std::unique_ptr<model_expression> clone() const = 0;

    virtual void substitute_child(const model_expression& old_child,
                                  std::unique_ptr<model_expression> new_child) = 0;

    virtual double evaluate(const Eigen::VectorXd& parameters,
                            const std::vector<double>& arguments) const = 0;

    virtual double df(const Eigen::VectorXd& parameters, const std::vector<double>& arguments,
                      long int index) const = 0;

    virtual void
    emit_evaluate_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const = 0;

    virtual void
    emit_df_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const = 0;

    virtual std::string dump(const Eigen::VectorXd& parameters) const = 0;

private:
    model_expression* parent_ = nullptr;
};

std::unique_ptr<model_expression> clone(const model_expression& expr)
{
    return expr.clone();
}

long int determine_number_of_expressions(const model_expression& root)
{
    long int number_of_expressions = 1;

    for (const auto& child : root.children())
    {
        number_of_expressions += determine_number_of_expressions(child);
    }

    QBB_ASSERT(number_of_expressions >= 0, "The number of expressions should be non-negative.");

    return number_of_expressions;
}

long int determine_number_of_parameters(const model_expression& root);

long int determine_depth(const model_expression& root)
{
    if (root.arity() == 0)
        return 1;

    long int depth = 1;

    for (const auto& child : root.children())
    {
        auto depth_of_child = determine_depth(child);

        depth = std::max(depth, depth_of_child);
    }

    return depth + 1;
}

class binary_operator_expression final : public model_expression
{
public:
    enum class tag
    {
        plus,
        minus,
        multiplies,
        divides
    };

    explicit binary_operator_expression(tag tag_, std::unique_ptr<model_expression> lhs_,
                                        std::unique_ptr<model_expression> rhs_)
    : tag_(tag_), lhs_(std::move(lhs_)), rhs_(std::move(rhs_))
    {
        this->lhs_->set_parent(*this);
        this->rhs_->set_parent(*this);
    }

    long int arity() const override
    {
        return 2;
    }

    const model_expression& child(long int index) const override
    {
        QBB_ASSERT(0 <= index && index < 2, "Invalid child index.");

        switch (index)
        {
        case 0:
            return *lhs_;
        case 1:
            return *rhs_;
        default:
            QBB_UNREACHABLE_BECAUSE("Invalid child index.");
        }
    }

    model_expression& child(long int index) override
    {
        QBB_ASSERT(0 <= index && index < 2, "Invalid child index.");

        switch (index)
        {
        case 0:
            return *lhs_;
        case 1:
            return *rhs_;
        default:
            QBB_UNREACHABLE_BECAUSE("Invalid child index.");
        }
    }

    std::unique_ptr<model_expression> clone() const override
    {
        return std::make_unique<binary_operator_expression>(tag_, lhs_->clone(), rhs_->clone());
    }

    void substitute_child(const model_expression& old_child,
                          std::unique_ptr<model_expression> new_child) override
    {
        if (&old_child == lhs_.get())
        {
            lhs_ = std::move(new_child);
            lhs_->set_parent(*this);
        }
        else if (&old_child == rhs_.get())
        {
            rhs_ = std::move(new_child);
            rhs_->set_parent(*this);
        }
        else
        {
            QBB_UNREACHABLE_BECAUSE("old_child is not a child of this node.");
        }
    }

    double evaluate(const Eigen::VectorXd& parameters,
                    const std::vector<double>& arguments) const override
    {
        switch (tag_)
        {
        case tag::plus:
            return lhs_->evaluate(parameters, arguments) + rhs_->evaluate(parameters, arguments);
        case tag::minus:
            return lhs_->evaluate(parameters, arguments) - rhs_->evaluate(parameters, arguments);
        case tag::multiplies:
            return lhs_->evaluate(parameters, arguments) * rhs_->evaluate(parameters, arguments);
        case tag::divides:
            return lhs_->evaluate(parameters, arguments) / rhs_->evaluate(parameters, arguments);
        }
    }

    double df(const Eigen::VectorXd& parameters, const std::vector<double>& arguments,
              long int index) const override
    {
        switch (tag_)
        {
        case tag::plus:
            return lhs_->df(parameters, arguments, index) + rhs_->df(parameters, arguments, index);
        case tag::minus:
            return lhs_->df(parameters, arguments, index) - rhs_->df(parameters, arguments, index);
        case tag::multiplies:
            return lhs_->df(parameters, arguments, index) * rhs_->evaluate(parameters, arguments) +
                   lhs_->evaluate(parameters, arguments) * rhs_->df(parameters, arguments, index);
        case tag::divides:
        {
            auto lhs_result = rhs_->evaluate(parameters, arguments);
            auto rhs_result = rhs_->evaluate(parameters, arguments);

            return (lhs_->df(parameters, arguments, index) * rhs_result -
                    lhs_result * rhs_->df(parameters, arguments, index)) /
                   (rhs_result * rhs_result);
        }
        }
    }

    void emit_evaluate_bytecode(
        std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        lhs_->emit_evaluate_bytecode(bytecode);
        rhs_->emit_evaluate_bytecode(bytecode);

        switch (tag_)
        {
        case tag::plus:
            bytecode.push_back(compiled_expression::opcode::add);
            return;
        case tag::minus:
            bytecode.push_back(compiled_expression::opcode::sub);
            return;
        case tag::multiplies:
            bytecode.push_back(compiled_expression::opcode::mul);
            return;
        case tag::divides:
            bytecode.push_back(compiled_expression::opcode::div);
            return;
        }
    }

    void
    emit_df_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        switch (tag_)
        {
        case tag::plus:
            lhs_->emit_df_bytecode(bytecode);
            rhs_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::add);
            break;
        case tag::minus:
            lhs_->emit_df_bytecode(bytecode);
            rhs_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::sub);
            break;
        case tag::multiplies:
            lhs_->emit_evaluate_bytecode(bytecode);
            rhs_->emit_evaluate_bytecode(bytecode);

            lhs_->emit_df_bytecode(bytecode);
            rhs_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::prod_rule);
            break;
        case tag::divides:
            lhs_->emit_evaluate_bytecode(bytecode);
            rhs_->emit_evaluate_bytecode(bytecode);

            lhs_->emit_df_bytecode(bytecode);
            rhs_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::quot_rule);

            break;
        }
    }

    std::string dump(const Eigen::VectorXd& parameters) const override
    {
        switch (tag_)
        {
        case tag::plus:
            return "(" + lhs_->dump(parameters) + " + " + rhs_->dump(parameters) + ")";
        case tag::minus:
            return "(" + lhs_->dump(parameters) + " - " + rhs_->dump(parameters) + ")";
        case tag::multiplies:
            return "(" + lhs_->dump(parameters) + " * " + rhs_->dump(parameters) + ")";
        case tag::divides:
            return "(" + lhs_->dump(parameters) + " / " + rhs_->dump(parameters) + ")";
        }
    }

private:
    tag tag_;
    std::unique_ptr<model_expression> lhs_;
    std::unique_ptr<model_expression> rhs_;
};

class function_expression final : public model_expression
{
public:
    enum class tag
    {
        exp,
        log
    };

    explicit function_expression(tag tag_, std::unique_ptr<model_expression> arg_)
    : tag_(tag_), arg_(std::move(arg_))
    {
        this->arg_->set_parent(*this);
    }

    long int arity() const override
    {
        return 1;
    }

    const model_expression& child(long int index) const override
    {
        QBB_ASSERT(0 <= index && index < 1, "Invalid child index.");

        switch (index)
        {
        case 0:
            return *arg_;
        default:
            QBB_UNREACHABLE_BECAUSE("Invalid child index.");
        }
    }

    model_expression& child(long int index) override
    {
        QBB_ASSERT(0 <= index && index < 1, "Invalid child index.");

        switch (index)
        {
        case 0:
            return *arg_;
        default:
            QBB_UNREACHABLE_BECAUSE("Invalid child index.");
        }
    }

    std::unique_ptr<model_expression> clone() const override
    {
        return std::make_unique<function_expression>(tag_, arg_->clone());
    }

    void substitute_child(const model_expression& old_child,
                          std::unique_ptr<model_expression> new_child) override
    {
        if (&old_child == arg_.get())
        {
            arg_ = std::move(new_child);
            arg_->set_parent(*this);
        }
        else
        {
            QBB_UNREACHABLE_BECAUSE("old_child is not a child of this node.");
        }
    }

    double evaluate(const Eigen::VectorXd& parameters,
                    const std::vector<double>& arguments) const override
    {
        switch (tag_)
        {
        case tag::exp:
            return std::exp(arg_->evaluate(parameters, arguments));
        case tag::log:
            return std::log(arg_->evaluate(parameters, arguments));
        }
    }

    double df(const Eigen::VectorXd& parameters, const std::vector<double>& arguments,
              long int index) const override
    {
        switch (tag_)
        {
        case tag::exp:
            return std::exp(arg_->evaluate(parameters, arguments)) *
                   arg_->df(parameters, arguments, index);
        case tag::log:
            return arg_->df(parameters, arguments, index) / arg_->evaluate(parameters, arguments);
        }
    }

    void emit_evaluate_bytecode(
        std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        arg_->emit_evaluate_bytecode(bytecode);

        switch (tag_)
        {
        case tag::exp:
            bytecode.push_back(compiled_expression::opcode::exp);
            return;
        case tag::log:
            bytecode.push_back(compiled_expression::opcode::log);
            return;
        }
    }

    void
    emit_df_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        switch (tag_)
        {
        case tag::exp:
            arg_->emit_evaluate_bytecode(bytecode);
            bytecode.push_back(compiled_expression::opcode::exp);

            arg_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::mul);
            return;
        case tag::log:
            arg_->emit_df_bytecode(bytecode);
            arg_->emit_evaluate_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::div);
            return;
        }
    }

    std::string dump(const Eigen::VectorXd& parameters) const override
    {
        switch (tag_)
        {
        case tag::exp:
            return "exp(" + arg_->dump(parameters) + ")";
        case tag::log:
            return "log(" + arg_->dump(parameters) + ")";
        }
    }

private:
    tag tag_;
    std::unique_ptr<model_expression> arg_;
};

class parameter_expression final : public model_expression
{
public:
    explicit parameter_expression(long int index_) : index_(std::move(index_))
    {
        QBB_ASSERT(this->index_ >= 0, "Invalid index.");
    }

    long int arity() const override
    {
        return 0;
    }

    const model_expression& child(long int index) const override
    {
        QBB_UNREACHABLE_BECAUSE("Invalid child index.");
    }

    model_expression& child(long int index) override
    {
        QBB_UNREACHABLE_BECAUSE("Invalid child index.");
    }

    std::unique_ptr<model_expression> clone() const override
    {
        return std::make_unique<parameter_expression>(index_);
    }

    void substitute_child(const model_expression& QBB_UNUSED(old_child),
                          std::unique_ptr<model_expression> QBB_UNUSED(new_child)) override
    {
        QBB_UNREACHABLE_BECAUSE("old_child is not a child of this node.");
    }

    long int index() const
    {
        return index_;
    }

    void set_index(long int index)
    {
        index_ = index;
    }

    double evaluate(const Eigen::VectorXd& parameters,
                    const std::vector<double>& QBB_UNUSED(arguments)) const override
    {
        QBB_ASSERT(index_ < parameters.size(), "Invalid index.");

        return parameters(util::integer_cast<Eigen::Index>(index_));
    }

    double df(const Eigen::VectorXd& QBB_UNUSED(parameters),
              const std::vector<double>& QBB_UNUSED(arguments), long int index) const override
    {
        return index_ == index ? 1.0 : 0.0;
    }

    void emit_evaluate_bytecode(
        std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        bytecode.push_back(compiled_expression::opcode::push_param);

        compiled_expression::bytecode_unit_t data[8];

        static_assert(sizeof(long int) == 8 * sizeof(compiled_expression::bytecode_unit_t),
                      "Unexpected size of bytecode unit.");
        std::memcpy(&data, &index_, sizeof(long int));

        for (std::size_t i = 0; i < 8; ++i)
        {
            bytecode.push_back(data[i]);
        }
    }

    void
    emit_df_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        bytecode.push_back(compiled_expression::opcode::kronecker);

        compiled_expression::bytecode_unit_t data[8];

        static_assert(sizeof(long int) == 8 * sizeof(compiled_expression::bytecode_unit_t),
                      "Unexpected size of bytecode unit.");
        std::memcpy(&data, &index_, sizeof(long int));

        for (std::size_t i = 0; i < 8; ++i)
        {
            bytecode.push_back(data[i]);
        }
    }

    std::string dump(const Eigen::VectorXd& parameters) const override
    {
        QBB_ASSERT(index_ < parameters.size(), "Invalid index.");

        return std::to_string(parameters(util::integer_cast<Eigen::Index>(index_)));
    }

private:
    long int index_;
};

class variable_expression final : public model_expression
{
public:
    explicit variable_expression(long int index_) : index_(std::move(index_))
    {
        QBB_ASSERT(this->index_ >= 0, "Invalid index.");
    }

    long int arity() const override
    {
        return 0;
    }

    const model_expression& child(long int index) const override
    {
        QBB_UNREACHABLE_BECAUSE("Invalid child index.");
    }

    model_expression& child(long int index) override
    {
        QBB_UNREACHABLE_BECAUSE("Invalid child index.");
    }

    std::unique_ptr<model_expression> clone() const override
    {
        return std::make_unique<variable_expression>(index_);
    }

    void substitute_child(const model_expression& QBB_UNUSED(old_child),
                          std::unique_ptr<model_expression> QBB_UNUSED(new_child)) override
    {
        QBB_UNREACHABLE_BECAUSE("old_child is not a child of this node.");
    }

    double evaluate(const Eigen::VectorXd& QBB_UNUSED(parameters),
                    const std::vector<double>& arguments) const override
    {
        QBB_ASSERT(index_ < arguments.size(), "Invalid index.");

        return arguments[util::integer_cast<std::size_t>(index_)];
    }

    double df(const Eigen::VectorXd& QBB_UNUSED(parameters),
              const std::vector<double>& QBB_UNUSED(arguments),
              long int QBB_UNUSED(index)) const override
    {
        return 0.0;
    }

    void emit_evaluate_bytecode(
        std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        bytecode.push_back(compiled_expression::opcode::push_arg);

        compiled_expression::bytecode_unit_t data[8];

        static_assert(sizeof(long int) == 8 * sizeof(compiled_expression::bytecode_unit_t),
                      "Unexpected size of bytecode unit.");
        std::memcpy(&data, &index_, sizeof(long int));

        for (std::size_t i = 0; i < 8; ++i)
        {
            bytecode.push_back(data[i]);
        }
    }

    void
    emit_df_bytecode(std::vector<compiled_expression::bytecode_unit_t>& bytecode) const override
    {
        bytecode.push_back(compiled_expression::opcode::push);

        compiled_expression::bytecode_unit_t data[8];
        const double value = 0.0;

        static_assert(sizeof(double) == 8 * sizeof(compiled_expression::bytecode_unit_t),
                      "Unexpected size of bytecode unit.");
        std::memcpy(&data, &index_, sizeof(double));

        for (std::size_t i = 0; i < 8; ++i)
        {
            bytecode.push_back(data[i]);
        }
    }

    std::string dump(const Eigen::VectorXd& QBB_UNUSED(parameters)) const override
    {
        return "a" + std::to_string(index_);
    }

private:
    long int index_;
};

long int determine_number_of_parameters(const model_expression& root)
{
    long int number_of_parameters = 0;

    if (auto parameter_expr = dynamic_cast<const parameter_expression*>(&root))
    {
        number_of_parameters = parameter_expr->index() + 1;
    }

    for (const auto& child : root.children())
    {
        number_of_parameters =
            std::max(number_of_parameters, determine_number_of_parameters(child));
    }

    QBB_ASSERT(number_of_parameters >= 0, "The number of parameters has to be non-negative.");

    return number_of_parameters;
}

template <typename Engine>
std::unique_ptr<model_expression>
generate_random_expression(Engine& engine, long int number_of_arguments,
                           long int& number_of_parameters, long int depth = 0)
{
    QBB_ASSERT(number_of_arguments > 0, "The expression has to contain at least one argument.");

    enum expression_type
    {
        parameter,
        variable,
        plus,
        minus,
        multiplies,
        divides,
        exp,
        log,
        number_of_types
    };

    std::discrete_distribution<int> type_dist({
        10, // parameter
        10, // variable
        10, // plus
        10, // minus
        10, // multiplies
        10, // divides
        2,  // exp
        2   // log
    });

    std::uniform_int_distribution<long int> parameter_index_dist(0, 2 * number_of_parameters);
    std::uniform_int_distribution<long int> argument_index_dist(0, number_of_arguments - 1);

    auto r = type_dist(engine);

    if (depth > 2)
        r = r % 2;

    expression_type type = static_cast<expression_type>(r);

    switch (type)
    {
    case parameter:
    {
        long int index = parameter_index_dist(engine);

        if (index < number_of_parameters)
        {
            return std::make_unique<parameter_expression>(index);
        }
        else
        {
            long int new_parameter = number_of_parameters;

            ++number_of_parameters;

            return std::make_unique<parameter_expression>(new_parameter);
        }
    }
    case variable:
    {
        long int index = argument_index_dist(engine);

        return std::make_unique<variable_expression>(index);
    }
    case plus:
    {
        auto lhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);
        auto rhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<binary_operator_expression>(binary_operator_expression::tag::plus,
                                                            std::move(lhs), std::move(rhs));
    }
    case minus:
    {
        auto lhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);
        auto rhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<binary_operator_expression>(binary_operator_expression::tag::minus,
                                                            std::move(lhs), std::move(rhs));
    }
    case multiplies:
    {
        auto lhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);
        auto rhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<binary_operator_expression>(
            binary_operator_expression::tag::multiplies, std::move(lhs), std::move(rhs));
    }
    case divides:
    {
        auto lhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);
        auto rhs = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<binary_operator_expression>(
            binary_operator_expression::tag::divides, std::move(lhs), std::move(rhs));
    }
    case exp:
    {
        auto arg = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<function_expression>(function_expression::tag::exp, std::move(arg));
    }
    case log:
    {
        auto arg = generate_random_expression(engine, number_of_arguments, number_of_parameters,
                                              depth + 1);

        return std::make_unique<function_expression>(function_expression::tag::log, std::move(arg));
    }
    case number_of_types:
        QBB_UNREACHABLE_BECAUSE("number_of_types is not an actual expression type.");
    }

    QBB_UNREACHABLE();
}

template <typename Engine>
const model_expression& select_random_node(const model_expression& expr, Engine& engine)
{
    auto number_of_nodes = determine_number_of_expressions(expr);

    std::uniform_int_distribution<long int> dist(0, number_of_nodes - 1);

    auto number_of_steps = dist(engine);

    std::stack<const model_expression*> node_stack;

    node_stack.push(&expr);

    while (!node_stack.empty())
    {
        auto current_node = node_stack.top();
        node_stack.pop();

        if (number_of_steps == 0)
            return *current_node;

        --number_of_steps;

        for (const auto& child : current_node->children())
        {
            node_stack.push(&child);
        }
    }

    QBB_UNREACHABLE_BECAUSE("The algorithm should have reached the selected node long ago.");
}

void remove_dead_parameters(model_expression& expr)
{
    std::unordered_map<long int, long int> index_map;

    std::stack<model_expression*> node_stack;

    node_stack.push(&expr);

    while (!node_stack.empty())
    {
        auto& current_node = *node_stack.top();
        node_stack.pop();

        if (auto typed_node = dynamic_cast<parameter_expression*>(&current_node))
        {
            auto old_index = typed_node->index();

            auto search_result = index_map.find(old_index);

            if (search_result != index_map.end())
            {
                typed_node->set_index(search_result->second);
            }
            else
            {
                auto new_index = util::integer_cast<long int>(index_map.size());

                typed_node->set_index(new_index);

                index_map.emplace(old_index, new_index);
            }
        }

        for (auto& child : current_node.children())
        {
            node_stack.push(&child);
        }
    }
}

template <typename Engine>
std::unique_ptr<model_expression> mutate_expression(const model_expression& expr,
                                                    long int number_of_arguments,
                                                    long int number_of_parameters, Engine& engine)
{
    auto cloned_expression = clone(expr);

    const auto& random_node = select_random_node(*cloned_expression, engine);

    auto new_sub_tree =
        generate_random_expression(engine, number_of_arguments, number_of_parameters);

    auto parent = random_node.parent();

    if (!parent)
        return new_sub_tree;

    parent->substitute_child(random_node, std::move(new_sub_tree));

    remove_dead_parameters(*cloned_expression);

    return cloned_expression;
}

template <typename Engine>
std::unique_ptr<model_expression> mix_expressions(const model_expression& mother,
                                                  const model_expression& father, Engine& engine)
{
    auto cloned_expression = clone(mother);

    const auto& random_node = select_random_node(*cloned_expression, engine);

    auto parent = random_node.parent();

    if (!parent)
        return clone(father);

    const auto& random_node2 = select_random_node(father, engine);

    parent->substitute_child(random_node, clone(random_node2));

    remove_dead_parameters(*cloned_expression);

    return cloned_expression;
}

template <typename Engine>
std::unique_ptr<model_expression> wrap_expression(const model_expression& expr, Engine& engine)
{
    std::uniform_int_distribution<long int> dist(0, 1);

    auto tag = dist(engine);

    switch (tag)
    {
    case 0:
        return std::make_unique<function_expression>(function_expression::tag::exp, clone(expr));
    case 1:
        return std::make_unique<function_expression>(function_expression::tag::log, clone(expr));
    default:
        QBB_UNREACHABLE_BECAUSE("No case left.");
    }
}

template <typename Engine>
std::unique_ptr<model_expression>
combine_expressions(const model_expression& mother, const model_expression& father, Engine& engine)
{
    std::uniform_int_distribution<long int> dist(0, 3);

    auto tag = dist(engine);

    switch (tag)
    {
    case 0:
        return std::make_unique<binary_operator_expression>(binary_operator_expression::tag::plus,
                                                            clone(mother), clone(father));
    case 1:
        return std::make_unique<binary_operator_expression>(binary_operator_expression::tag::minus,
                                                            clone(mother), clone(father));
    case 2:
        return std::make_unique<binary_operator_expression>(
            binary_operator_expression::tag::multiplies, clone(mother), clone(father));
    case 3:
        return std::make_unique<binary_operator_expression>(
            binary_operator_expression::tag::divides, clone(mother), clone(father));
    default:
        QBB_UNREACHABLE_BECAUSE("No case left.");
    }
}

struct data_point
{
    std::vector<double> arguments;
    std::chrono::microseconds execution_time;
};

template <typename Dataset>
class model
{
public:
    static constexpr long int max_depth = 20;

    explicit model(std::unique_ptr<model_expression> root_, const Dataset& dataset_)
    : root_(std::move(root_)),
      number_of_parameters_(determine_number_of_parameters(*this->root_)),
      dataset_(&dataset_)
    {
        QBB_ASSERT(determine_depth(*this->root_) <= max_depth,
                   "The depth is larger than expected.");

        static_assert(max_depth < compiled_expression::stack_size,
                      "The size of the stack might not be sufficient.");

        {
            std::vector<compiled_expression::bytecode_unit_t> bytecode;

            this->root_->emit_evaluate_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::halt);

            evaluate_ = compiled_expression(std::move(bytecode));
        }

        {
            std::vector<compiled_expression::bytecode_unit_t> bytecode;

            this->root_->emit_df_bytecode(bytecode);

            bytecode.push_back(compiled_expression::opcode::halt);

            df_ = compiled_expression(std::move(bytecode));
        }
    }

    model(const model& other)
    : root_(clone(*other.root_)),
      number_of_parameters_(other.number_of_parameters_),
      dataset_(other.dataset_),
      evaluate_(other.evaluate_),
      df_(other.df_)
    {
    }

    model& operator=(const model& other)
    {
        root_ = clone(*other.root_);
        number_of_parameters_ = other.number_of_parameters_;
        dataset_ = other.dataset_;
        evaluate_ = other.evaluate_;
        df_ = other.df_;

        return *this;
    }

    model(model&& other) noexcept
    : root_(std::move(other.root_)),
      number_of_parameters_(std::move(other.number_of_parameters_)),
      dataset_(other.dataset_),
      evaluate_(std::move(other.evaluate_)),
      df_(std::move(other.df_))
    {
    }

    model& operator=(model&& other) noexcept
    {
        root_ = std::move(other.root_);
        number_of_parameters_ = std::move(other.number_of_parameters_);
        dataset_ = other.dataset_;
        evaluate_ = std::move(other.evaluate_);
        df_ = std::move(other.df_);

        return *this;
    }

    const model_expression& expr() const
    {
        QBB_ASSERT(root_, "Invalid object.");

        return *root_;
    }

    long int number_of_arguments() const
    {
        return dataset_->front().arguments.size();
    }

    long int number_of_parameters() const
    {
        return number_of_parameters_;
    }

    const Dataset& data_set() const
    {
        return *dataset_;
    }

    int values() const
    {
        return util::integer_cast<int>(dataset_->size());
    }

    std::chrono::microseconds query(const Eigen::VectorXd& parameters,
                                    const std::vector<double>& arguments) const
    {
        auto result = root_->evaluate(parameters, arguments);

        using rep = std::chrono::microseconds::rep;
        double max_value = std::numeric_limits<rep>::max() / 2;

        if (!std::isfinite(result) || result > max_value || result < 0)
        {
            result = max_value;
        }

        auto count = static_cast<rep>(result);

        QBB_ASSERT(count >= 0, "Invalid duration.");

        return std::chrono::microseconds(std::move(count));
    }

    int operator()(const Eigen::VectorXd& parameters, Eigen::VectorXd& result) const
    {
        QBB_ASSERT(number_of_parameters_ == parameters.size(), "Unexpected number of parameters.");

        for (long int i = 0; i < result.size(); ++i)
        {
            auto i_idx = util::integer_cast<Eigen::Index>(i);
            auto i_std = util::integer_cast<std::size_t>(i);

            result(i_idx) = evaluate_(parameters, (*dataset_)[i_std].arguments, 0) -
                            (*dataset_)[i_std].execution_time.count();
        }

        return 0;
    }

    int df(const Eigen::VectorXd& parameters, Eigen::MatrixXd& result) const
    {
        QBB_ASSERT(result.rows() == dataset_->size(), "Unexpected number of results.");
        QBB_ASSERT(result.cols() == parameters.size(), "Unexpected number of variables.");

        for (long int i = 0; i < result.rows(); ++i)
        {
            auto i_idx = util::integer_cast<Eigen::Index>(i);
            auto i_std = util::integer_cast<std::size_t>(i);

            for (long int col = 0; col < result.cols(); ++col)
            {
                auto col_idx = util::integer_cast<Eigen::Index>(col);

                result(i_idx, col_idx) = df_(parameters, (*dataset_)[i_std].arguments, col);
            }
        }

        return 0;
    }

    std::string dump(const Eigen::VectorXd& parameters) const
    {
        return root_->dump(parameters);
    }

private:
    std::unique_ptr<model_expression> root_;
    long int number_of_parameters_;
    const Dataset* dataset_;

    compiled_expression evaluate_;
    compiled_expression df_;
};

template <typename Dataset>
class regression_model
{
public:
    template <typename Engine>
    regression_model(model<Dataset> m, Engine& engine) : model_(m)
    {
        auto number_of_parameters = this->model_.number_of_parameters();

        parameters_.resize(number_of_parameters);

        std::uniform_real_distribution<double> parameter_dist(-100, 100);

        for (long int i = 0; i < number_of_parameters; ++i)
        {
            parameters_(util::integer_cast<Eigen::Index>(i)) = parameter_dist(engine);
        }

        update();
    }

    const model_expression& expr() const
    {
        return model_.expr();
    }

    long int number_of_arguments() const
    {
        return model_.number_of_arguments();
    }

    long int number_of_parameters() const
    {
        return model_.number_of_parameters();
    }

    std::chrono::microseconds accuracy() const
    {
        return accuracy_;
    }

    double fitness() const
    {
        return fitness_;
    }

    std::chrono::microseconds query(const std::vector<double>& arguments) const
    {
        return model_.query(parameters_, arguments);
    }

    void update()
    {
        update_parameters();
        update_accuarcy();
        update_fitness();
    }

private:
    void update_parameters()
    {
        auto number_of_parameters = this->model_.number_of_parameters();

        if (number_of_parameters > 0 && number_of_parameters < this->model_.data_set().size())
        {
            Eigen::LevenbergMarquardt<model<Dataset>> lm(this->model_);
            lm.minimize(parameters_);
        }
    }

    void update_accuarcy()
    {
        double norm = 0.0;

        for (const auto& point : model_.data_set())
        {
            norm +=
                std::abs(static_cast<double>(model_.query(parameters_, point.arguments).count()) -
                         static_cast<double>(point.execution_time.count()));
        }

        norm /= model_.data_set().size();

        QBB_ASSERT(norm >= 0, "A norm shall be non-negative.");

        double max_norm = std::numeric_limits<std::chrono::microseconds::rep>::max() / 2;

        if (!std::isfinite(norm))
        {
            norm = max_norm;
        }
        else
        {
            norm = std::min(norm, max_norm);
        }

        accuracy_ = std::chrono::microseconds(static_cast<std::chrono::microseconds::rep>(norm));

        QBB_ASSERT(accuracy_.count() >= 0, "An accuary shall be non-negative.");
    }

    void update_fitness()
    {
        fitness_ = static_cast<double>(accuracy_.count()) +
                   2 * determine_number_of_expressions(model_.expr());
    }

    model<Dataset> model_;
    Eigen::VectorXd parameters_;
    std::chrono::microseconds accuracy_;
    double fitness_;
};

template <typename Dataset, typename Engine>
std::vector<regression_model<Dataset>>
perform_genetic_programming_step(std::vector<regression_model<Dataset>> old_generation,
                                 long int number_of_arguments, const Dataset& dataset,
                                 Engine& engine)
{
    constexpr long int elite_share = 10;
    constexpr long int immigrant_share = 10;
    constexpr long int mutation_rate = 20;

    constexpr long int wrap_rate = 10;
    constexpr long int combination_rate = 10;

    constexpr long int max_depth = model<Dataset>::max_depth;

    constexpr long int crossover_rate = 100 - mutation_rate - wrap_rate - combination_rate;

    const auto fitness_comperator = [](const regression_model<Dataset>& lhs,
                                       const regression_model<Dataset>& rhs) {
        return lhs.fitness() < rhs.fitness();
    };

    QBB_ASSERT(std::is_sorted(old_generation.begin(), old_generation.end(), fitness_comperator),
               "The old generation has to be sorted.");

    const auto population_size = old_generation.size();

    std::discrete_distribution<int> dist(
        {mutation_rate, wrap_rate, combination_rate, crossover_rate});
    std::uniform_int_distribution<long int> population_index_dist(0, population_size - 1);

    std::vector<regression_model<Dataset>> new_generation;
    new_generation.reserve(population_size);

    auto number_of_elites = (population_size * elite_share) / 100;

    for (std::size_t i = 0; i < number_of_elites; ++i)
    {
        new_generation.push_back(old_generation[i]);
        new_generation.back().update();
    }

    auto number_of_immigrants = (population_size * immigrant_share) / 100;

    for (std::size_t i = number_of_elites; i < number_of_elites + number_of_immigrants; ++i)
    {
        long int number_of_parameters = 0;

        auto new_expr =
            generate_random_expression(engine, number_of_arguments, number_of_parameters);

        auto new_model = model<Dataset>(std::move(new_expr), dataset);

        new_generation.push_back(regression_model<Dataset>(std::move(new_model), engine));
    }

    for (std::size_t i = number_of_elites + number_of_immigrants; i < population_size; ++i)
    {
        auto action = dist(engine);

        switch (action)
        {
        case 0:
        {
            auto index = population_index_dist(engine);

            const auto& reg_model = old_generation[index];

            auto new_expr = mutate_expression(reg_model.expr(), reg_model.number_of_arguments(),
                                              reg_model.number_of_parameters(), engine);

            if (determine_depth(*new_expr) <= max_depth)
            {
                remove_dead_parameters(*new_expr);
            }
            else
            {
                new_expr = clone(reg_model.expr());
            }

            auto new_model = model<Dataset>(std::move(new_expr), dataset);

            new_generation.push_back(regression_model<Dataset>(std::move(new_model), engine));

            break;
        }
        case 1:
        {
            auto index = population_index_dist(engine);

            const auto& reg_model = old_generation[index];

            auto new_expr = wrap_expression(reg_model.expr(), engine);

            if (determine_depth(*new_expr) > max_depth)
            {
                new_expr = clone(reg_model.expr());
            }

            auto new_model = model<Dataset>(std::move(new_expr), dataset);

            new_generation.push_back(regression_model<Dataset>(std::move(new_model), engine));

            break;
        }
        case 2:
        {
            auto index = population_index_dist(engine);
            auto index2 = population_index_dist(engine);

            const auto& mother = old_generation[index];
            const auto& father = old_generation[index2];

            auto new_expr = combine_expressions(mother.expr(), father.expr(), engine);

            if (determine_depth(*new_expr) > max_depth)
            {
                new_expr = clone(mother.expr());
            }

            auto new_model = model<Dataset>(std::move(new_expr), dataset);

            new_generation.push_back(regression_model<Dataset>(std::move(new_model), engine));

            break;
        }
        case 3:
        {
            auto index = population_index_dist(engine);
            auto index2 = population_index_dist(engine);

            const auto& mother = old_generation[index];
            const auto& father = old_generation[index2];

            auto new_expr = mix_expressions(mother.expr(), father.expr(), engine);

            if (determine_depth(*new_expr) <= max_depth)
            {
                remove_dead_parameters(*new_expr);
            }
            else
            {
                new_expr = clone(mother.expr());
            }

            auto new_model = model<Dataset>(std::move(new_expr), dataset);

            new_generation.push_back(regression_model<Dataset>(std::move(new_model), engine));

            break;
        }
        default:
            QBB_UNREACHABLE_BECAUSE("No case left.");
        }
    }

    std::sort(new_generation.begin(), new_generation.end(), fitness_comperator);

    QBB_ASSERT(std::is_sorted(new_generation.begin(), new_generation.end(), fitness_comperator),
               "The new generation has to be sorted.");

    return new_generation;
}
}

class symbolic_regression_impl
{
public:
    explicit symbolic_regression_impl(long int window_size_)
    : dataset_(util::integer_cast<dataset_type::size_type>(window_size_))
    {
        std::random_device dev;
        engine_ = std::mt19937(dev());
    }

    void add_datapoint(std::vector<double> arguments, std::chrono::microseconds execution_time)
    {
        dataset_.push_back(data_point{std::move(arguments), std::move(execution_time)});
    }

    boost::optional<std::chrono::microseconds> update()
    {
        if (dataset_.empty())
            return accuracy();

        if (models_.empty())
        {
            constexpr long int size_of_population = 200;

            models_.reserve(size_of_population);

            for (long int i = 0; i < size_of_population; ++i)
            {
                long int number_of_parameters = 0;

                auto new_model = model<dataset_type>(
                    generate_random_expression(engine_, dataset_.front().arguments.size(),
                                               number_of_parameters),
                    dataset_);

                models_.push_back(regression_model<dataset_type>(std::move(new_model), engine_));
            }

            const auto fitness_comperator = [](const regression_model<dataset_type>& lhs,
                                               const regression_model<dataset_type>& rhs) {
                return lhs.fitness() < rhs.fitness();
            };

            std::sort(models_.begin(), models_.end(), fitness_comperator);
        }
        else
        {
            models_ = perform_genetic_programming_step(
                std::move(models_), dataset_.front().arguments.size(), dataset_, engine_);
        }

        return accuracy();
    }

    boost::optional<std::chrono::microseconds> update_cheaply()
    {
        models_.front().update();

        return accuracy();
    }

    long int size_of_dataset() const
    {
        return util::integer_cast<long int>(dataset_.size());
    }

    boost::optional<std::chrono::microseconds> query(const std::vector<double>& arguments) const
    {
        if (models_.empty())
            return boost::none;

        return models_.front().query(arguments);
    }

    boost::optional<std::chrono::microseconds> accuracy() const
    {
        if (models_.empty())
            return boost::none;

        return models_.front().accuracy();
    }

private:
    using dataset_type = boost:: circular_buffer_space_optimized<data_point>;

    std::mt19937 engine_;

    dataset_type dataset_;
    std::vector<regression_model<dataset_type>> models_;
};

symbolic_regression::symbolic_regression(long int window_size_)
: impl_(std::make_unique<symbolic_regression_impl>(window_size_))
{
}

symbolic_regression::~symbolic_regression() = default;

symbolic_regression::symbolic_regression(symbolic_regression&&) = default;
symbolic_regression& symbolic_regression::operator=(symbolic_regression&&) = default;

void symbolic_regression::add_datapoint(std::vector<double> arguments,
                                        std::chrono::microseconds execution_time)
{
    impl_->add_datapoint(std::move(arguments), std::move(execution_time));
}

boost::optional<std::chrono::microseconds> symbolic_regression::update()
{
    return impl_->update();
}

boost::optional<std::chrono::microseconds> symbolic_regression::update_cheaply()
{
    return impl_->update_cheaply();
}

long int symbolic_regression::size_of_dataset() const
{
    return impl_->size_of_dataset();
}

boost::optional<std::chrono::microseconds>
symbolic_regression::query(const std::vector<double>& arguments) const
{
    return impl_->query(arguments);
}

boost::optional<std::chrono::microseconds> symbolic_regression::accuracy() const
{
    return impl_->accuracy();
}
}
}