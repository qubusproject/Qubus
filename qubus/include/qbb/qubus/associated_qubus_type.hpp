#ifndef QUBUS_ASSOCIATED_QUBUS_TYPE_HPP
#define QUBUS_ASSOCIATED_QUBUS_TYPE_HPP

#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>

#include <complex>

namespace qubus
{
 
template<typename T>
struct associated_qubus_type;

template<>
struct associated_qubus_type<double>
{
    static type get()
    {
        return types::double_();
    }
};

template<>
struct associated_qubus_type<float>
{
    static type get()
    {
        return types::float_();
    }
};

template<>
struct associated_qubus_type<util::index_t>
{
    static type get()
    {
        return types::integer();
    }
};

template<typename T>
struct associated_qubus_type<std::complex<T>>
{
    static type get()
    {
        return types::complex(associated_qubus_type<T>::get());
    }
};
    
}

#endif