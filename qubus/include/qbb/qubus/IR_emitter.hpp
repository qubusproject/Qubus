#ifndef QUBUS_AST_EMITTER_HPP
#define QUBUS_AST_EMITTER_HPP

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/grammar.hpp>
#include <qbb/qubus/object.hpp>
#include <qbb/qubus/tensor_expr_closure.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/integer_sequence.hpp>
#include <qbb/util/unused.hpp>
#include <qbb/util/optional_ref.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>

#include <memory>
#include <functional>
#include <deque>
#include <utility>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <vector>

namespace qbb
{
namespace qubus
{

class name_scope
{
public:
    using iterator = std::map<util::handle, variable_declaration>::iterator;
    using const_iterator = std::map<util::handle, variable_declaration>::const_iterator;

    void add(util::handle id, variable_declaration decl)
    {
        table_.emplace(id, decl);
    }

    util::optional_ref<const variable_declaration> lookup(const util::handle& id) const
    {
        auto iter = table_.find(id);

        if (iter != table_.end())
        {
            return iter->second;
        }
        else
        {
            return {};
        }
    }

    iterator begin()
    {
        return table_.begin();
    }

    iterator end()
    {
        return table_.end();
    }

    const_iterator begin() const
    {
        return table_.begin();
    }

    const_iterator end() const
    {
        return table_.end();
    }

private:
    std::map<util::handle, variable_declaration> table_;
};

class symbol_table
{
public:
    symbol_table()
    {
        enter_scope();
    }

    void add(util::handle id, variable_declaration decl)
    {
        if (scopes_.empty())
            throw 0;

        current_scope().add(std::move(id), std::move(decl));
    }

    util::optional_ref<const variable_declaration> lookup(const util::handle& id) const
    {
        for (auto iter = scopes_.crbegin(), end = scopes_.crend(); iter != end; ++iter)
        {
            if (auto decl = iter->lookup(id))
            {
                return decl;
            }
        }

        return {};
    }

    void enter_scope()
    {
        scopes_.emplace_back();
    }

    void leave_scope()
    {
        if (scopes_.empty())
            throw 0;

        scopes_.pop_back();
    }

    name_scope& current_scope()
    {
        return scopes_.back();
    }

    const name_scope& current_scope() const
    {
        return scopes_.back();
    }

private:
    std::deque<name_scope> scopes_;
};

struct ast_context
{
public:
    symbol_table& index_table()
    {
        return index_table_;
    }

