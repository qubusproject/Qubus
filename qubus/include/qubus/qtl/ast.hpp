#ifndef QUBUS_QTL_AST_HPP
#define QUBUS_QTL_AST_HPP

#include <qubus/qtl/index.hpp>
#include <qubus/qtl/kernel.hpp>

#include <qubus/associated_qubus_type.hpp>

#include <qubus/IR/access.hpp>
#include <qubus/IR/binary_operator_expr.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/subscription_expr.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <boost/hana/any.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <qubus/util/integers.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace qubus
{
namespace qtl
{

struct range
{
    range(util::index_t start, util::index_t end, util::index_t stride = 1)
    : start(start), end(end), stride(stride)
    {
    }

    util::index_t start;
    util::index_t end;
    util::index_t stride;
};

namespace ast
{
struct domain
{
};

template <typename T>
struct is_term : std::is_arithmetic<T>
{
};

template <typename Tensor, typename... Indices>
class subscripted_tensor;

template <typename Tensor, typename... Ranges>
class sliced_tensor;

template <typename Tensor, typename... Indices>
class subscripted_tensor
{
public:
    explicit subscripted_tensor(Tensor tensor_, Indices... indices_)
    : tensor_(std::move(tensor_)), indices_(boost::hana::make_tuple(indices_...))
    {
    }

    const Tensor& tensor() const
    {
        return tensor_;
    }

    const boost::hana::tuple<Indices...>& indices() const
    {
        return indices_;
    }

    template <typename RHS>
    void operator=(RHS rhs) const
    {
        domain dom;

        std::vector<std::unique_ptr<expression>> translated_indices;

        boost::hana::for_each(indices(), [&translated_indices, &dom](auto index) {
            translated_indices.push_back(translate_ast(index, dom));
        });

        auto translated_tensor = translate_ast(tensor(), dom);

        auto accessible_tensor = translated_tensor->template try_as<access_expr>();

        if (!accessible_tensor)
            throw 0; // Invalid left-hand side

        // FIXME: Avoid the copy of accessible_tensor.
        auto lhs = subscription(clone(*accessible_tensor), std::move(translated_indices));

        auto translated_rhs = translate_ast(rhs, dom);

        auto code = assign(std::move(lhs), std::move(translated_rhs));

        this_kernel::add_code(std::move(code));
    }

private:
    Tensor tensor_;
    boost::hana::tuple<Indices...> indices_;
};

template <typename Tensor, typename... Indices>
struct is_term<subscripted_tensor<Tensor, Indices...>> : std::true_type
{
};

template <typename Tensor, typename... Ranges>
class sliced_tensor
{
public:
    explicit sliced_tensor(Tensor tensor_, Ranges... ranges_)
    : tensor_(std::move(tensor_)), ranges_(boost::hana::make_tuple(ranges_...))
    {
    }

    const Tensor& tensor() const
    {
        return tensor_;
    }

    const boost::hana::tuple<Ranges...>& ranges() const
    {
        return ranges_;
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type = std::conditional_t<boost::hana::any(boost::hana::make_tuple(
                                                         std::is_same<Indices, range>::value...)),
                                                     sliced_tensor<sliced_tensor, Indices...>,
                                                     subscripted_tensor<sliced_tensor, Indices...>>;

        return subscription_type(*this, std::move(indices)...);
    }

private:
    Tensor tensor_;
    boost::hana::tuple<Ranges...> ranges_;
};

template <typename Tensor, typename... Indices>
struct is_term<sliced_tensor<Tensor, Indices...>> : std::true_type
{
};

enum class unary_operator_tag
{
    plus,
    negate
};

template <unary_operator_tag Tag, typename Arg>
class unary_operator
{
public:
    explicit unary_operator(Arg arg_) : arg_(std::move(arg_))
    {
    }

    constexpr unary_operator_tag tag() const
    {
        return Tag;
    }

    const Arg& arg() const
    {
        return arg_;
    }

private:
    Arg arg_;
};

template <unary_operator_tag Tag, typename Arg>
struct is_term<unary_operator<Tag, Arg>> : std::true_type
{
};

template <typename Arg, typename = typename std::enable_if<is_term<Arg>::value>::type>
auto operator+(Arg arg)
{
    return unary_operator<unary_operator_tag::plus, Arg>(std::move(arg));
}

template <typename Arg, typename = typename std::enable_if<is_term<Arg>::value>::type>
auto operator-(Arg arg)
{
    return unary_operator<unary_operator_tag::negate, Arg>(std::move(arg));
}

enum class binary_operator_tag
{
    plus,
    minus,
    multiplies,
    divides
};

template <binary_operator_tag Tag, typename LHS, typename RHS>
class binary_operator
{
public:
    binary_operator(LHS lhs_, RHS rhs_) : lhs_(std::move(lhs_)), rhs_(std::move(rhs_))
    {
    }

    constexpr binary_operator_tag tag() const
    {
        return Tag;
    }

    const LHS& lhs() const
    {
        return lhs_;
    }

    const RHS& rhs() const
    {
        return rhs_;
    }

private:
    LHS lhs_;
    RHS rhs_;
};

template <binary_operator_tag Tag, typename LHS, typename RHS>
struct is_term<binary_operator<Tag, LHS, RHS>> : std::true_type
{
};

template <typename LHS, typename RHS,
          typename = typename std::enable_if<is_term<LHS>::value && is_term<RHS>::value>::type>
auto operator+(LHS lhs, RHS rhs)
{
    return binary_operator<binary_operator_tag::plus, LHS, RHS>(std::move(lhs), std::move(rhs));
}

template <typename LHS, typename RHS,
          typename = typename std::enable_if<is_term<LHS>::value && is_term<RHS>::value>::type>
auto operator-(LHS lhs, RHS rhs)
{
    return binary_operator<binary_operator_tag::minus, LHS, RHS>(std::move(lhs), std::move(rhs));
}

template <typename LHS, typename RHS,
          typename = typename std::enable_if<is_term<LHS>::value && is_term<RHS>::value>::type>
auto operator*(LHS lhs, RHS rhs)
{
    return binary_operator<binary_operator_tag::multiplies, LHS, RHS>(std::move(lhs),
                                                                      std::move(rhs));
}

template <typename LHS, typename RHS,
          typename = typename std::enable_if<is_term<LHS>::value && is_term<RHS>::value>::type>
auto operator/(LHS lhs, RHS rhs)
{
    return binary_operator<binary_operator_tag::divides, LHS, RHS>(std::move(lhs), std::move(rhs));
}

template <typename ContractionIndex, typename Body>
class contraction
{
public:
    contraction(ContractionIndex contraction_index_, Body body_)
    : contraction_index_(std::move(contraction_index_)), body_(std::move(body_))
    {
    }

    const ContractionIndex& contraction_index() const
    {
        return contraction_index_;
    }

    const Body& body() const
    {
        return body_;
    }

private:
    ContractionIndex contraction_index_;
    Body body_;
};

template <typename ContractionIndex, typename Body>
struct is_term<contraction<ContractionIndex, Body>> : std::true_type
{
};

template <typename ContractionIndex, typename Body,
          typename = typename std::enable_if<is_index<ContractionIndex>::value &&
                                             is_term<Body>::value>::type>
auto sum(ContractionIndex contraction_index, Body body)
{
    return contraction<ContractionIndex, Body>(std::move(contraction_index), std::move(body));
}

template <typename FirstIndex, typename SecondIndex>
class kronecker_delta
{
public:
    kronecker_delta(util::index_t extent_, FirstIndex first_index_, SecondIndex second_index_)
    : extent_(std::move(extent_)),
      first_index_(std::move(first_index_)),
      second_index_(std::move(second_index_))
    {
    }

    util::index_t extent() const
    {
        return extent_;
    }

    const FirstIndex& first_index() const
    {
        return first_index_;
    }

    const SecondIndex& second_index() const
    {
        return second_index_;
    }

private:
    util::index_t extent_;
    FirstIndex first_index_;
    SecondIndex second_index_;
};

template <typename FirstIndex, typename SecondIndex>
struct is_term<kronecker_delta<FirstIndex, SecondIndex>> : std::true_type
{
};

template <typename FirstIndex, typename SecondIndex,
          typename = typename std::enable_if<is_index<FirstIndex>::value &&
                                             is_index<SecondIndex>::value>::type>
auto delta(util::index_t extent, FirstIndex first_index, SecondIndex second_index)
{
    return kronecker_delta<FirstIndex, SecondIndex>(std::move(extent), std::move(first_index),
                                                    std::move(second_index));
}

template <typename T, long int Rank>
class tensor_var
{
public:
    // FIXME: Add a proper name for each tensor.
    tensor_var() : var_("tensor", types::array(associated_qubus_type<T>::get(), Rank))
    {
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type =
            std::conditional_t<boost::hana::any(
                                   boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
                               sliced_tensor<tensor_var<T, Rank>, Indices...>,
                               subscripted_tensor<tensor_var<T, Rank>, Indices...>>;

        return subscription_type(*this, std::move(indices)...);
    }

    const variable_declaration& var() const
    {
        return var_;
    }

private:
    variable_declaration var_;
};

template <typename T, long int Rank>
struct is_term<tensor_var<T, Rank>> : std::true_type
{
};

template <typename T, long int Rank>
class sparse_tensor_var
{
public:
    // FIXME: Add a proper name for each tensor.
    sparse_tensor_var() : var_("tensor", types::sparse_tensor(associated_qubus_type<T>::get()))
    {
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type =
            std::conditional_t<boost::hana::any(
                                   boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
                               sliced_tensor<sparse_tensor_var<T, Rank>, Indices...>,
                               subscripted_tensor<sparse_tensor_var<T, Rank>, Indices...>>;

        return subscription_type(*this, std::move(indices)...);
    }

    const variable_declaration& var() const
    {
        return var_;
    }

private:
    variable_declaration var_;
};

template <typename T, long int Rank>
struct is_term<sparse_tensor_var<T, Rank>> : std::true_type
{
};
}

template <typename T, long int Rank>
using tensor_var = ast::tensor_var<T, Rank>;

template <typename T, long int Rank>
using sparse_tensor_var = ast::sparse_tensor_var<T, Rank>;
}
}

#include <qubus/qtl/ast_translation.hpp>

#endif
