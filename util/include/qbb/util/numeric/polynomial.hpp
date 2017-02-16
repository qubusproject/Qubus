#ifndef QBB_UTIL_POLYNOMIAL_HPP
#define QBB_UTIL_POLYNOMIAL_HPP

#include <qbb/util/integers.hpp>

#include <cmath>
#include <vector>
#include <utility>

inline namespace qbb
{
namespace util
{
namespace numeric
{

template<typename T>
class polynomial
{
public:
    explicit polynomial(std::vector<T> coefficients_)
    : coefficients_(std::move(coefficients_))
    {
    }
    
    T operator()(T x) const
    {
        index_t degree = to_uindex(coefficients_.size());
        
        T result = 0;
        
        for (index_t i = 0; i < degree; ++i)
        {
            result = result * x + coefficients_[i];
        }
        
        return result;
    }
private:
    std::vector<T> coefficients_;
};

}   
}
}

#endif