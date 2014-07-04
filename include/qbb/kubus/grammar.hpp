#ifndef KUBUS_GRAMMAR_HPP
#define KUBUS_GRAMMAR_HPP

#include <qbb/kubus/indexed_tensor_expr_context.hpp>
#include <qbb/kubus/index.hpp>
#include <qbb/kubus/tensor_variable.hpp>

#include <boost/proto/core.hpp>

namespace qbb
{
namespace kubus
{
    
namespace proto = boost::proto;
   
template<typename T>
struct is_index : boost::mpl::false_
{};

template<char id>
struct is_index<index<id> > : boost::mpl::true_
{};

struct index_terminal
    : proto::and_<
            proto::terminal< proto::_ >
          , proto::if_< is_index< proto::_value >() >
        >
{};

template<typename T,std::size_t Rank>
class tensor_holder;

template<typename>
struct is_tensor : boost::mpl::false_
{};

template<>
struct is_tensor<tensor_variable> : boost::mpl::true_
{};

struct tensor_terminal
  : proto::and_<
        proto::terminal< proto::_ >
      , proto::if_< is_tensor< proto::_value >() >
       >
{};

struct index_grammar
: proto::or_<
      index_terminal
      ,proto::terminal<int>
       >
{};

namespace tag
{
        struct sum {};

        inline std::ostream& operator<<(std::ostream& os,sum)
        {
                return os << "sum";
        }

        struct select {};

        inline std::ostream& operator<<(std::ostream& os,select)
        {
                return os << "select";
        }
        
        struct delta {};

        inline std::ostream& operator<<(std::ostream& os,delta)
        {
                return os << "delta";
        }

        struct sin {};

        inline std::ostream& operator<<(std::ostream& os,sin)
        {
                return os << "sin";
        }

        struct cos {};

        inline std::ostream& operator<<(std::ostream& os,cos)
        {
                return os << "cos";
        }

        struct tan {};

        inline std::ostream& operator<<(std::ostream& os,tan)
        {
                return os << "tan";
        }

        struct sinh {};

        inline std::ostream& operator<<(std::ostream& os,sinh)
        {
                return os << "sinh";
        }

        struct cosh {};

        inline std::ostream& operator<<(std::ostream& os,cosh)
        {
                return os << "cosh";
        }

        struct tanh {};

        inline std::ostream& operator<<(std::ostream& os,tanh)
        {
                return os << "tanh";
        }

        struct exp {};

        inline std::ostream& operator<<(std::ostream& os,exp)
        {
                return os << "exp";
        }
}

template<typename Arg,typename Index>
struct sum_ : public proto::transform<sum_<Arg,Index> >
{
        typedef proto::expr< tag::sum, proto::list2< Arg, Index > >       type;
        typedef proto::basic_expr< tag::sum, proto::list2< Arg, Index > > proto_grammar;

        template<typename Expr, typename State, typename Data>
        struct impl :  proto::pass_through<sum_>::template impl<Expr, State, Data>
        {};
};

template<typename Arg,typename Index>
inline typename proto::result_of::make_expr<
    tag::sum
  , tensor_expr_domain
  , Arg const &
  , Index const &
>::type const
sum(Arg const &arg, Index const &index)
{
    return proto::make_expr<tag::sum,tensor_expr_domain>(
        boost::ref(arg)
      , boost::ref(index)
    );
}

template<typename Cond,typename Then,typename Else>
struct select_ : public proto::transform<select_<Cond,Then,Else> >
{
        typedef proto::expr< tag::select, proto::list3< Cond,Then,Else > >       type;
        typedef proto::basic_expr< tag::select, proto::list3< Cond,Then,Else > > proto_grammar;

