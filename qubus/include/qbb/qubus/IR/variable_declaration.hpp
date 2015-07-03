#ifndef QBB_QUBUS_VARIABLE_DECLARATION_HPP
#define QBB_QUBUS_VARIABLE_DECLARATION_HPP

#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/annotations.hpp>

#include <qbb/util/handle.hpp>

#include <memory>
#include <functional>

namespace qbb
{
namespace qubus
{

class variable_declaration_info;

enum class variable_intent
{
    generic,
    in,
    out
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

namespace std
{

template<>
struct less<qbb::qubus::variable_declaration>
{
    using result_type = bool;
    using first_argument_type = qbb::qubus::variable_declaration;
    using second_argument_type = first_argument_type;

    bool operator()(const qbb::qubus::variable_declaration& lhs, const qbb::qubus::variable_declaration& rhs) const
    {
        return lhs.id() < rhs.id();
    }
};

}

#endif