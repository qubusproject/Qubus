#ifndef ASSOCIATED_KUBUS_TYPE_HPP
#define ASSOCIATED_KUBUS_TYPE_HPP

#include <qbb/kubus/IR/type.hpp>

#include <qbb/util/integers.hpp>

#include <complex>

namespace qbb
{
namespace qubus
{
 
template<typename T>
struct associated_kubus_type;

template<>
struct associated_kubus_type<double>
{
    static type get()
    {
        return types::double_();
    }
};

template<>
struct associated_kubus_type<float>
{
    static type get()
    {
        return types::float_();
    }
};

template<>
struct associated_kubus_type<util::index_t>
{
    static type get()
    {
        return types::integer();
    }
};

template<typename T>
struct associated_kubus_type<std::complex<T>>
{
    static type get()
    {
        return types::complex(associated_kubus_type<T>::get());
    }
};
    
}
}

#endif