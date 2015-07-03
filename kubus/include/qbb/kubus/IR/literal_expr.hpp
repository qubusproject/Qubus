#ifndef QBB_KUBUS_LITERAL_EXPR_HPP
#define QBB_KUBUS_LITERAL_EXPR_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/annotations.hpp>
#include <qbb/kubus/IR/expression_traits.hpp>

#include <qbb/util/integers.hpp>

#include <vector>

namespace qbb
{
namespace qubus
{
 
class double_literal_expr
{
    
public:
    explicit double_literal_expr(double value_);
    
    double value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
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
    explicit float_literal_expr(float value_);
    
    float value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
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
    explicit integer_literal_expr(qbb::util::index_t value_);
    
    qbb::util::index_t value() const;
    
    std::vector<expression> sub_expressions() const;
    expression substitute_subexpressions(const std::vector<expression>& subexprs) const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
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