        template<typename Expr, typename State, typename Data>
        struct impl :  proto::pass_through<select_>::template impl<Expr, State, Data>
        {};
};

template<typename Cond,typename Then,typename Else>
inline typename proto::result_of::make_expr<
    tag::select
  , tensor_expr_domain
  , Cond const &
  , Then const &
  , Else const &
>::type const
select(Cond const &cond, Then const &then,Else const &else_)
{
    return proto::make_expr<tag::select,tensor_expr_domain>(
        boost::ref(cond)
      , boost::ref(then)
      , boost::ref(else_)
    );
}

template<typename... Indices>
using delta_ = proto::nary_expr<tag::delta,Indices...>;

template<typename... Indices>
inline typename proto::result_of::make_expr<
    tag::delta
  , tensor_expr_domain
  , Indices const &...
>::type const
delta(Indices const &... indices)
{
    return proto::make_expr<tag::delta,tensor_expr_domain>(
        boost::ref(indices)...
    );
}

template<typename Tag,typename Arg>
using unary_function_ = proto::unary_expr<Tag,Arg>;

template<typename Arg>
using sin_ = unary_function_<tag::sin,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::sin
  , tensor_expr_domain
  , Arg const &
>::type const
sin(Arg const &arg)
{
    return proto::make_expr<tag::sin,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using cos_ = unary_function_<tag::cos,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::cos
  , tensor_expr_domain
  , Arg const &
>::type const
cos(Arg const &arg)
{
    return proto::make_expr<tag::cos,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using tan_ = unary_function_<tag::tan,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::tan
  , tensor_expr_domain
  , Arg const &
>::type const
tan(Arg const &arg)
{
    return proto::make_expr<tag::tan,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using sinh_ = unary_function_<tag::sinh,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::sinh
  , tensor_expr_domain
  , Arg const &
>::type const
sinh(Arg const &arg)
{
    return proto::make_expr<tag::sinh,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using cosh_ = unary_function_<tag::cosh,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::cosh
  , tensor_expr_domain
  , Arg const &
>::type const
cosh(Arg const &arg)
{
    return proto::make_expr<tag::cosh,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using tanh_ = unary_function_<tag::tanh,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::tanh
  , tensor_expr_domain
  , Arg const &
>::type const
tanh(Arg const &arg)
{
    return proto::make_expr<tag::tanh,tensor_expr_domain>(
        boost::ref(arg)
    );
}

template<typename Arg>
using exp_ = unary_function_<tag::exp,Arg>;

template<typename Arg>
inline typename proto::result_of::make_expr<
    tag::exp
  , tensor_expr_domain
  , Arg const &
>::type const
exp(Arg const &arg)
{
    return proto::make_expr<tag::exp,tensor_expr_domain>(
        boost::ref(arg)
    );
}

struct indexed_tensor_expr_grammar;

struct boolean_expr
 :proto::or_<
    proto::terminal<bool>
   ,proto::equal_to<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::not_equal_to<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::less<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::greater<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::greater_equal<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::less_equal<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>
   ,proto::logical_and<boolean_expr,boolean_expr>
   ,proto::logical_or<boolean_expr,boolean_expr>
   ,proto::logical_not<boolean_expr>
  >
{};

struct indexed_tensor
 : proto::or_< 
      proto::function<tensor_terminal, proto::vararg< index_grammar > > ,
      delta_<proto::vararg< index_grammar > >
   >
 {};

struct indexed_tensor_expr_grammar
:proto::or_<
          proto::plus< indexed_tensor_expr_grammar , indexed_tensor_expr_grammar >
        , proto::minus< indexed_tensor_expr_grammar, indexed_tensor_expr_grammar >
        , proto::multiplies< indexed_tensor_expr_grammar, indexed_tensor_expr_grammar >
        , proto::divides< indexed_tensor_expr_grammar, indexed_tensor_expr_grammar >
        , proto::unary_plus< indexed_tensor_expr_grammar >
        , proto::negate< indexed_tensor_expr_grammar >
        , indexed_tensor
        , index_grammar
        , proto::terminal<double>
        , proto::or_<
           sum_< indexed_tensor_expr_grammar, index_grammar >
        ,  select_<boolean_expr,indexed_tensor_expr_grammar,indexed_tensor_expr_grammar>
        ,  unary_function_<proto::_,indexed_tensor_expr_grammar>
       >
   >
{};

struct valid_indexed_tensor_expression
:  proto::assign<indexed_tensor,indexed_tensor_expr_grammar>
{};

// intermediate representation

template<typename I>
struct numbered_index
{
};

template<typename I>
inline std::ostream& operator<<(std::ostream& os,const numbered_index<I>& value)
{
        return os << "numbered_index( " << I::value << " )";
}

template<typename I>
struct is_numbered_index : boost::mpl::false_
{};

template<typename I>
struct is_numbered_index<numbered_index<I> > : boost::mpl::true_
{};

struct numbered_index_terminal
    : proto::and_<
            proto::terminal< proto::_ >
          , proto::if_< is_numbered_index< proto::_value >() >
        >
{};

namespace tag
{
        struct memonize {};

        inline std::ostream& operator<<(std::ostream& os,memonize)
        {
                return os << "memonize";
        }

}

template<typename MemonizedExpr>
struct memonize_ : public proto::transform<memonize_<MemonizedExpr> >
{
        typedef proto::expr< tag::memonize, proto::list1< MemonizedExpr > >       type;
        typedef proto::basic_expr< tag::memonize, proto::list1< MemonizedExpr > > proto_grammar;

        template<typename Expr, typename State, typename Data>
        struct impl :  proto::pass_through<memonize_>::template impl<Expr, State, Data>
        {};
};

typedef proto::functional::make_expr<tag::memonize> _make_memonize;

}
}

#endif
