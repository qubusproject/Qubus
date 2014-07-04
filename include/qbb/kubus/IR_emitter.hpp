#ifndef KUBUS_AST_EMITTER_HPP
#define KUBUS_AST_EMITTER_HPP

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/grammar.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/integer_sequence.hpp>

namespace qbb
{
namespace kubus
{

template<typename Evaluator>
struct emit_plus_node : proto::transform<emit_plus_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::plus, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_minus_node : proto::transform<emit_minus_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::minus, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_multiplies_node : proto::transform<emit_multiplies_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::multiplies, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_divides_node : proto::transform<emit_divides_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::divides, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_assign_node : proto::transform<emit_assign_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::assign, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_equal_to_node : proto::transform<emit_equal_to_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::equal_to, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_not_equal_to_node : proto::transform<emit_not_equal_to_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::not_equal_to, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_less_node : proto::transform<emit_less_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::less, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_greater_node : proto::transform<emit_greater_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::greater, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_less_equal_node : proto::transform<emit_less_equal_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::less_equal, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_greater_equal_node : proto::transform<emit_greater_equal_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::greater_equal, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_logical_and_node : proto::transform<emit_logical_and_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::logical_and, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_logical_or_node : proto::transform<emit_logical_or_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return binary_operator_expr(binary_op_tag::logical_or, Evaluator()(proto::left(expr),state,data), Evaluator()(proto::right(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_unary_plus_node : proto::transform<emit_unary_plus_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return unary_operator_expr(unary_op_tag::plus, Evaluator()(proto::child_c<0>(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_negate_node : proto::transform<emit_negate_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return unary_operator_expr(unary_op_tag::negate, Evaluator()(proto::child_c<0>(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_logical_not_node : proto::transform<emit_logical_not_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return unary_operator_expr(unary_op_tag::logical_not, Evaluator()(proto::child_c<0>(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_sum_node : proto::transform<emit_sum_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return sum_expr(Evaluator()(proto::child_c<0>(expr),state,data),
                          Evaluator()(proto::child_c<1>(expr),state,data));
      }
    };
};

template<typename Evaluator>
struct emit_index_node : proto::transform<emit_index_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return index_expr(std::string(1,id(proto::value(expr))));
      }
    };
};

namespace detail
{

template<typename Evaluator,typename Expr,typename State,typename Data,std::size_t Front,std::size_t... Indices>
inline std::vector<expression> emit_indices(const Expr& expr,
                                                                const State& state,
                                                                const Data& data,
                                                                util::index_sequence<Front,Indices...>)
{
    std::vector<expression> indices;

    auto dummy = {(indices.push_back(Evaluator()(proto::child_c<Indices>(expr),state,data)),0)...};

    return indices;
}

}

template<typename Evaluator>
struct emit_tensor_node : proto::transform<emit_tensor_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          constexpr std::size_t arity = proto::arity_of<Expr>::value;

          auto indices = detail::emit_indices<Evaluator>(expr,state,data,util::make_index_sequence<arity>());

          qbb::util::handle_factory fac;
          qbb::util::handle h = fac.create();
          
          return subscription_expr(tensor_variable_expr(h, types::tensor(types::double_{})),std::move(indices));
      }
    };
};

template<typename Evaluator>
struct emit_literal_node : proto::transform<emit_literal_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          return double_literal_expr(proto::value(expr));
      }
    };
};

namespace detail
{

template<typename Evaluator,typename Expr,typename State,typename Data,std::size_t... Indices>
inline std::vector<expression> emit_args(const Expr& expr,
                                                             const State& state,
                                                             const Data& data,
                                                             util::index_sequence<Indices...>)
{
    std::vector<expression> args;

    auto dummy = {(args.push_back(Evaluator()(proto::child_c<Indices>(expr),state,data)),0)...};

    return args;
}

inline constexpr const char* function_tag_to_name(tag::exp) { return "exp"; }
inline constexpr const char* function_tag_to_name(tag::sin) { return "sin"; }
inline constexpr const char* function_tag_to_name(tag::cos) { return "cos"; }
inline constexpr const char* function_tag_to_name(tag::tan) { return "tan"; }
inline constexpr const char* function_tag_to_name(tag::sinh) { return "sinh"; }
inline constexpr const char* function_tag_to_name(tag::cosh) { return "cosh"; }
inline constexpr const char* function_tag_to_name(tag::tanh) { return "tanh"; }

}

template<typename Evaluator>
struct emit_function_node : proto::transform<emit_function_node<Evaluator> >
{
   template<typename Expr, typename State, typename Data>
   struct impl : proto::transform_impl<Expr, State, Data>
   {
      using result_type = expression;

      result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
         ) const
      {
          constexpr std::size_t arity = proto::arity_of<Expr>::value;

          auto args = detail::emit_args<Evaluator>(expr,state,data,util::make_index_sequence<arity>());

          const char* name = detail::function_tag_to_name(typename proto::tag_of<Expr>::type{});

          return intrinsic_function_expr(name,std::move(args));
      }
    };
};

struct emit_AST;

struct emit_arithmetic_operations
: proto::or_<
      proto::when< proto::plus< proto::_,proto::_ >,
                   emit_plus_node<emit_AST>
                 >,
      proto::when< proto::minus< proto::_,proto::_ >,
                   emit_minus_node<emit_AST>
                 >,
      proto::when< proto::multiplies< proto::_,proto::_ >,
                   emit_multiplies_node<emit_AST>
                 >,
      proto::when< proto::divides< proto::_,proto::_ >,
                   emit_divides_node<emit_AST>
                 >,
      proto::when< proto::unary_plus< proto::_ >,
                   emit_unary_plus_node<emit_AST>
                 >,
      proto::when< proto::negate< proto::_ >,
                   emit_negate_node<emit_AST>
                 >
            >
{
};

struct emit_logical_operations
: proto::or_<
      proto::when< proto::logical_and< proto::_,proto::_ >,
                   emit_logical_and_node<emit_AST>
                 >,
      proto::when< proto::logical_or< proto::_,proto::_ >,
                   emit_logical_or_node<emit_AST>
                 >,
      proto::when< proto::logical_not< proto::_ >,
                   emit_logical_not_node<emit_AST>
                 >
            >
{
};

struct emit_AST
: proto::or_<
      emit_logical_operations,
      emit_arithmetic_operations,
      proto::when< indexed_tensor,
                   emit_tensor_node<emit_AST>
                 >,
      proto::when< proto::assign< proto::_,proto::_ >,
                   emit_assign_node<emit_AST>
                 >,
      proto::when< sum_< proto::_,proto::_ >,
                   emit_sum_node<emit_AST>
                 >,
      proto::when< unary_function_< proto::_, proto::_>,
                   emit_function_node<emit_AST>
                 >,
      proto::when< index_terminal,
                   emit_index_node<emit_AST>
                 >,
      proto::when< proto::terminal<proto::_>,
                   emit_literal_node<emit_AST>
                 >
  >
{
};

}
}

#endif
