#ifndef QBB_QUBUS_LITERAL_EXPR_HPP
#define QBB_QUBUS_LITERAL_EXPR_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/annotations.hpp>
#include <qbb/qubus/IR/expression_traits.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/unused.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{
 
class double_literal_expr
{
    
public:
    double_literal_expr() = default;
    explicit double_literal_expr(double value_);
    
    double value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & value_;
    }
private:
    double value_;
    
    mutable annotation_map annotations_;
};

bool operator==(const double_literal_expr& lhs, const double_literal_expr& rhs);
bool operator!=(const double_literal_expr& lhs, const double_literal_expr& rhs);

template<>
struct is_expression<double_literal_expr> : std::true_type
{
};

class float_literal_expr
{
    
public:
    float_literal_expr() = default;
    explicit float_literal_expr(float value_);
    
    float value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & value_;
    }
private:
    float value_;
    
    mutable annotation_map annotations_;
};

bool operator==(const float_literal_expr& lhs, const float_literal_expr& rhs);
bool operator!=(const float_literal_expr& lhs, const float_literal_expr& rhs);

template<>
struct is_expression<float_literal_expr> : std::true_type
{
};

class integer_literal_expr
{
    
public:
    integer_literal_expr() = default;
    explicit integer_literal_expr(qbb::util::index_t value_);
    
    qbb::util::index_t value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & value_;
    }
private:
    qbb::util::index_t value_;
    
    mutable annotation_map annotations_;
};

bool operator==(const integer_literal_expr& lhs, const integer_literal_expr& rhs);
bool operator!=(const integer_literal_expr& lhs, const integer_literal_expr& rhs);

template<>
struct is_expression<integer_literal_expr> : std::true_type
{
};

}
}

#endif