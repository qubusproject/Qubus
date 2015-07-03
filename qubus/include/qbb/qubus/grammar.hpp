#ifndef QUBUS_GRAMMAR_HPP
#define QUBUS_GRAMMAR_HPP

#include <qbb/qubus/local_tensor.hpp>
#include <qbb/qubus/tensor_expr_info.hpp>

#include <qbb/qubus/indexed_tensor_expr_context.hpp>
#include <qbb/qubus/index.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/qubus/IR/expression.hpp>

#include <boost/proto/core.hpp>

#include <memory>

namespace qbb
{
namespace qubus
{
 
namespace proto = boost::proto;

template <typename T>
struct is_index : boost::mpl::false_
{
};

template <>
struct is_index<index> : boost::mpl::true_
{
};

struct index_terminal
    : proto::and_<proto::terminal<proto::_>, proto::if_<is_index<proto::_value>()>>
{
};

template <typename T>
struct is_multi_index : boost::mpl::false_
{
};

template <long int N>
struct is_multi_index<multi_index<N>> : boost::mpl::true_
{
};

struct multi_index_terminal
    : proto::and_<proto::terminal<proto::_>, proto::if_<is_multi_index<proto::_value>()>>
{
};

template <typename T, std::size_t Rank>
class tensor_holder;

template <typename>
struct is_tensor : boost::mpl::false_
{
};

template <>
struct is_tensor<std::shared_ptr<local_tensor>> : boost::mpl::true_
{
};

template <>
struct is_tensor<tensor_expr_info> : boost::mpl::true_
{
};

struct tensor_terminal
    : proto::and_<proto::terminal<proto::_>, proto::if_<is_tensor<proto::_value>()>>
{
};

struct index_grammar : proto::or_<index_terminal, multi_index_terminal, proto::terminal<int>>
{
};

namespace tag
{
struct sum
{
};

inline std::ostream& operator<<(std::ostream& os, sum)
{
    return os << "sum";
}

struct select
{
};

inline std::ostream& operator<<(std::ostream& os, select)
{
    return os << "select";
}

struct delta
{
};

inline std::ostream& operator<<(std::ostream& os, delta)
{
    return os << "delta";
}

struct sin
{
};

inline std::ostream& operator<<(std::ostream& os, sin)
{
    return os << "sin";
}

struct cos
{
};

inline std::ostream& operator<<(std::ostream& os, cos)
{
    return os << "cos";
}

struct tan
{
};

inline std::ostream& operator<<(std::ostream& os, tan)
{
    return os << "tan";
}

struct sinh
{
};

inline std::ostream& operator<<(std::ostream& os, sinh)
{
    return os << "sinh";
}

struct cosh
{
};

inline std::ostream& operator<<(std::ostream& os, cosh)
{
    return os << "cosh";
}

struct tanh
{
};

inline std::ostream& operator<<(std::ostream& os, tanh)
{
    return os << "tanh";
}

struct exp
{
};

inline std::ostream& operator<<(std::ostream& os, exp)
{
    return os << "exp";
}
}

template <typename Arg, typename Index>
struct sum_ : public proto::transform<sum_<Arg, Index>>
{
    typedef proto::expr<tag::sum, proto::list2<Arg, Index>> type;
    typedef proto::basic_expr<tag::sum, proto::list2<Arg, Index>> proto_grammar;

    template <typename Expr, typename State, typename Data>
    struct impl : proto::pass_through<sum_>::template impl<Expr, State, Data>
    {
    };
};

template <typename Arg, typename Index>
inline typename proto::result_of::make_expr<tag::sum, tensor_expr_domain, Arg const&,
                                            Index const&>::type const
sum(Arg const& arg, Index const& index)
{
    return proto::make_expr<tag::sum, tensor_expr_domain>(boost::ref(arg), boost::ref(index));
}

template <typename Cond, typename Then, typename Else>
struct select_ : public proto::transform<select_<Cond, Then, Else>>
{
    typedef proto::expr<tag::select, proto::list3<Cond, Then, Else>> type;
    typedef proto::basic_expr<tag::select, proto::list3<Cond, Then, Else>> proto_grammar;

