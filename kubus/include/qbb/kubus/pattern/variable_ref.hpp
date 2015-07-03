#ifndef QBB_QUBUS_PATTERN_VARIABLE_REF_HPP
#define QBB_QUBUS_PATTERN_VARIABLE_REF_HPP

#include <qbb/kubus/IR/variable_ref_expr.hpp>
#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename Declaration>
class variable_ref_pattern
{
public:
    explicit variable_ref_pattern(Declaration declaration_)
    : declaration_(declaration_)
    {
    }
    
    template <typename BaseType>
    bool match(const BaseType& value, const variable<variable_ref_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<variable_ref_expr>())
        {
            if (declaration_.match(concret_value->declaration()))
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
        declaration_.reset();
    }
private:
    Declaration declaration_;
};

template <typename Declaration>
variable_ref_pattern<Declaration> variable_ref(Declaration declaration)
{
    return variable_ref_pattern<Declaration>(declaration);
}

inline variable_ref_pattern<any> variable_ref()
{
    return variable_ref_pattern<any>(_);
}

}
}
}

#endif