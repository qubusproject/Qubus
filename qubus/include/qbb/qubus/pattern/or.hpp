#ifndef QBB_QUBUS_PATTERN_OR_HPP
#define QBB_QUBUS_PATTERN_OR_HPP

#include <qbb/qubus/pattern/variable.hpp>

#include <utility>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename LHS, typename RHS>
class or_pattern
{
public:
    or_pattern(LHS lhs_, RHS rhs_) : lhs_(std::move(lhs_)), rhs_(std::move(rhs_))
    {
    }

    template <typename BaseType>
    bool match(BaseType&& value, const variable<BaseType>* var = nullptr) const
    {
        if (lhs_.match(value) || rhs_.match(value))
        {
            if (var)
            {
                var->set(value);
            }
            
            return true;
        }
        
        return false;
    }

    void reset() const
    {
        lhs_.reset();
        rhs_.reset();
    }
private:
    LHS lhs_;
    RHS rhs_;
};

template<typename LHS, typename RHS>
or_pattern<LHS, RHS> operator||(LHS lhs, RHS rhs)
{
    return or_pattern<LHS, RHS>(lhs, rhs);
}

}
}
}

#endif