    symbol_table& variable_table()
    {
        return variable_table_;
    }

private:
    symbol_table index_table_;
    symbol_table variable_table_;
};

template <typename Evaluator>
struct emit_plus_node : proto::transform<emit_plus_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::plus,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_minus_node : proto::transform<emit_minus_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::minus,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_multiplies_node : proto::transform<emit_multiplies_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::multiplies,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_divides_node : proto::transform<emit_divides_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::divides,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_equal_to_node : proto::transform<emit_equal_to_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::equal_to,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_not_equal_to_node : proto::transform<emit_not_equal_to_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::not_equal_to,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_less_node : proto::transform<emit_less_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::less,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_greater_node : proto::transform<emit_greater_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::greater,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_less_equal_node : proto::transform<emit_less_equal_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::less_equal,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_greater_equal_node : proto::transform<emit_greater_equal_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::greater_equal,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_logical_and_node : proto::transform<emit_logical_and_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::logical_and,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_logical_or_node : proto::transform<emit_logical_or_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return binary_operator_expr(binary_op_tag::logical_or,
                                        Evaluator()(proto::left(expr), state, data),
                                        Evaluator()(proto::right(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_unary_plus_node : proto::transform<emit_unary_plus_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return unary_operator_expr(unary_op_tag::plus,
                                       Evaluator()(proto::child_c<0>(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_negate_node : proto::transform<emit_negate_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return unary_operator_expr(unary_op_tag::negate,
                                       Evaluator()(proto::child_c<0>(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_logical_not_node : proto::transform<emit_logical_not_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return unary_operator_expr(unary_op_tag::logical_not,
                                       Evaluator()(proto::child_c<0>(expr), state, data));
        }
    };
};

template <typename Evaluator>
struct emit_sum_node : proto::transform<emit_sum_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            return dispatch(proto::value(proto::child_c<1>(expr)), proto::child_c<0>(expr), state,
                            data);
        }

    private:
        template <typename Body>
        result_type dispatch(const index& idx, const Body& body, typename impl::state_param state,
                             typename impl::data_param data) const
        {
            auto& context = state.get();

            context.index_table().enter_scope();

            auto index_id = id(idx);

            auto index_decl = variable_declaration(types::index());

            if (idx.debug_name())
            {
                index_decl.annotations().add("qubus.debug.name",
                                             annotation(std::string(idx.debug_name())));
            }

            context.index_table().add(index_id, index_decl);

            auto body_expr = Evaluator()(body, state, data);

            context.index_table().leave_scope();

            return sum_expr(index_decl, body_expr);
        }

        template <long int N, typename Body>
        result_type dispatch(const multi_index<N>& idx, const Body& body,
                             typename impl::state_param state, typename impl::data_param data) const
        {
            auto& context = state.get();

            context.index_table().enter_scope();

            auto alias_id = id(idx);

            auto alias_decl = variable_declaration(types::multi_index(idx.rank()));

            if (idx.debug_name())
            {
                alias_decl.annotations().add("qubus.debug.name",
                                             annotation(std::string(idx.debug_name())));
            }

            context.index_table().add(alias_id, alias_decl);

            std::vector<variable_declaration> indices;

            for (long int i = 0; i < idx.rank(); ++i)
            {
                auto index_decl = variable_declaration(types::index());

                if (idx[i].debug_name())
                {
                    index_decl.annotations().add("qubus.debug.name",
                                                 annotation(std::string(idx[i].debug_name())));
                }

                context.index_table().add(id(idx[i]), index_decl);

                indices.push_back(index_decl);
            }

            auto body_expr = Evaluator()(body, state, data);

            context.index_table().leave_scope();

            return sum_expr(indices, alias_decl, body_expr);
        }
    };
};

template <typename Evaluator>
struct emit_index_node : proto::transform<emit_index_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr,
                               typename impl::state_param QBB_UNUSED(state),
                               typename impl::data_param QBB_UNUSED(data)) const
        {
            auto& context = state.get();

            if (auto decl = context.index_table().lookup(id(proto::value(expr))))
            {
                variable_ref_expr index_ref(decl.get());

                return index_ref;
            }
            else
            {
                // error();
                throw 0;
            }
        }
    };
};

namespace detail
{

template <typename Evaluator, typename Expr, typename State, typename Data, std::size_t Front,
          std::size_t... Indices>
inline std::vector<expression> emit_indices(const Expr& expr, const State& state, const Data& data,
                                            util::index_sequence<Front, Indices...>)
{
    std::vector<expression> indices;

    auto QBB_UNUSED(dummy) = {
        (indices.push_back(Evaluator()(proto::child_c<Indices>(expr), state, data)), 0)...};

    return indices;
}
}

template <typename Evaluator>
struct emit_tensor_node : proto::transform<emit_tensor_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            auto obj = proto::value(proto::child_c<0>(expr));

            auto& symbol_table = state.get().variable_table();

            boost::optional<variable_declaration> tensor_decl =
                symbol_table.lookup(util::handle_from_ptr(obj.get()));

            if (!tensor_decl)
            {
                throw 0;
            }

            auto tensor_expr = variable_ref_expr(tensor_decl.get());

            constexpr std::size_t arity = proto::arity_of<Expr>::value;

            auto indices = detail::emit_indices<Evaluator>(expr, state, data,
                                                           util::make_index_sequence<arity>());

            return subscription_expr(tensor_expr, std::move(indices));
        }
    };
};

