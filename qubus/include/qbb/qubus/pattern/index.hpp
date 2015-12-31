#ifndef QBB_QUBUS_PATTERN_INDEX_HPP
#define QBB_QUBUS_PATTERN_INDEX_HPP

#include <qbb/qubus/IR/variable_ref_expr.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>
#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/pattern/type.hpp>
#include <qbb/qubus/IR/type.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
namespace pattern
{
template <typename Declaration>
class index_pattern
{
public:
    index_pattern(Declaration declaration_) : declaration_(declaration_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<variable_declaration>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<variable_ref_expr>())
        {
            if (concret_value->declaration().var_type() == types::index())
            {
                if (declaration_.match(concret_value->declaration()))
                {
                    if (var)
                    {
                        var->set(concret_value->declaration());
                    }

                    return true;
                }
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
index_pattern<Declaration> index(Declaration declaration)
{
    return index_pattern<Declaration>(declaration);
}

inline index_pattern<any> index()
{
    return index_pattern<any>(_);
}

template <typename Declaration>
class multi_index_pattern
{
public:
    multi_index_pattern(Declaration declaration_) : declaration_(declaration_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<variable_declaration>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<variable_ref_expr>())
        {
            if (multi_index_t(_).match(concret_value->declaration().var_type()))
            {
                if (declaration_.match(concret_value->declaration()))
                {
                    if (var)
                    {
                        var->set(concret_value->declaration());
                    }

                    return true;
                }
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
multi_index_pattern<Declaration> multi_index(Declaration declaration)
{
    return multi_index_pattern<Declaration>(declaration);
}

inline multi_index_pattern<any> multi_index()
{
    return multi_index_pattern<any>(_);
}
}
}
}

#endif