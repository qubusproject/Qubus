#ifndef QUBUS_INDEXED_TENSOR_EXPR_CONTEXT_HPP
#define QUBUS_INDEXED_TENSOR_EXPR_CONTEXT_HPP

#include <boost/proto/core.hpp>
#include <boost/proto/context.hpp>
#include <boost/proto/transform.hpp>
#include <boost/proto/debug.hpp>
#include <boost/proto/functional.hpp>

#include <vector>
#include <set>
#include <algorithm>

#include <array>

#include <boost/type_traits.hpp>

namespace qbb
{
namespace qubus
{

namespace proto = boost::proto;

struct tensor_expr_domain;

template<typename Expr>
struct tensor_expr_;

struct tensor_expr_domain
: proto::domain< proto::generator< tensor_expr_ > >
{};

template<typename Expr>
struct tensor_expr_
: proto::extends<Expr, tensor_expr_<Expr>, tensor_expr_domain >
{
        typedef proto::extends<Expr, tensor_expr_<Expr>, tensor_expr_domain> base_type;

        BOOST_PROTO_EXTENDS_USING_ASSIGN(tensor_expr_<Expr>)

        tensor_expr_(const Expr& expr = Expr())
        :tensor_expr_::proto_extends(expr)
        {}
};

}
}

#endif