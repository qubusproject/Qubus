#ifndef QUBUS_QTL_KERNEL_HPP
#define QUBUS_QTL_KERNEL_HPP

#include <hpx/config.hpp>

#include <qubus/qtl/kernel_helpers.hpp>

#include <qubus/runtime.hpp>

#include <qubus/computelet.hpp>
#include <qubus/kernel_arguments.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/unpack.hpp>

#include <qubus/util/assert.hpp>

#include <functional>
#include <memory>

namespace qubus
{
namespace qtl
{

class kernel;

namespace this_kernel
{
void add_code(std::unique_ptr<expression> code);
void construct(kernel& new_kernel, std::function<void()> constructor);
}

class kernel
{
public:
    template <typename Kernel>
    kernel(Kernel kernel_)
    {
        auto kernel_args = instantiate_kernel_args<Kernel>();

        this_kernel::construct(*this, [&kernel_args, kernel_] { boost::hana::unpack(kernel_args, kernel_); });

        std::vector<variable_declaration> params;

        boost::hana::for_each(kernel_args,
                              [&params](const auto& arg) { params.push_back(arg.var()); });

        translate_kernel(std::move(params));
    }

    kernel(const kernel&) = delete;
    kernel& operator=(const kernel&) = delete;

    kernel(kernel&&) = default;
    kernel& operator=(kernel&&) = default;

    template <typename... Args>
    void operator()(const Args&... args) const
    {
        std::vector<object> full_args = {args.get_object()...};

        full_args.insert(full_args.end(), args_.begin(), args_.end());

        if (full_args.size() != code_.code().get().arity() + 1)
            throw 0;

        kernel_arguments kernel_args;

        for (const auto& mapping : immutable_argument_map_)
        {
            kernel_args.push_back_arg(full_args[mapping]);
        }

        kernel_args.push_back_result(full_args[mutable_argument_map_[0]]);

        get_runtime().execute(code_, std::move(kernel_args)).get();
    }

    void add_code(std::unique_ptr<expression> code)
    {
        computations_.push_back(std::move(code));
    }

private:
    void translate_kernel(std::vector<variable_declaration> params);

    std::vector<std::unique_ptr<expression>> computations_;

    computelet code_;
    std::vector<object> args_;
    std::vector<std::size_t> immutable_argument_map_;
    std::vector<std::size_t> mutable_argument_map_;
};
}
}

#endif
