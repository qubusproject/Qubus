#ifndef QUBUS_TENSOR_VAR_HPP
#define QUBUS_TENSOR_VAR_HPP

#include <qubus/local_runtime.hpp>

#include <qubus/qtl/task_generator.hpp>

#include <qubus/IR/pretty_printer.hpp>
#include <qubus/IR/qir.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/integers.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/execution_context.hpp>

#include <qubus/qtl/kronecker_delta_folding_pass.hpp>
#include <qubus/qtl/lower_abstract_indices.hpp>
#include <qubus/qtl/lower_top_level_sums.hpp>
#include <qubus/qtl/multi_index_handling.hpp>
#include <qubus/qtl/sparse_patterns.hpp>

#include <qubus/qtl/kernel_helpers.hpp>

#include <qubus/get_view.hpp>

#include <qubus/associated_qubus_type.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>
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

class closure
{
public:
    closure(computelet stored_computelet_, std::vector<object> args_, std::vector<std::size_t> argument_map_)
    : stored_computelet_(std::move(stored_computelet_)), args_(std::move(args_)), argument_map_(std::move(argument_map_))
    {
    }

    computelet stored_computelet() const
    {
        return stored_computelet_;
    }

    const std::vector<object>& args() const
    {
        return args_;
    }

    const std::vector<std::size_t>& argument_map() const
    {
        return argument_map_;
    }

private:
    computelet stored_computelet_;
    std::vector<object> args_;
    std::vector<std::size_t> argument_map_;
};

template <typename T, long int Rank>
class tensor_expr
{
public:
    template <typename Kernel>
    tensor_expr(const Kernel& kernel)
    {
        std::vector<index_info> indices;
        std::vector<variable_declaration> params;

        auto rhs = build_kernel(kernel, indices, params, args_);

        auto result_type = types::array(associated_qubus_type<T>::get(), Rank);

        std::vector<std::unique_ptr<expression>> subscripts;

        for (const auto& idx : indices)
        {
            if (idx.alias)
            {
                subscripts.push_back(var(*idx.alias));
            }
            else
            {
                QUBUS_ASSERT(idx.indices.size() == 1,
                           "Unnamed multi-indices are currently not supported.");

                subscripts.push_back(var(idx.indices[0]));
            }
        }

        variable_declaration result(result_type);

        auto lhs = subscription(var(result), std::move(subscripts));

        std::unique_ptr<expression> expr = assign(std::move(lhs), std::move(rhs));

        for (const auto& idx : indices | boost::adaptors::reversed)
        {
            if (idx.alias)
            {
                expr = for_all(idx.indices, *idx.alias, std::move(expr));
            }
            else
            {
                QUBUS_ASSERT(idx.indices.size() == 1,
                           "Unnamed multi-indices are currently not supported.");

                expr = for_all(idx.indices[0], std::move(expr));
            }
        }

        expr = expand_multi_indices(*expr);

        expr = fold_kronecker_deltas(*expr);

        auto entry = wrap_code_in_task(clone(*expr));

        expr = optimize_sparse_patterns(*expr);

        expr = lower_top_level_sums(*expr);

        expr = lower_abstract_indices(*expr);

        entry.substitute_body(std::move(expr));

        for (const auto& param : entry.params())
        {
            auto pos = std::find(params.begin(), params.end(), param);

            argument_map_.push_back(pos - params.begin());
        }

        QUBUS_ASSERT(argument_map_.size() == entry.arity(), "Wrong number of arguments mappings.");

        stored_computelet_ = make_computelet(std::move(entry));
    }

    tensor_expr(const tensor<T, Rank>& var) : tensor_expr(tensor_to_expr(var))
    {
    }

    operator closure() const
    {
        if (args_.size() != stored_computelet_.code().get().arity())
            throw 0;

        return closure(stored_computelet_, args_, argument_map_);
    }

    template <typename... Args>
    closure operator()(const Args&... args) const
    {
        std::vector<object> full_args = {args.get_object()...};

        if (full_args.size() != stored_computelet_.code().get().arity())
            throw 0;

        full_args.insert(full_args.end(), args_.begin(), args_.end());

        return closure(stored_computelet_, std::move(full_args), argument_map_);
    }

private:
    static auto tensor_to_expr(const tensor<T, Rank>& var)
    {
        return [var](multi_index<Rank> I) { return var(I); };
    }

    std::vector<object> args_;
    std::vector<std::size_t> argument_map_;
    computelet stored_computelet_;
};

template <typename T, long int Rank>
class tensor
{
public:
    using value_type = T;

    template <typename... SizeTypes>
    explicit tensor(SizeTypes... sizes_)
    : data_(get_runtime().get_object_factory().create_array(associated_qubus_type<T>::get(),
                                                            {util::to_uindex(sizes_)...}))
    {
    }

    tensor<T, Rank>& operator=(closure c)
    {
        const auto& args = c.args();

        execution_context ctx;

        for (const auto& mapping : c.argument_map())
        {
            ctx.push_back_arg(args[mapping]);
        }

        ctx.push_back_result(get_object());

        auto complet = c.stored_computelet();

        get_runtime().execute(complet, std::move(ctx)).get();

        return *this;
    }

    template <typename... Indices>
    auto operator()(Indices... indices) const
    {
        using subscription_type = std::conditional_t<
                boost::hana::any(boost::hana::make_tuple(std::is_same<Indices, range>::value...)),
                sliced_tensor<tensor<T, Rank>, Indices...>, subscripted_tensor<tensor<T, Rank>, Indices...>>;

        return subscription_type(*this, indices...);
    }

    hpx::future<void> when_ready()
    {
        return data_.acquire_read_access();
    }

    object get_object() const
    {
        return data_;
    }

private:
    object data_;
};

template <typename View, typename T, long int Rank>
auto get_view(const tensor<T, Rank>& value)
{
    return get_view<View>(value.get_object());
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