template <typename Evaluator>
struct emit_delta_node : proto::transform<emit_delta_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            auto extent = proto::value(proto::child_c<0>(expr));

            auto first_index = Evaluator()(proto::child_c<1>(expr), state, data);
            auto second_index = Evaluator()(proto::child_c<2>(expr), state, data);

            return kronecker_delta_expr(extent, first_index, second_index);
        }
    };
};

template <typename Evaluator>
struct emit_literal_node : proto::transform<emit_literal_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr,
                               typename impl::state_param QBB_UNUSED(state),
                               typename impl::data_param QBB_UNUSED(data)) const
        {
            return double_literal_expr(proto::value(expr));
        }
    };
};

namespace detail
{

template <typename Evaluator, typename Expr, typename State, typename Data, std::size_t... Indices>
inline std::vector<expression> emit_args(const Expr& expr, const State& state, const Data& data,
                                         util::index_sequence<Indices...>)
{
    std::vector<expression> args;

    auto dummy = {(args.push_back(Evaluator()(proto::child_c<Indices>(expr), state, data)), 0)...};

    return args;
}

inline constexpr const char* function_tag_to_name(tag::exp)
{
    return "exp";
}
inline constexpr const char* function_tag_to_name(tag::sin)
{
    return "sin";
}
inline constexpr const char* function_tag_to_name(tag::cos)
{
    return "cos";
}
inline constexpr const char* function_tag_to_name(tag::tan)
{
    return "tan";
}
inline constexpr const char* function_tag_to_name(tag::sinh)
{
    return "sinh";
}
inline constexpr const char* function_tag_to_name(tag::cosh)
{
    return "cosh";
}
inline constexpr const char* function_tag_to_name(tag::tanh)
{
    return "tanh";
}
}

template <typename Evaluator>
struct emit_function_node : proto::transform<emit_function_node<Evaluator>>
{
    template <typename Expr, typename State, typename Data>
    struct impl : proto::transform_impl<Expr, State, Data>
    {
        using result_type = expression;

        result_type operator()(typename impl::expr_param expr, typename impl::state_param state,
                               typename impl::data_param data) const
        {
            constexpr std::size_t arity = proto::arity_of<Expr>::value;

            auto args =
                detail::emit_args<Evaluator>(expr, state, data, util::make_index_sequence<arity>());

            const char* name = detail::function_tag_to_name(typename proto::tag_of<Expr>::type{});

            return intrinsic_function_expr(name, std::move(args));
        }
    };
};

struct emit_AST;

struct emit_arithmetic_operations
    : proto::or_<proto::when<proto::plus<proto::_, proto::_>, emit_plus_node<emit_AST>>,
                 proto::when<proto::minus<proto::_, proto::_>, emit_minus_node<emit_AST>>,
                 proto::when<proto::multiplies<proto::_, proto::_>, emit_multiplies_node<emit_AST>>,
                 proto::when<proto::divides<proto::_, proto::_>, emit_divides_node<emit_AST>>,
                 proto::when<proto::unary_plus<proto::_>, emit_unary_plus_node<emit_AST>>,
                 proto::when<proto::negate<proto::_>, emit_negate_node<emit_AST>>>
{
};

struct emit_logical_operations
    : proto::or_<
          proto::when<proto::logical_and<proto::_, proto::_>, emit_logical_and_node<emit_AST>>,
          proto::when<proto::logical_or<proto::_, proto::_>, emit_logical_or_node<emit_AST>>,
          proto::when<proto::logical_not<proto::_>, emit_logical_not_node<emit_AST>>>
{
};

struct emit_AST
    : proto::or_<emit_logical_operations, emit_arithmetic_operations,
                 proto::when<indexed_dense_tensor, emit_tensor_node<emit_AST>>,
                 proto::when<delta_<proto::_, proto::_, proto::_>, emit_delta_node<emit_AST>>,
                 proto::when<sum_<proto::_, proto::_>, emit_sum_node<emit_AST>>,
                 proto::when<unary_function_<proto::_, proto::_>, emit_function_node<emit_AST>>,
                 proto::when<index_terminal, emit_index_node<emit_AST>>,
                 proto::when<multi_index_terminal, emit_index_node<emit_AST>>,
                 proto::when<proto::terminal<proto::_>, emit_literal_node<emit_AST>>>
{
};

