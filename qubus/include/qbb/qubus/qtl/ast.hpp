#ifndef QUBUS_QTL_AST_HPP
#define QUBUS_QTL_AST_HPP

#include <qbb/qubus/qtl/index.hpp>

#include <boost/hana/any.hpp>
#include <boost/hana/tuple.hpp>

#include <qbb/util/integers.hpp>

#include <functional>
#include <type_traits>
#include <utility>

inline namespace qbb
{
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
template <typename T>
struct is_term : std::is_arithmetic<T>
{
};

template <typename T>
struct literal
{
public:
    explicit literal(T value_) : value_(std::move(value_))
    {
    }

    const T& value() const
    {
        return value_;
    }

private:
    T value_;
};

template <typename Tensor, typename... Indices>
class subscripted_tensor;

template <typename Tensor, typename... Ranges>
class sliced_tensor;

template <typename T>
class variable
{
public:
    variable() : id_(new_here<id_type_server>())
    {
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type = std::conditional_t<
            boost::hana::any(boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
            sliced_tensor<variable<T>, Indices...>, subscripted_tensor<variable<T>, Indices...>>;

        return subscription_type(*this, std::move(indices)...);
    }

    hpx::naming::gid_type id() const
    {
        return id_.get_id().get_gid();
    }

private:
    id_type id_;
};

template <typename T>
struct is_term<variable<T>> : std::true_type
{
};

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

template <typename Arg>
auto operator+(Arg arg)
{
    static_assert(is_term<Arg>::value, "Expecting a term as the argument.");

    return unary_operator<unary_operator_tag::plus, Arg>(std::move(arg));
}

template <typename Arg>
auto operator-(Arg arg)
{
    static_assert(is_term<Arg>::value, "Expecting a term as the argument.");

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

template <typename LHS, typename RHS>
auto operator+(LHS lhs, RHS rhs)
{
    static_assert(is_term<LHS>::value, "Expecting a term as the left-hand side.");
    static_assert(is_term<RHS>::value, "Expecting a term as the right-hand side.");

    return binary_operator<binary_operator_tag::plus, LHS, RHS>(std::move(lhs), std::move(rhs));
}

template <typename LHS, typename RHS>
auto operator-(LHS lhs, RHS rhs)
{
    static_assert(is_term<LHS>::value, "Expecting a term as the left-hand side.");
    static_assert(is_term<RHS>::value, "Expecting a term as the right-hand side.");

    return binary_operator<binary_operator_tag::minus, LHS, RHS>(std::move(lhs), std::move(rhs));
}

template <typename LHS, typename RHS>
auto operator*(LHS lhs, RHS rhs)
{
    static_assert(is_term<LHS>::value, "Expecting a term as the left-hand side.");
    static_assert(is_term<RHS>::value, "Expecting a term as the right-hand side.");

    return binary_operator<binary_operator_tag::multiplies, LHS, RHS>(std::move(lhs),
                                                                      std::move(rhs));
}

template <typename LHS, typename RHS>
auto operator/(LHS lhs, RHS rhs)
{
    static_assert(is_term<LHS>::value, "Expecting a term as the left-hand side.");
    static_assert(is_term<RHS>::value, "Expecting a term as the right-hand side.");

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

template <typename ContractionIndex, typename Body>
auto sum(ContractionIndex contraction_index, Body body)
{
    static_assert(is_index<ContractionIndex>::value, "Expecting an index.");
    static_assert(is_term<Body>::value, "Expecting a term as the sum body.");

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

template <typename FirstIndex, typename SecondIndex>
auto delta(util::index_t extent, FirstIndex first_index, SecondIndex second_index)
{
    static_assert(is_index<FirstIndex>::value, "Expecting an index as the first argument.");
    static_assert(is_index<SecondIndex>::value, "Expecting an index as the second argument.");

    return kronecker_delta<FirstIndex, SecondIndex>(std::move(extent), std::move(first_index),
                                                    std::move(second_index));
}
}

template <typename T>
using variable = ast::variable<T>;
}
}
}

#endif