    template <typename Expr, typename State, typename Data>
    struct impl : proto::pass_through<select_>::template impl<Expr, State, Data>
    {
    };
};

template <typename Cond, typename Then, typename Else>
inline typename proto::result_of::make_expr<tag::select, tensor_expr_domain, Cond const&,
                                            Then const&, Else const&>::type const
select(Cond const& cond, Then const& then, Else const& else_)
{
    return proto::make_expr<tag::select, tensor_expr_domain>(boost::ref(cond), boost::ref(then),
                                                             boost::ref(else_));
}

template <typename... Indices>
using delta_ = proto::nary_expr<tag::delta, Indices...>;

template <typename... Indices>
inline typename proto::result_of::make_expr<tag::delta, tensor_expr_domain,
                                            Indices const&...>::type const
delta(Indices const&... indices)
{
    return proto::make_expr<tag::delta, tensor_expr_domain>(boost::ref(indices)...);
}

template <typename Tag, typename Arg>
using unary_function_ = proto::unary_expr<Tag, Arg>;

template <typename Arg>
using sin_ = unary_function_<tag::sin, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::sin, tensor_expr_domain, Arg const&>::type const
sin(Arg const& arg)
{
    return proto::make_expr<tag::sin, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using cos_ = unary_function_<tag::cos, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::cos, tensor_expr_domain, Arg const&>::type const
cos(Arg const& arg)
{
    return proto::make_expr<tag::cos, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using tan_ = unary_function_<tag::tan, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::tan, tensor_expr_domain, Arg const&>::type const
tan(Arg const& arg)
{
    return proto::make_expr<tag::tan, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using sinh_ = unary_function_<tag::sinh, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::sinh, tensor_expr_domain, Arg const&>::type const
sinh(Arg const& arg)
{
    return proto::make_expr<tag::sinh, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using cosh_ = unary_function_<tag::cosh, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::cosh, tensor_expr_domain, Arg const&>::type const
cosh(Arg const& arg)
{
    return proto::make_expr<tag::cosh, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using tanh_ = unary_function_<tag::tanh, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::tanh, tensor_expr_domain, Arg const&>::type const
tanh(Arg const& arg)
{
    return proto::make_expr<tag::tanh, tensor_expr_domain>(boost::ref(arg));
}

template <typename Arg>
using exp_ = unary_function_<tag::exp, Arg>;

template <typename Arg>
inline typename proto::result_of::make_expr<tag::exp, tensor_expr_domain, Arg const&>::type const
exp(Arg const& arg)
{
    return proto::make_expr<tag::exp, tensor_expr_domain>(boost::ref(arg));
}

struct indexed_tensor_expr_grammar;

struct boolean_expr
    : proto::or_<proto::terminal<bool>,
                 proto::equal_to<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::not_equal_to<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::less<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::greater<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::greater_equal<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::less_equal<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::logical_and<boolean_expr, boolean_expr>,
                 proto::logical_or<boolean_expr, boolean_expr>, proto::logical_not<boolean_expr>>
{
};

struct indexed_tensor : proto::or_<proto::function<tensor_terminal, proto::vararg<index_grammar>>,
                                   delta_<proto::vararg<index_grammar>>>
{
};

struct indexed_tensor_expr_grammar
    : proto::or_<proto::plus<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::minus<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::multiplies<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::divides<indexed_tensor_expr_grammar, indexed_tensor_expr_grammar>,
                 proto::unary_plus<indexed_tensor_expr_grammar>,
                 proto::negate<indexed_tensor_expr_grammar>, indexed_tensor, index_grammar,
                 proto::terminal<double>,
                 proto::or_<sum_<indexed_tensor_expr_grammar, index_grammar>,
                            select_<boolean_expr, indexed_tensor_expr_grammar,
                                    indexed_tensor_expr_grammar>,
                            unary_function_<proto::_, indexed_tensor_expr_grammar>>>
{
};

namespace tag
{
struct def_tensor
{
};

inline std::ostream& operator<<(std::ostream& os, def_tensor)
{
    return os << "def_tensor";
}
}

template <typename Indices>
using def_tensor_ = proto::nary_expr<tag::def_tensor, Indices>;

template <typename... Indices>
inline typename proto::result_of::make_expr<tag::def_tensor, tensor_expr_domain,
                                            Indices const&...>::type const
def_tensor(Indices const&... indices)
{
    return proto::make_expr<tag::def_tensor, tensor_expr_domain>(boost::ref(indices)...);
}

struct tensor_def
: proto::subscript<def_tensor_<proto::vararg<proto::_>>, indexed_tensor_expr_grammar>
{
};

}
}

#endif
