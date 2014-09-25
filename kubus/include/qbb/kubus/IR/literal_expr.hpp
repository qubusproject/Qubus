#ifndef QBB_KUBUS_LITERAL_EXPR_HPP
#define QBB_KUBUS_LITERAL_EXPR_HPP

#include <qbb/kubus/IR/annotations.hpp>

#include <qbb/util/integers.hpp>

namespace qbb
{
namespace kubus
{
 
class double_literal_expr
{
    
public:
    explicit double_literal_expr(double value_);
    
    double value() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    double value_;
    
    mutable annotation_map annotations_;
};

class float_literal_expr
{
    
public:
    explicit float_literal_expr(float value_);
    
    float value() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    float value_;
    
    mutable annotation_map annotations_;
};

class integer_literal_expr
{
    
public:
    explicit integer_literal_expr(qbb::util::index_t value_);
    
    qbb::util::index_t value() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    qbb::util::index_t value_;
    
    mutable annotation_map annotations_;
};
    
}
}

#endif