inline index_info declare_index(ast_context& ctx, const index& idx)
{
    auto handle = id(idx);

    auto decl = variable_declaration(types::index());

    if (idx.debug_name())
    {
        decl.annotations().add("qubus.debug.name", annotation(std::string(idx.debug_name())));
    }

    ctx.index_table().add(handle, decl);

    return index_info(decl);
}

template <long int N>
inline index_info declare_index(ast_context& ctx, const multi_index<N>& idx)
{
    auto handle = id(idx);

    auto decl = variable_declaration(types::multi_index(idx.rank()));

    if (idx.debug_name())
    {
        decl.annotations().add("qubus.debug.name", annotation(std::string(idx.debug_name())));
    }

    ctx.index_table().add(handle, decl);

    std::vector<variable_declaration> element_indices;

    for (long int i = 0; i < idx.rank(); ++i)
    {
        auto element_index = variable_declaration(types::index());

        if (idx[i].debug_name())
        {
            decl.annotations().add("qubus.debug.name",
                                   annotation(std::string(idx[i].debug_name())));
        }

        element_indices.push_back(element_index);

        ctx.index_table().add(id(idx[i]), element_index);
    }

    return index_info(element_indices, decl);
}

struct deduce_indices : proto::callable
{
    using result_type = std::vector<index_info>;

    template <typename... Indices>
    result_type operator()(std::reference_wrapper<ast_context> ctx, const Indices&... indices) const
    {
        return {declare_index(ctx.get(), indices)...};
    }
};

struct deduce_free_indices
    : proto::or_<
          proto::when<proto::subscript<proto::_, proto::_>, deduce_free_indices(proto::_left)>,
          proto::when<def_tensor_<proto::vararg<proto::_>>,
                      deduce_indices(proto::_state, proto::_value(proto::pack(proto::_))...)>>
{
};

struct add_variable : proto::callable
{
    using result_type = std::vector<std::shared_ptr<object>>&;

    result_type operator()(result_type variables, std::shared_ptr<object> h) const
    {
        variables.push_back(h);

        return variables;
    }
};

struct data_handle : proto::callable
{
    using result_type = std::shared_ptr<object>;

    template <typename Expr>
    result_type operator()(const Expr& expr) const
    {
        return proto::value(expr);
    }
};

struct deduce_variables
    : proto::or_<
          proto::when<indexed_dense_tensor, add_variable(proto::_data, data_handle(proto::_child0))>,
          proto::when<proto::nary_expr<proto::_, proto::vararg<proto::_>>,
                      proto::fold<proto::_, proto::_data, deduce_variables(proto::_)>>,
          proto::when<proto::_, proto::_data>>
{
};

template <typename Expr>
std::tuple<tensor_expr_closure, std::vector<std::shared_ptr<object>>> emit_ast(const Expr& expr)
{
    using namespace boost::adaptors;

    boost::proto::assert_matches<tensor_def>(expr);

    ast_context ctx;

    auto free_indices = deduce_free_indices()(expr, std::ref(ctx));

    std::vector<std::shared_ptr<object>> param_objs;

    deduce_variables()(expr, 0, std::ref(param_objs));

    boost::sort(param_objs);
    auto new_end = std::unique(param_objs.begin(), param_objs.end());

    param_objs.erase(new_end, param_objs.end());

    std::vector<variable_declaration> params;

    for (const auto& obj : param_objs)
    {
        auto decl = variable_declaration(obj->object_type());

        ctx.variable_table().add(util::handle_from_ptr(obj.get()), decl);

        params.push_back(decl);
    }

    expression rhs = emit_AST()(proto::right(expr), std::ref(ctx));

    return std::make_tuple(tensor_expr_closure(free_indices, params, rhs), param_objs);
}
}
}

#endif