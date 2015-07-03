#ifndef QBB_KUBUS_FUNCTION_DECLARATION_HPP
#define QBB_KUBUS_FUNCTION_DECLARATION_HPP

#include <qbb/kubus/IR/type.hpp>
#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/annotations.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>

#include <qbb/util/handle.hpp>

#include <boost/optional.hpp>

#include <memory>
#include <vector>

namespace qbb
{
namespace qubus
{

class function_declaration_info;

// TODO: add may_alias attribute
class function_declaration
{
public:
    function_declaration(std::string name_, std::vector<variable_declaration> params_,
                         variable_declaration result_, expression body_);

    const std::string& name() const;
    const std::vector<variable_declaration>& params() const;
    const variable_declaration& result() const;
    
    void substitute_body(expression body);

    const expression& body() const;

    std::vector<expression> sub_expressions() const;

    util::handle id() const;

    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    std::shared_ptr<function_declaration_info> info_;
};

bool operator==(const function_declaration& lhs, const function_declaration& rhs);
bool operator!=(const function_declaration& lhs, const function_declaration& rhs);
}
}

#endif