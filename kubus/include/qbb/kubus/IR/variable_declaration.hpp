#ifndef QBB_KUBUS_VARIABLE_DECLARATION_HPP
#define QBB_KUBUS_VARIABLE_DECLARATION_HPP

#include <qbb/kubus/IR/type.hpp>
#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/annotations.hpp>

#include <qbb/util/handle.hpp>

#include <memory>

namespace qbb
{
namespace kubus
{

class variable_declaration_info;

enum class variable_intent
{
    generic,
    in_param,
    out_param,
    inout_param
};

class variable_declaration
{
public:
    explicit variable_declaration(type var_type_, variable_intent intent_ = variable_intent::generic);
    
    const type& var_type() const;
    variable_intent intent() const;
    util::handle id() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::shared_ptr<variable_declaration_info> info_;
};

bool operator==(const variable_declaration& lhs, const variable_declaration& rhs);
bool operator!=(const variable_declaration& lhs, const variable_declaration& rhs);
 
}
}

#endif