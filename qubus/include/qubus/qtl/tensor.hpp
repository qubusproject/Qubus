#ifndef QUBUS_TENSOR_VAR_HPP
#define QUBUS_TENSOR_VAR_HPP

#include <qubus/runtime.hpp>

#include <qubus/qtl/ast.hpp>
#include <qubus/array.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/integers.hpp>

#include <qubus/IR/expression.hpp>

#include <qubus/get_view.hpp>
#include <qubus/array.hpp>

#include <qubus/associated_qubus_type.hpp>

#include <boost/hana/any.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace qubus
{
namespace qtl
{
namespace ast
{

template <typename T, long int Rank>
class tensor;

template <typename T, long int Rank>
class tensor_expr
{
public:
     template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type = std::conditional_t<
                boost::hana::any(boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
                sliced_tensor<tensor_expr<T, Rank>, Indices...>, subscripted_tensor<tensor_expr<T, Rank>, Indices...>>;

        return subscription_type(*this, indices...);
    }

    const expression& code() const
    {
        return *code_;
    }

private:
    std::shared_ptr<expression> code_;
};

template <typename T, long int Rank>
class tensor
{
public:
    using value_type = T;

    template <typename... SizeTypes>
    explicit tensor(SizeTypes... sizes_)
    : data_(sizes_...)
    {
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type = std::conditional_t<
                boost::hana::any(boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
                sliced_tensor<tensor<T, Rank>, Indices...>, subscripted_tensor<tensor<T, Rank>, Indices...>>;

        return subscription_type(*this, indices...);
    }

    object get_object() const
    {
        return data_.get_object();
    }

private:
    array<T, Rank> data_;
};

template <typename T, long int Rank, typename AccessType, typename ArchTag>
auto get_view(const tensor<T, Rank>& value, AccessType access_type, ArchTag arch)
{
    return get_view<array<T, Rank>>(value.get_object(), access_type, arch);
}
}

template <typename T, long int Rank>
using tensor_expr = ast::tensor_expr<T, Rank>;

template <typename T, long int Rank>
using tensor = ast::tensor<T, Rank>;

using ast::get_view;
}

template <typename T, long int Rank>
struct associated_qubus_type<qtl::ast::tensor<T, Rank>>
{
    static type get()
    {
        return types::array(associated_qubus_type<T>::get(), Rank);
    }
};
}

#endif