#ifndef QUBUS_PATTERN_ACCESS_HPP
#define QUBUS_PATTERN_ACCESS_HPP

#include <qubus/IR/access.hpp>
#include <qubus/IR/access_qualifier.hpp>

#include <qubus/pattern/variable.hpp>

#include <utility>

namespace qubus
{
namespace pattern
{
class access_pattern
{
public:
    template<typename BaseType>
    bool match(const BaseType &value, const variable<const access_expr &> *var = nullptr) const
    {
        if (auto concret_value = value.template try_as<access_expr>())
        {
            if (var)
            {
                var->set(*concret_value);
            }

            return true;
        }

        return false;
    }

    void reset() const
    {
    }
};

template<typename QualifiedAccess>
class access_qualifier_pattern
{
public:
    explicit access_qualifier_pattern(QualifiedAccess qualified_access_)
            : qualified_access_(std::move(qualified_access_))
    {
    }

    template<typename BaseType>
    bool match(const BaseType &value,
               const variable<const access_qualifier_expr &> *var = nullptr) const
    {
        if (auto concret_value = value.template try_as<access_qualifier_expr>())
        {
            if (qualified_access_.match(concret_value->qualified_access()))
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
        qualified_access_.reset();
    }

private:
    QualifiedAccess qualified_access_;
};

inline access_pattern access()
{
    return access_pattern();
}

template<typename QualifiedAccess>
access_qualifier_pattern<QualifiedAccess> access_qualifier(QualifiedAccess qualified_access)
{
    return access_qualifier<QualifiedAccess>(std::move(qualified_access));
}

}
}

#endif
