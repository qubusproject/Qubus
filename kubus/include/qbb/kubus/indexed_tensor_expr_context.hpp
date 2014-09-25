#ifndef KUBUS_INDEXED_TENSOR_EXPR_CONTEXT_HPP
#define KUBUS_INDEXED_TENSOR_EXPR_CONTEXT_HPP

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
namespace kubus
{

namespace proto = boost::proto;

struct tensor_expr_domain;

template<template<std::size_t Rank> class IterationEngine,typename Expr,typename Tensor,
         typename AssignmentOp>
inline void eval_tensor_expr(Tensor& dst,const Expr& expr,AssignmentOp assignment_op);

template<std::size_t RankOfExpr>
class simple_interation_engine;

template<std::size_t RankOfExpr>
class tiling_interation_engine;

template<typename Expr>
struct tensor_expr;

struct tensor_expr_domain
: proto::domain< proto::generator< tensor_expr > >
{};

template<typename Expr>
struct tensor_expr
: proto::extends<Expr, tensor_expr<Expr>, tensor_expr_domain >
{
        typedef proto::extends<Expr, tensor_expr<Expr>, tensor_expr_domain> base_type;

        BOOST_PROTO_EXTENDS_USING_ASSIGN(tensor_expr<Expr>)

        tensor_expr(const Expr& expr = Expr())
        :tensor_expr::proto_extends(expr)
        {}
};

}
}

#endif