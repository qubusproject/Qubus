#ifndef QUBUS_QTL_AST_TRANSLATION_HPP
#define QUBUS_QTL_AST_TRANSLATION_HPP

#include <hpx/config.hpp>

#include <qubus/qtl/ast.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/qtl/IR/all.hpp>

#include <hpx/include/naming.hpp>

#include <boost/hana/for_each.hpp>

#include <qubus/util/unreachable.hpp>

#include <qubus/util/optional_ref.hpp>
#include <qubus/util/unused.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace qubus
{
namespace qtl
{
namespace ast
{

template <typename T, long int Rank>
class tensor_expr;

template <binary_operator_tag Tag, typename LHS, typename RHS>
auto translate_ast(binary_operator<Tag, LHS, RHS> node, domain& dom)
{
    auto lhs = translate_ast(node.lhs(), dom);
    auto rhs = translate_ast(node.rhs(), dom);

    switch (node.tag())
    {
    case binary_operator_tag::plus:
        return qubus::operator+(std::move(lhs), std::move(rhs));
    case binary_operator_tag::minus:
        return qubus::operator-(std::move(lhs), std::move(rhs));
    case binary_operator_tag::multiplies:
        return qubus::operator*(std::move(lhs), std::move(rhs));
    case binary_operator_tag::divides:
        return qubus::operator/(std::move(lhs), std::move(rhs));
    }

    QUBUS_UNREACHABLE();
}

template <typename Body>
auto translate_ast(contraction<index, Body> node, domain& dom)
{
    auto idx_var = node.contraction_index().var();

    auto body = translate_ast(node.body(), dom);

    return qubus::qtl::sum(std::move(idx_var), std::move(body));
}

template <long int Rank, typename Body>
auto translate_ast(contraction<multi_index<Rank>, Body> node, domain& dom)
{
    auto multi_idx_var = node.contraction_index().var();

    std::vector<variable_declaration> indices;

    for (long int i = 0; i < node.contraction_index().rank(); ++i)
    {
        auto idx_var = node.contraction_index()[i].var();

        indices.push_back(idx_var);
    }

    auto body = translate_ast(node.body(), dom);

    return qubus::qtl::sum(std::move(indices), std::move(multi_idx_var), std::move(body));
}

template <typename Tensor, typename... Indices>
auto translate_ast(subscripted_tensor<Tensor, Indices...> node, domain& dom)
{
    auto tensor = translate_ast(node.tensor(), dom);

    auto accessible_tensor = tensor->template try_as<access_expr>();

    if (!accessible_tensor)
        throw 0; // Invalid tensor

    std::vector<std::unique_ptr<expression>> indices;

    boost::hana::for_each(node.indices(), [&indices, &dom](auto index) {
        indices.push_back(translate_ast(index, dom));
    });

    // FIXME: Avoid the copy of accessible_tensor.
    return subscription(clone(*accessible_tensor), std::move(indices));
}

template <typename Tensor, typename... Ranges>
auto translate_ast(sliced_tensor<Tensor, Ranges...> node, domain& dom)
{
    auto tensor = translate_ast(node.tensor(), dom);

    std::vector<std::unique_ptr<expression>> offset;
    offset.reserve(sizeof...(Ranges));

    std::vector<std::unique_ptr<expression>> shape;
    shape.reserve(sizeof...(Ranges));

    std::vector<std::unique_ptr<expression>> strides;
    strides.reserve(sizeof...(Ranges));

    boost::hana::for_each(node.ranges(), [&offset, &shape, &strides](auto range) {
        offset.push_back(integer_literal(range.start));
        shape.push_back(integer_literal((range.end - range.start) / range.stride));
        strides.push_back(integer_literal(range.stride));
    });

    return slice(std::move(tensor), std::move(offset), std::move(shape), std::move(strides));
}

inline auto translate_ast(const index& idx, domain& /*unused*/)
{
    return var(idx.var());
}

template <long int Rank>
auto translate_ast(const multi_index<Rank>& idx, domain& /*unused*/)
{
    return capture_multi_index(idx);
}

template <typename T, long int Rank>
auto translate_ast(const tensor_expr<T, Rank>& tensor, domain& /*unused*/)
{
    return clone(tensor.code());
}

template <typename T, long int Rank>
auto translate_ast(const tensor_var<T, Rank>& tensor, domain& /*unused*/)
{
    return var(tensor.var());
}

template <typename T, long int Rank>
auto translate_ast(const sparse_tensor_var<T, Rank>& tensor, domain& /*unused*/)
{
    return var(tensor.var());
}

template <typename Tensor,
          typename /*Enabled*/ = typename std::enable_if<!std::is_arithmetic<Tensor>::value>::type>
auto translate_ast(const Tensor& tensor, domain& /*unused*/)
{
    return qubus::qtl::obj(tensor.get_object());
}

template <typename T,
          typename /*Enabled*/ = typename std::enable_if<std::is_arithmetic<T>::value>::type>
auto translate_ast(T node, domain& dom)
{
    return lit(node);
}

template <typename FirstIndex, typename SecondIndex>
auto translate_ast(kronecker_delta<FirstIndex, SecondIndex> node, domain& dom)
{
    auto first_index = translate_ast(node.first_index(), dom);
    auto second_index = translate_ast(node.second_index(), dom);

    return qubus::qtl::kronecker_delta(node.extent(), std::move(first_index),
                                       std::move(second_index));
}
}
}
}

#endif
